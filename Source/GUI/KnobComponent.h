#pragma once
#include <JuceHeader.h>

class KnobComponent : public juce::Component
{
public:
    enum class LabelPosition { Top, Bottom };

    KnobComponent(const juce::String& labelText, juce::AudioProcessorValueTreeState& apvts,
                  const juce::String& paramID, LabelPosition pos = LabelPosition::Top)
        : label(labelText), labelPos(pos)
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 14);
        slider.setTextValueSuffix("");
        addAndMakeVisible(slider);

        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, paramID, slider);

        nameLabel.setText(labelText, juce::dontSendNotification);
        nameLabel.setJustificationType(labelPos == LabelPosition::Top ? juce::Justification::centredTop : juce::Justification::centredBottom);
        nameLabel.setFont(juce::Font(12.0f, juce::Font::bold)); // A bit bolder to look like silkscreened text
        nameLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF8899AA));
        addAndMakeVisible(nameLabel);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        if (labelPos == LabelPosition::Top)
        {
            nameLabel.setBounds(bounds.removeFromTop(18));
            slider.setBounds(bounds);
        }
        else
        {
            nameLabel.setBounds(bounds.removeFromBottom(18));
            slider.setBounds(bounds);
        }
    }
    
    // Give external components a way to force label text color
    void setLabelColour(juce::Colour newColour)
    {
        nameLabel.setColour(juce::Label::textColourId, newColour);
    }

private:
    juce::Slider slider;
    juce::Label nameLabel;
    juce::String label;
    LabelPosition labelPos;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};
