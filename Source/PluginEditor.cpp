#include "PluginEditor.h"

#if ASSPLUG_HAS_FONT
 #include "BinaryData.h"
#endif

namespace
{
    constexpr float pi = juce::MathConstants<float>::pi;
    const juce::Colour pageBackground { 0xff1b1412 };
    const juce::Colour inkColour      { 0xff26140a };

    // Polarkoordinaten: Winkel 0 = oben, im Uhrzeigersinn
    juce::Point<float> polar (float centre, float radius, float angle)
    {
        return { centre + radius * std::sin (angle), centre - radius * std::cos (angle) };
    }

    // Deterministischer Zufall, damit die Maserung immer gleich aussieht
    struct Rng
    {
        juce::int64 s = 7;
        float next() { s = (s * 16807) % 2147483647; return (float) s / 2147483647.0f; }
    };
}

//==============================================================================
RubyLookAndFeel::RubyLookAndFeel()
{
    const float C = 60.0f;
    const int n = 8;
    const float step = 2.0f * pi / (float) n;

    const juce::uint32 tones[] = { 0xffff8f9e, 0xffe0334f, 0xffc41a37, 0xff9e0f2a,
                                   0xffffc2cb, 0xffd6243f, 0xff7a0820, 0xfff05470 };
    int k = 0;
    auto nextTone = [&] { k = (k * 5 + 3) % 8; return juce::Colour (tones[k]); };

    juce::Path table;
    for (int i = 0; i < n; ++i)
    {
        const auto p = polar (C, 15.0f, (float) i * step);
        if (i == 0) table.startNewSubPath (p); else table.lineTo (p);
    }
    table.closeSubPath();
    facets.push_back ({ table, juce::Colour (0xffff7086) });

    auto addTriangle = [&] (juce::Point<float> a, juce::Point<float> b, juce::Point<float> c)
    {
        juce::Path p;
        p.addTriangle (a, b, c);
        facets.push_back ({ p, nextTone() });
    };

    for (int i = 0; i < n; ++i)
    {
        const float a = (float) i * step, b = (float) (i + 1) * step, m = a + step * 0.5f;
        const auto t0 = polar (C, 15.0f, a), t1 = polar (C, 15.0f, b), md = polar (C, 28.0f, m);
        const auto g0 = polar (C, 40.0f, a), g1 = polar (C, 40.0f, b), gm = polar (C, 40.0f, m);

        addTriangle (t0, t1, md);
        addTriangle (t0, md, g0);
        addTriangle (t1, g1, md);
        addTriangle (md, g0, gm);
        addTriangle (md, gm, g1);
    }

    // Werteanzeige beim Drehen
    setColour (juce::BubbleComponent::backgroundColourId, inkColour.withAlpha (0.92f));
    setColour (juce::BubbleComponent::outlineColourId,    juce::Colour (0xff6b4222));
    setColour (juce::TooltipWindow::textColourId,         juce::Colour (0xfff8ebe2));
}

void RubyLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float startAngle, float endAngle, juce::Slider&)
{
    const float size = (float) juce::jmin (width, height);
    const float ox = (float) x + ((float) width  - size) * 0.5f;
    const float oy = (float) y + ((float) height - size) * 0.5f;
    const float angle = startAngle + sliderPos * (endAngle - startAngle);

    juce::Graphics::ScopedSaveState state (g);
    g.addTransform (juce::AffineTransform::scale (size / 120.0f).translated (ox, oy));

    // Metallfassung
    juce::ColourGradient bezel (juce::Colour (0xff8a888e), 48.0f, 36.0f,
                                juce::Colour (0xff232226), 138.0f, 36.0f, true);
    bezel.addColour (0.55, juce::Colour (0xff4a484d));
    g.setGradientFill (bezel);
    g.fillEllipse (13.0f, 13.0f, 94.0f, 94.0f);
    g.setColour (juce::Colour (0xff2b2a2e));
    g.fillEllipse (19.0f, 19.0f, 82.0f, 82.0f);

    // Facetten, mit dem Regler gedreht
    {
        juce::Graphics::ScopedSaveState clipState (g);
        juce::Path clip;
        clip.addEllipse (20.0f, 20.0f, 80.0f, 80.0f);
        g.reduceClipRegion (clip);

        const auto rotation = juce::AffineTransform::rotation (angle, 60.0f, 60.0f);
        const auto edge = juce::Colour (0xffffd6dc).withAlpha (0.4f);

        for (const auto& f : facets)
        {
            auto p = f.path;
            p.applyTransform (rotation);
            g.setColour (f.colour);
            g.fillPath (p);
            g.setColour (edge);
            g.strokePath (p, juce::PathStrokeType (0.5f, juce::PathStrokeType::curved));
        }
    }

    // Stellungspunkt
    const auto dot = polar (60.0f, 43.5f, angle);
    g.setColour (juce::Colour (0xfffff4f5));
    g.fillEllipse (dot.x - 2.6f, dot.y - 2.6f, 5.2f, 5.2f);

    // Tiefe zum Rand hin
    juce::ColourGradient depth (juce::Colour (0x003a0010), 60.0f, 66.0f,
                                juce::Colour (0x733a0010), 126.0f, 66.0f, true);
    depth.addColour (0.6, juce::Colour (0x003a0010));
    g.setGradientFill (depth);
    g.fillEllipse (20.0f, 20.0f, 80.0f, 80.0f);

    // Glanzlicht
    juce::ColourGradient shine (juce::Colours::white.withAlpha (0.5f), 42.0f, 34.0f,
                                juce::Colours::white.withAlpha (0.0f), 114.0f, 34.0f, true);
    shine.addColour (0.5, juce::Colours::white.withAlpha (0.06f));
    g.setGradientFill (shine);
    g.fillEllipse (20.0f, 20.0f, 80.0f, 80.0f);

    g.setColour (juce::Colour (0xff1a191c));
    g.drawEllipse (20.0f, 20.0f, 80.0f, 80.0f, 1.2f);
}

//==============================================================================
AssPlugEditor::AssPlugEditor (AssPlugProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
   #if ASSPLUG_HAS_FONT
    titleTypeface = juce::Typeface::createSystemTypefaceFor (BinaryData::RockSaltRegular_ttf,
                                                             BinaryData::RockSaltRegular_ttfSize);
   #endif

    setLookAndFeel (&rubyLookAndFeel);

    knob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    knob.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    knob.setRotaryParameters (1.25f * pi, 2.75f * pi, true);   // -135° bis +135°
    knob.setMouseDragSensitivity (250);
    knob.setScrollWheelEnabled (true);
    knob.setPopupDisplayEnabled (true, true, this, 1500);
    knob.setTitle ("AssPlug");
    addAndMakeVisible (knob);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "tilt", knob);
    knob.setDoubleClickReturnValue (true, 0.0);

    setResizable (true, true);
    setResizeLimits (320, 320, 900, 900);
    if (auto* c = getConstrainer())
        c->setFixedAspectRatio (1.0);
    setSize (440, 440);
}

AssPlugEditor::~AssPlugEditor()
{
    setLookAndFeel (nullptr);
}

juce::Rectangle<float> AssPlugEditor::plateBounds() const
{
    const auto b = getLocalBounds().toFloat();
    const float size = juce::jmin (b.getWidth(), b.getHeight()) * 0.94f;
    return juce::Rectangle<float> (size, size).withCentre (b.getCentre());
}

void AssPlugEditor::renderWood()
{
    const auto plate = plateBounds();
    const int px = juce::jmax (1, juce::roundToInt (plate.getWidth() * 2.0f));   // 2x für scharfe Darstellung

    woodCache = juce::Image (juce::Image::ARGB, px, px, true);
    juce::Graphics g (woodCache);
    g.addTransform (juce::AffineTransform::scale ((float) px / 300.0f));

    juce::Path disc;
    disc.addEllipse (0.0f, 0.0f, 300.0f, 300.0f);
    g.reduceClipRegion (disc);

    // Grundfarbe mit dunklem Ast in der Mitte
    juce::ColourGradient base (juce::Colour (0xff4a2a12), 150.0f, 171.0f,
                               juce::Colour (0xff9a6234), 360.0f, 171.0f, true);
    base.addColour (0.20, juce::Colour (0xff7a4a22));
    base.addColour (0.45, juce::Colour (0xffa86f3c));
    base.addColour (0.75, juce::Colour (0xffb98353));
    g.setGradientFill (base);
    g.fillRect (0.0f, 0.0f, 300.0f, 300.0f);

    // Jahresringe um das Astloch, nach außen zur Längsmaserung gestreckt
    Rng rng;
    const float cx = 150.0f, cy = 172.0f;
    float r = 34.0f;

    while (r < 330.0f)
    {
        const float p1 = rng.next() * 6.28f, p2 = rng.next() * 6.28f, p3 = rng.next() * 6.28f;
        const float amp = 1.5f + r * 0.06f;
        const float sx = 1.0f + (r / 300.0f) * 0.9f;
        const float sy = 1.0f - (r / 300.0f) * 0.25f;

        constexpr int numPts = 96;
        std::array<juce::Point<float>, numPts> pts;
        for (int k = 0; k < numPts; ++k)
        {
            const float t = (float) k / (float) numPts * 2.0f * pi;
            const float rr = r + amp * (0.5f * std::sin (2.0f * t + p1)
                                      + 0.3f * std::sin (5.0f * t + p2)
                                      + 0.2f * std::sin (9.0f * t + p3));
            pts[(size_t) k] = { cx + std::cos (t) * rr * sx, cy + std::sin (t) * rr * sy };
        }

        auto mid = [&] (int a, int b) { return (pts[(size_t) a] + pts[(size_t) b]) * 0.5f; };

        juce::Path ring;
        ring.startNewSubPath (mid (0, 1));
        for (int k = 1; k < numPts; ++k)
            ring.quadraticTo (pts[(size_t) k], mid (k, (k + 1) % numPts));
        ring.quadraticTo (pts[0], mid (0, 1));
        ring.closeSubPath();

        const bool light = rng.next() < 0.25f;
        const auto colour = light ? juce::Colour (245, 205, 150).withAlpha (0.10f + rng.next() * 0.12f)
                                  : juce::Colour (62, 32, 12).withAlpha (0.14f + rng.next() * 0.34f);
        const float strokeWidth = 0.4f + rng.next() * 1.4f;

        g.setColour (colour);
        g.strokePath (ring, juce::PathStrokeType (strokeWidth));

        r += 2.5f + rng.next() * 5.0f + r * 0.015f;
    }

    // Lackglanz
    juce::ColourGradient varnish (juce::Colour (0xfffff2dc).withAlpha (0.28f), 96.0f, 66.0f,
                                  juce::Colour (0xfffff2dc).withAlpha (0.0f), 306.0f, 66.0f, true);
    varnish.addColour (0.5, juce::Colour (0xfffff2dc).withAlpha (0.04f));
    g.setGradientFill (varnish);
    g.fillRect (0.0f, 0.0f, 300.0f, 300.0f);

    // Dunkler Rand
    juce::ColourGradient rim (juce::Colour (0x001e0f05), 150.0f, 150.0f,
                              juce::Colour (0x8c1e0f05), 300.0f, 150.0f, true);
    rim.addColour (0.86, juce::Colour (0x001e0f05));
    g.setGradientFill (rim);
    g.fillRect (0.0f, 0.0f, 300.0f, 300.0f);
}

void AssPlugEditor::paint (juce::Graphics& g)
{
    g.fillAll (pageBackground);

    const auto plate = plateBounds();
    const float S = plate.getWidth();

    // Schatten und Holzkante
    juce::Path plateShape;
    plateShape.addEllipse (plate);
    juce::DropShadow (juce::Colours::black.withAlpha (0.6f), juce::roundToInt (S * 0.06f),
                      { 0, juce::roundToInt (S * 0.025f) }).drawForPath (g, plateShape);

    g.setColour (juce::Colour (0xff6b4222));
    g.fillEllipse (plate.expanded (S * 0.012f));
    g.setColour (juce::Colour (0xff3b220f));
    g.fillEllipse (plate.expanded (S * 0.006f));

    g.drawImage (woodCache, plate);

    // Schriftzug
    {
        const juce::Font font = titleTypeface != nullptr
            ? juce::Font (juce::FontOptions (titleTypeface).withHeight (S * 0.115f))
            : juce::Font (juce::FontOptions (S * 0.11f, juce::Font::bold | juce::Font::italic));

        const juce::Point<float> centre (plate.getCentreX(), plate.getY() + S * 0.19f);
        const auto area = juce::Rectangle<float> (S, S * 0.22f).withCentre (centre);

        juce::Graphics::ScopedSaveState state (g);
        g.addTransform (juce::AffineTransform::rotation (-5.0f * pi / 180.0f, centre.x, centre.y));
        g.setFont (font);
        g.setColour (juce::Colour (0xffffdeb4).withAlpha (0.35f));
        g.drawText ("AssPlug", area.translated (0.0f, 1.0f), juce::Justification::centred, false);
        g.setColour (inkColour);
        g.drawText ("AssPlug", area, juce::Justification::centred, false);
    }

    // Schatten unter dem Rubin
    {
        const auto k = knob.getBounds().toFloat();
        juce::Path gemShape;
        gemShape.addEllipse (k.reduced (k.getWidth() * (13.0f / 120.0f)));
        juce::DropShadow (juce::Colour (0x8c1e0e04), juce::roundToInt (S * 0.035f),
                          { 0, juce::roundToInt (S * 0.02f) }).drawForPath (g, gemShape);
    }
}

void AssPlugEditor::resized()
{
    const auto plate = plateBounds();
    const float S = plate.getWidth();
    const float knobSize = S * 0.44f;

    knob.setBounds (juce::Rectangle<float> (knobSize, knobSize)
                        .withCentre ({ plate.getCentreX(), plate.getY() + S * 0.57f })
                        .toNearestInt());

    renderWood();
}
