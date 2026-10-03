#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

// AssPlug: Glocke bei 700 Hz (Q 1,5) und High Shelf ab 3 kHz,
// gegenläufig gesteuert von einem einzigen Regler.
//   Regler rechts (+): Mitten hoch, Höhen runter
//   Regler links  (-): Höhen hoch, Mitten runter
class AssPlugProcessor : public juce::AudioProcessor
{
public:
    static constexpr float midFreq   = 700.0f;
    static constexpr float midQ      = 1.5f;
    static constexpr float highFreq  = 3000.0f;
    static constexpr float maxGainDb = 18.0f;

    AssPlugProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override                        { return true; }

    const juce::String getName() const override            { return JucePlugin_Name; }
    bool acceptsMidi() const override                      { return false; }
    bool producesMidi() const override                     { return false; }
    bool isMidiEffect() const override                     { return false; }
    double getTailLengthSeconds() const override           { return 0.0; }

    int getNumPrograms() override                          { return 1; }
    int getCurrentProgram() override                       { return 0; }
    void setCurrentProgram (int) override                  {}
    const juce::String getProgramName (int) override       { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    juce::AudioProcessorValueTreeState apvts;

private:
    void updateFilters (float tiltDb);

    using Filter = juce::dsp::IIR::Filter<float>;
    using Coeffs = juce::dsp::IIR::Coefficients<float>;
    juce::dsp::ProcessorDuplicator<Filter, Coeffs> mid, high;

    std::atomic<float>* tiltParam = nullptr;
    juce::SmoothedValue<float> tiltSmoothed;
    double currentSampleRate = 44100.0;
    float lastTilt = 1.0e9f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AssPlugProcessor)
};
