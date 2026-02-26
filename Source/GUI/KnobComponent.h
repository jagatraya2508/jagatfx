#pragma once
#include <JuceHeader.h>

class KnobComponent : public juce::Component
{
public:
    KnobComponent(const juce::String& labelText, juce::AudioProcessorValueTreeState& apvts,
                  const juce::String& paramID)
        : label(labelText)
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 14);
        slider.setTextValueSuffix("");
        addAndMakeVisible(slider);

        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, paramID, slider);

        nameLabel.setText(labelText, juce::dontSendNotification);
        nameLabel.setJustificationType(juce::Justification::centredTop);
        nameLabel.setFont(juce::Font(11.0f));
        nameLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF8899AA));
        addAndMakeVisible(nameLabel);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        nameLabel.setBounds(bounds.removeFromTop(16));
        slider.setBounds(bounds);
    }

private:
    juce::Slider slider;
    juce::Label nameLabel;
    juce::String label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};
