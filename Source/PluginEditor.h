#pragma once

#include "PluginProcessor.h"

// Zeichnet den Regler als facettierten Rubin in einer Metallfassung
class RubyLookAndFeel : public juce::LookAndFeel_V4
{
public:
    RubyLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider&) override;

private:
    struct Facet { juce::Path path; juce::Colour colour; };
    std::vector<Facet> facets;
};

// Reagiert nur auf Klicks innerhalb der runden Fassung
class RubyKnob : public juce::Slider
{
public:
    bool hitTest (int x, int y) override
    {
        const auto c = getLocalBounds().toFloat().getCentre();
        return c.getDistanceFrom ({ (float) x, (float) y }) <= (float) getWidth() * 0.5f * (47.0f / 60.0f);
    }
};

class AssPlugEditor : public juce::AudioProcessorEditor
{
public:
    explicit AssPlugEditor (AssPlugProcessor&);
    ~AssPlugEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::Rectangle<float> plateBounds() const;
    void renderWood();

    AssPlugProcessor& proc;
    RubyLookAndFeel rubyLookAndFeel;
    RubyKnob knob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

    juce::Image woodCache;
    juce::Typeface::Ptr titleTypeface;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AssPlugEditor)
};
