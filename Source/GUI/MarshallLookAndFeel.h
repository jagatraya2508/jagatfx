#pragma once
#include <JuceHeader.h>

class MarshallLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MarshallLookAndFeel()
    {
        // General text color overridden for marshall knobs
        setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xFF222222));
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override
    {
        auto radius = (float)juce::jmin(width, height) * 0.45f;
        auto centreX = (float)x + (float)width * 0.5f;
        auto centreY = (float)y + (float)height * 0.5f;
        auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // 1. Drop shadow
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.fillEllipse(centreX - radius + 2.0f, centreY - radius + 3.0f, radius * 2, radius * 2);

        // 2. Base brass cone/gradient
        juce::ColourGradient baseGradient(
            juce::Colour(0xFFE8D08B), centreX - radius, centreY - radius,
            juce::Colour(0xFFB08C44), centreX + radius, centreY + radius, false);
        g.setGradientFill(baseGradient);
        g.fillEllipse(centreX - radius, centreY - radius, radius * 2, radius * 2);

        // 3. Inner bright rim to simulate 3D cylinder/bevel (Marshall knobs have a flat shiny top with a beveled edge)
        auto innerRadius = radius * 0.85f;
        juce::ColourGradient innerGradient(
            juce::Colour(0xFFF7E2A9), centreX, centreY - innerRadius,
            juce::Colour(0xFFB89240), centreX, centreY + innerRadius, false);
        g.setGradientFill(innerGradient);
        g.fillEllipse(centreX - innerRadius, centreY - innerRadius, innerRadius * 2, innerRadius * 2);

        // 4. Center concavity / shadow
        auto centerRadius = innerRadius * 0.6f;
        juce::ColourGradient centerGradient(
            juce::Colour(0xFFD4B15C), centreX - centerRadius, centreY - centerRadius,
            juce::Colour(0xFFECDBA6), centreX + centerRadius, centreY + centerRadius, false);
        g.setGradientFill(centerGradient);
        g.fillEllipse(centreX - centerRadius, centreY - centerRadius, centerRadius * 2, centerRadius * 2);

        // 5. The Pointer Indicator (Black, extending from center to inner rim)
        juce::Path pointer;
        auto pointerLength = innerRadius * 0.95f;
        auto pointerThickness = 2.5f;
        
        // Draw the line down from center
        pointer.addRectangle(-pointerThickness * 0.5f, 0.0f, pointerThickness, pointerLength);
        
        // Rotate the pointer by the calculated angle, offsetting so it pivots at the center
        pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        
        g.setColour(juce::Colour(0xFF151515)); // Almost black
        g.fillPath(pointer);
    }
    
    // Customize Combo box look to match the gold bottom panel or top dark panel
    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat().reduced(1);

        g.setColour(juce::Colour(0xFF151518)); // Very dark for contrast against gold
        g.fillRoundedRectangle(bounds, 3.0f);
        g.setColour(juce::Colour(0xFF886633)); // Brass-like border
        g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

        // Arrow
        auto arrowBounds = juce::Rectangle<float>((float)buttonX, (float)buttonY, (float)buttonW, (float)buttonH);
        juce::Path arrow;
        arrow.addTriangle(arrowBounds.getCentreX() - 4, arrowBounds.getCentreY() - 2,
                          arrowBounds.getCentreX() + 4, arrowBounds.getCentreY() - 2,
                          arrowBounds.getCentreX(), arrowBounds.getCentreY() + 3);
        g.setColour(juce::Colour(0xFFE8D08B));
        g.fillPath(arrow);
    }
    
    // Used text color for label in custom Amp Section
    juce::Colour getAmpLabelColour() { return juce::Colour(0xFF222222); }
};
