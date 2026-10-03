#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    juce::String tiltToText (float v, int)
    {
        if (std::abs (v) < 0.05f)
            return "neutral";

        const auto amount = juce::String (std::abs (v), 1) + " dB";
        return v > 0.0f ? "Mitten +" + amount
                        : juce::String (juce::CharPointer_UTF8 ("H\xc3\xb6hen +")) + amount;
    }

    float textToTilt (const juce::String& text)
    {
        const auto s = text.trim();
        if (s.startsWithIgnoreCase ("neutral"))
            return 0.0f;

        const auto v = s.retainCharacters ("0123456789.,-").replaceCharacter (',', '.').getFloatValue();
        if (s.startsWithIgnoreCase ("H")) return -std::abs (v);
        if (s.startsWithIgnoreCase ("M")) return  std::abs (v);
        return v;
    }
}

AssPlugProcessor::AssPlugProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "AssPlug", createLayout())
{
    tiltParam = apvts.getRawParameterValue ("tilt");
}

juce::AudioProcessorValueTreeState::ParameterLayout AssPlugProcessor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto attributes = juce::AudioParameterFloatAttributes()
                          .withStringFromValueFunction (tiltToText)
                          .withValueFromStringFunction (textToTilt);

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "tilt", 1 }, "Tilt",
        juce::NormalisableRange<float> (-maxGainDb, maxGainDb, 0.1f),
        0.0f, attributes));

    return layout;
}

void AssPlugProcessor::updateFilters (float tiltDb)
{
    using AC = juce::dsp::IIR::ArrayCoefficients<float>;
    const auto sr = currentSampleRate;

    // Keine Speicherallokation im Audio-Thread: Koeffizienten werden direkt überschrieben
    *mid.state  = AC::makePeakFilter (sr, midFreq, midQ, juce::Decibels::decibelsToGain (tiltDb));
    *high.state = AC::makeHighShelf  (sr, highFreq, juce::MathConstants<float>::sqrt2 * 0.5f,
                                      juce::Decibels::decibelsToGain (-tiltDb));
}

void AssPlugProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    const float t = tiltParam->load();
    updateFilters (t);
    lastTilt = t;

    juce::dsp::ProcessSpec spec { sampleRate,
                                  (juce::uint32) samplesPerBlock,
                                  (juce::uint32) juce::jmax (1, getTotalNumOutputChannels()) };
    mid.prepare (spec);
    high.prepare (spec);
    mid.reset();
    high.reset();

    tiltSmoothed.reset (sampleRate, 0.05);
    tiltSmoothed.setCurrentAndTargetValue (t);
}

bool AssPlugProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    return out == layouts.getMainInputChannelSet();
}

void AssPlugProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numIn  = getTotalNumInputChannels();
    const int numOut = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    for (int ch = numIn; ch < numOut; ++ch)
        buffer.clear (ch, 0, numSamples);

    tiltSmoothed.setTargetValue (tiltParam->load());

    auto block = juce::dsp::AudioBlock<float> (buffer).getSubsetChannelBlock (0, (size_t) numOut);

    // In kleinen Abschnitten rechnen, damit Reglerbewegungen weich und knackfrei bleiben
    constexpr int subBlock = 32;
    for (int start = 0; start < numSamples; start += subBlock)
    {
        const int len = juce::jmin (subBlock, numSamples - start);
        const float t = tiltSmoothed.skip (len);

        if (t != lastTilt)
        {
            updateFilters (t);
            lastTilt = t;
        }

        auto part = block.getSubBlock ((size_t) start, (size_t) len);
        juce::dsp::ProcessContextReplacing<float> context (part);
        mid.process (context);
        high.process (context);
    }
}

juce::AudioProcessorEditor* AssPlugProcessor::createEditor()
{
    return new AssPlugEditor (*this);
}

void AssPlugProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void AssPlugProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AssPlugProcessor();
}
