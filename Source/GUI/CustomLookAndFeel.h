#pragma once
#include <JuceHeader.h>

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel()
    {
        // Dark theme colors
        setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xFF1A1A2E));
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xFFE94560));
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xFF333355));
        setColour(juce::Slider::thumbColourId, juce::Colour(0xFFE94560));
        setColour(juce::Label::textColourId, juce::Colour(0xFFCCCCDD));
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF252545));
        setColour(juce::ComboBox::textColourId, juce::Colour(0xFFCCCCDD));
        setColour(juce::ComboBox::outlineColourId, juce::Colour(0xFF444466));
        setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF252545));
        setColour(juce::TextButton::textColourOffId, juce::Colour(0xFFCCCCDD));
        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xFF1A1A2E));
        setColour(juce::PopupMenu::textColourId, juce::Colour(0xFFCCCCDD));
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xFFE94560));
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override
    {
        auto radius = (float)juce::jmin(width, height) * 0.4f;
        auto centreX = (float)x + (float)width * 0.5f;
        auto centreY = (float)y + (float)height * 0.5f;
        auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // Outer ring shadow
        g.setColour(juce::Colour(0xFF0D0D1A));
        g.fillEllipse(centreX - radius - 2, centreY - radius - 2, radius * 2 + 4, radius * 2 + 4);

        // Background circle
        g.setColour(juce::Colour(0xFF252545));
        g.fillEllipse(centreX - radius, centreY - radius, radius * 2, radius * 2);

        // Outer ring (track)
        juce::Path backgroundArc;
        backgroundArc.addCentredArc(centreX, centreY, radius - 2, radius - 2,
                                     0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(juce::Colour(0xFF333355));
        g.strokePath(backgroundArc, juce::PathStrokeType(3.0f));

        // Value arc
        if (sliderPosProportional > 0.0f)
        {
            juce::Path valueArc;
            valueArc.addCentredArc(centreX, centreY, radius - 2, radius - 2,
                                    0.0f, rotaryStartAngle, angle, true);

            juce::ColourGradient gradient(
                juce::Colour(0xFF0F3460), centreX, centreY - radius,
                juce::Colour(0xFFE94560), centreX, centreY + radius, false);
            g.setGradientFill(gradient);
            g.strokePath(valueArc, juce::PathStrokeType(3.0f));
        }

        // Pointer line
        juce::Path pointer;
        auto pointerLength = radius * 0.65f;
        auto pointerThickness = 2.5f;
        pointer.addRoundedRectangle(-pointerThickness * 0.5f, -pointerLength, pointerThickness, pointerLength, 1.0f);
        pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        g.setColour(juce::Colour(0xFFE94560));
        g.fillPath(pointer);

        // Center dot
        g.setColour(juce::Colour(0xFF1A1A2E));
        g.fillEllipse(centreX - 4, centreY - 4, 8, 8);
        g.setColour(juce::Colour(0xFFE94560));
        g.fillEllipse(centreX - 2.5f, centreY - 2.5f, 5, 5);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(4);

        // LED-style toggle
        auto ledSize = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.4f;
        auto ledBounds = juce::Rectangle<float>(
            bounds.getCentreX() - ledSize * 0.5f,
            bounds.getY() + 2,
            ledSize, ledSize);

        if (button.getToggleState())
        {
            // ON - green glow
            g.setColour(juce::Colour(0x4400FF00));
            g.fillEllipse(ledBounds.expanded(3));
            g.setColour(juce::Colour(0xFF00FF00));
            g.fillEllipse(ledBounds);
        }
        else
        {
            // OFF - dim red
            g.setColour(juce::Colour(0xFF442222));
            g.fillEllipse(ledBounds);
        }

        // Label
        g.setColour(juce::Colour(0xFFAABBCC));
        g.setFont(juce::Font(13.0f, juce::Font::bold));
        auto textBounds = bounds.withTop(ledBounds.getBottom() + 1);
        g.drawText(button.getButtonText(), textBounds, juce::Justification::centredTop);
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat().reduced(1);

        g.setColour(juce::Colour(0xFF252545));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(0xFF444466));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        // Arrow
        auto arrowBounds = juce::Rectangle<float>((float)buttonX, (float)buttonY, (float)buttonW, (float)buttonH);
        juce::Path arrow;
        arrow.addTriangle(arrowBounds.getCentreX() - 4, arrowBounds.getCentreY() - 2,
                          arrowBounds.getCentreX() + 4, arrowBounds.getCentreY() - 2,
                          arrowBounds.getCentreX(), arrowBounds.getCentreY() + 3);
        g.setColour(juce::Colour(0xFFCCCCDD));
        g.fillPath(arrow);
    }

    juce::Button* createDocumentWindowButton(int buttonType) override
    {
        auto* btn = juce::LookAndFeel_V4::createDocumentWindowButton(buttonType);

        if (auto* shapeBtn = dynamic_cast<juce::ShapeButton*>(btn))
        {
            // Make all buttons (minimize, maximize, close) white so they are visible on dark background
            shapeBtn->setColours(juce::Colour(0xFFFFFFFF).withAlpha(0.6f),
                                 juce::Colour(0xFFFFFFFF),
                                 juce::Colour(0xFFE94560)); // red tint when clicked
        }

        return btn;
    }
};
