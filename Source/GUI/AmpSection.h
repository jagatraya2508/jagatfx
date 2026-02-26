#pragma once
#include <JuceHeader.h>
#include "KnobComponent.h"

class AmpSection : public juce::Component
{
public:
    AmpSection(juce::AudioProcessorValueTreeState& apvts)
    {
        // Amp model selector
        ampModelSelector.addItemList(
            juce::StringArray{"Clean", "Crunch", "High Gain", "Metal",
                             "Fender Twin", "Marshall JCM", "Mesa Rectifier", "Soldano Lead"}, 1);
        addAndMakeVisible(ampModelSelector);
        ampModelAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            apvts, "ampModel", ampModelSelector);

        // Cab model selector
        cabModelSelector.addItemList(
            juce::StringArray{"1x12 Open Back", "2x12 Closed", "4x12 V30", "4x12 Greenback", "Custom IR"}, 1);
        addAndMakeVisible(cabModelSelector);
        cabModelAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            apvts, "cabModel", cabModelSelector);

        // Mic position
        micSelector.addItemList(juce::StringArray{"On-Axis", "Off-Axis", "Edge", "Room"}, 1);
        addAndMakeVisible(micSelector);
        micAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            apvts, "cabMic", micSelector);

        // Amp controls
        inputGain = std::make_unique<KnobComponent>("INPUT", apvts, "inputGain");
        ampGain = std::make_unique<KnobComponent>("GAIN", apvts, "ampGain");
        bass = std::make_unique<KnobComponent>("BASS", apvts, "tsBass");
        mid = std::make_unique<KnobComponent>("MID", apvts, "tsMid");
        treble = std::make_unique<KnobComponent>("TREBLE", apvts, "tsTreble");
        presence = std::make_unique<KnobComponent>("PRESENCE", apvts, "paPresence");
        resonance = std::make_unique<KnobComponent>("RESONANCE", apvts, "paResonance");
        master = std::make_unique<KnobComponent>("MASTER", apvts, "paMaster");
        outputGain = std::make_unique<KnobComponent>("OUTPUT", apvts, "outputGain");

        addAndMakeVisible(*inputGain);
        addAndMakeVisible(*ampGain);
        addAndMakeVisible(*bass);
        addAndMakeVisible(*mid);
        addAndMakeVisible(*treble);
        addAndMakeVisible(*presence);
        addAndMakeVisible(*resonance);
        addAndMakeVisible(*master);
        addAndMakeVisible(*outputGain);

        // Bypass toggles
        ampBypass.setButtonText("AMP");
        ampBypass.setClickingTogglesState(true);
        addAndMakeVisible(ampBypass);
        ampBypassAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            apvts, "ampEnabled", ampBypass);

        cabBypass.setButtonText("CAB");
        cabBypass.setClickingTogglesState(true);
        addAndMakeVisible(cabBypass);
        cabBypassAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            apvts, "cabEnabled", cabBypass);

        // Section labels
        ampLabel.setText("AMPLIFIER", juce::dontSendNotification);
        ampLabel.setFont(juce::Font(14.0f, juce::Font::bold));
        ampLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFE94560));
        ampLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(ampLabel);

        cabLabel.setText("CABINET", juce::dontSendNotification);
        cabLabel.setFont(juce::Font(14.0f, juce::Font::bold));
        cabLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF0F3460));
        cabLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(cabLabel);
    }

    void paint(juce::Graphics& g) override
    {
        // Background gradient (semi-transparent to show background image)
        juce::ColourGradient gradient(
            juce::Colour(0xB016213E), 0, 0,
            juce::Colour(0xB01A1A2E), 0, (float)getHeight(), false);
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 8.0f);

        // Border
        g.setColour(juce::Colour(0xFF333355));
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1), 8.0f, 1.5f);

        // Divider between amp and cab
        int divX = getWidth() * 3 / 4;
        g.setColour(juce::Colour(0xFF333355));
        g.drawLine((float)divX, 10.0f, (float)divX, (float)getHeight() - 10.0f, 1.0f);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced(8);
        int ampWidth = bounds.getWidth() * 3 / 4;
        int cabWidth = bounds.getWidth() - ampWidth;

        auto ampArea = bounds.removeFromLeft(ampWidth);
        auto cabArea = bounds;

        // Amp section
        auto ampTop = ampArea.removeFromTop(28);
        ampLabel.setBounds(ampTop.removeFromLeft(100));
        ampModelSelector.setBounds(ampTop.removeFromLeft(160).reduced(2));
        ampBypass.setBounds(ampTop.removeFromLeft(40).reduced(2));

        // Knob row
        auto knobArea = ampArea.reduced(4);
        int knobWidth = knobArea.getWidth() / 9;
        int knobHeight = knobArea.getHeight();

        inputGain->setBounds(knobArea.removeFromLeft(knobWidth).reduced(2));
        ampGain->setBounds(knobArea.removeFromLeft(knobWidth).reduced(2));
        bass->setBounds(knobArea.removeFromLeft(knobWidth).reduced(2));
        mid->setBounds(knobArea.removeFromLeft(knobWidth).reduced(2));
        treble->setBounds(knobArea.removeFromLeft(knobWidth).reduced(2));
        presence->setBounds(knobArea.removeFromLeft(knobWidth).reduced(2));
        resonance->setBounds(knobArea.removeFromLeft(knobWidth).reduced(2));
        master->setBounds(knobArea.removeFromLeft(knobWidth).reduced(2));
        outputGain->setBounds(knobArea.removeFromLeft(knobWidth).reduced(2));

        // Cab section
        auto cabTop = cabArea.removeFromTop(28);
        cabLabel.setBounds(cabTop.removeFromLeft(80));
        cabBypass.setBounds(cabTop.removeFromRight(40).reduced(2));

        cabModelSelector.setBounds(cabArea.removeFromTop(26).reduced(4, 2));
        micSelector.setBounds(cabArea.removeFromTop(26).reduced(4, 2));
    }

private:
    juce::ComboBox ampModelSelector, cabModelSelector, micSelector;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> ampModelAttach, cabModelAttach, micAttach;

    std::unique_ptr<KnobComponent> inputGain, ampGain, bass, mid, treble, presence, resonance, master, outputGain;

    juce::ToggleButton ampBypass, cabBypass;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> ampBypassAttach, cabBypassAttach;

    juce::Label ampLabel, cabLabel;
};
