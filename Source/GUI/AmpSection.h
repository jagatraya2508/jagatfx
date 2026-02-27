#pragma once
#include <JuceHeader.h>
#include "KnobComponent.h"
#include "MarshallLookAndFeel.h"

class AmpSection : public juce::Component
{
public:
    AmpSection(juce::AudioProcessorValueTreeState& apvts)
    {
        setLookAndFeel(&marshallLookAndFeel);
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

        // Amp controls (using bottom labels for Marshall style)
        inputGain = std::make_unique<KnobComponent>("BOOST", apvts, "inputGain", KnobComponent::LabelPosition::Bottom);
        ampGain = std::make_unique<KnobComponent>("PREAMP", apvts, "ampGain", KnobComponent::LabelPosition::Bottom);
        bass = std::make_unique<KnobComponent>("BASS", apvts, "tsBass", KnobComponent::LabelPosition::Bottom);
        mid = std::make_unique<KnobComponent>("MID", apvts, "tsMid", KnobComponent::LabelPosition::Bottom);
        treble = std::make_unique<KnobComponent>("TREBLE", apvts, "tsTreble", KnobComponent::LabelPosition::Bottom);
        presence = std::make_unique<KnobComponent>("PRESENCE", apvts, "paPresence", KnobComponent::LabelPosition::Bottom);
        resonance = std::make_unique<KnobComponent>("RES", apvts, "paResonance", KnobComponent::LabelPosition::Bottom);
        master = std::make_unique<KnobComponent>("MASTER", apvts, "paMaster", KnobComponent::LabelPosition::Bottom);
        outputGain = std::make_unique<KnobComponent>("OUT", apvts, "outputGain", KnobComponent::LabelPosition::Bottom);

        // Force label colors to black for visibility on the gold background
        auto blackColor = marshallLookAndFeel.getAmpLabelColour();
        inputGain->setLabelColour(blackColor);
        ampGain->setLabelColour(blackColor);
        bass->setLabelColour(blackColor);
        mid->setLabelColour(blackColor);
        treble->setLabelColour(blackColor);
        presence->setLabelColour(blackColor);
        resonance->setLabelColour(blackColor);
        master->setLabelColour(blackColor);
        outputGain->setLabelColour(blackColor);

        addAndMakeVisible(*inputGain);
        addAndMakeVisible(*ampGain);
        addAndMakeVisible(*bass);
        addAndMakeVisible(*mid);
        addAndMakeVisible(*treble);
        addAndMakeVisible(*presence);
        addAndMakeVisible(*resonance);
        addAndMakeVisible(*master);
        addAndMakeVisible(*outputGain);

        // Bypass toggles (Made invisible to sit over the LED areas)
        ampBypass.setAlpha(0.0f);
        ampBypass.setClickingTogglesState(true);
        addAndMakeVisible(ampBypass);
        ampBypassAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            apvts, "ampEnabled", ampBypass);

        cabBypass.setAlpha(0.0f);
        cabBypass.setClickingTogglesState(true);
        addAndMakeVisible(cabBypass);
        cabBypassAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            apvts, "cabEnabled", cabBypass);

        // Section labels (Not needed for Amp since we draw "LEAD 800", keeping only Cab)
        cabLabel.setText("CABINET", juce::dontSendNotification);
        cabLabel.setFont(juce::Font(14.0f, juce::Font::bold));
        cabLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF0F3460));
        cabLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(cabLabel);
    }

    ~AmpSection() override
    {
        setLookAndFeel(nullptr);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().reduced(2).toFloat();
        int ampWidth = bounds.getWidth() * 3 / 4 - 8;
        
        // Block boundaries
        juce::Rectangle<float> ampArea(bounds.getX(), bounds.getY(), (float)ampWidth, bounds.getHeight());
        juce::Rectangle<float> cabArea(ampArea.getRight() + 16.0f, bounds.getY(), bounds.getWidth() - ampWidth - 16.0f, bounds.getHeight());

        // --- DRAW AMP BLOCK (MARSHALL LEAD 800 STYLE) ---
        bool isAmpOn = ampBypass.getToggleState();
        float topPanelRatio = 0.5f;
        auto ampTopArea = ampArea.withHeight(ampArea.getHeight() * topPanelRatio);
        auto ampBotArea = ampArea.withTrimmedTop(ampTopArea.getHeight());

        // Fill Bottom (Gold/Brass)
        juce::ColourGradient goldGradient(
            juce::Colour(0xFFF2C86F), ampBotArea.getX(), ampBotArea.getY(),
            juce::Colour(0xFFC7983F), ampBotArea.getX(), ampBotArea.getBottom(), false);
        g.setGradientFill(goldGradient);
        g.fillRoundedRectangle(ampArea, 6.0f); // Fill whole base so corners round nice

        // Fill Top (Dark Tolex/Grill)
        g.setColour(juce::Colour(0xFF222222));
        g.fillRoundedRectangle(ampTopArea, 6.0f);
        g.fillRect(ampTopArea.withTrimmedTop(6.0f)); // Square off the bottom of the top panel to merge with gold

        // Add subtle grill texture to top panel
        g.setColour(juce::Colour(0x10FFFFFF));
        for (int i = 0; i < ampTopArea.getWidth(); i += 4) g.drawVerticalLine((int)ampTopArea.getX() + i, ampTopArea.getY(), ampTopArea.getBottom());
        for (int i = 0; i < ampTopArea.getHeight(); i += 4) g.drawHorizontalLine((int)ampTopArea.getY() + i, ampTopArea.getX(), ampTopArea.getRight());

        // "LEAD 800" Text
        g.setColour(isAmpOn ? juce::Colours::white : juce::Colour(0xFF888888));
        g.setFont(juce::Font(36.0f, juce::Font::bold));
        g.drawText("LEAD  800", ampTopArea, juce::Justification::centred);

        // Active Outline for Amp
        g.setColour(isAmpOn ? juce::Colours::white : juce::Colour(0xFFE94560).withAlpha(0.2f));
        g.drawRoundedRectangle(ampArea.reduced(1.0f), 6.0f, isAmpOn ? 2.0f : 1.0f);

        // Separator lines in the Gold bottom panel
        g.setColour(juce::Colour(0xFF553311).withAlpha(0.6f));
        int totalKnobs = 9;
        int sectionWidth = ampBotArea.getWidth() / totalKnobs;
        g.drawLine(ampBotArea.getX() + sectionWidth * 2, ampBotArea.getY() + 10, ampBotArea.getX() + sectionWidth * 2, ampBotArea.getBottom() - 10, 1.0f); // Boost/Preamp | EQ
        g.drawLine(ampBotArea.getX() + sectionWidth * 5, ampBotArea.getY() + 10, ampBotArea.getX() + sectionWidth * 5, ampBotArea.getBottom() - 10, 1.0f); // EQ | Presence

        // Amp LED (Pushed down a bit to sit on the dark panel corner)
        juce::Colour ampLedColor = isAmpOn ? juce::Colours::limegreen : juce::Colour(0xFF333333);
        float ledSize = 8.0f;
        juce::Rectangle<float> ampLedRect(ampArea.getX() + 8.0f, ampArea.getY() + 8.0f, ledSize, ledSize);
        if (isAmpOn) { g.setColour(ampLedColor.withAlpha(0.3f)); g.fillEllipse(ampLedRect.expanded(4.0f)); }
        g.setColour(ampLedColor);
        g.fillEllipse(ampLedRect);

        // --- DRAW CAB BLOCK ---
        juce::Colour bgColor = juce::Colour(0xFF1E1E28);
        g.setColour(bgColor);
        g.fillRoundedRectangle(cabArea, 6.0f);

        bool isCabOn = cabBypass.getToggleState();
        g.setColour(isCabOn ? juce::Colours::white : juce::Colour(0xFF3F51B5).withAlpha(0.4f));
        g.drawRoundedRectangle(cabArea.reduced(1.0f), 6.0f, isCabOn ? 1.5f : 1.0f);

        // Cab LED
        juce::Colour cabLedColor = isCabOn ? juce::Colours::limegreen : juce::Colour(0xFF333333);
        juce::Rectangle<float> cabLedRect(cabArea.getX() + 12.0f, cabArea.getY() + 12.0f, ledSize, ledSize);
        if (isCabOn) { g.setColour(cabLedColor.withAlpha(0.3f)); g.fillEllipse(cabLedRect.expanded(4.0f)); }
        g.setColour(cabLedColor);
        g.fillEllipse(cabLedRect);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced(2);
        int ampWidth = bounds.getWidth() * 3 / 4 - 8;
        
        auto ampArea = bounds.removeFromLeft(ampWidth);
        bounds.removeFromLeft(16); // Gap
        auto cabArea = bounds;

        // --- Amp Section Layout ---
        auto ampTop = ampArea.removeFromTop(ampArea.getHeight() / 2); // Split top and bottom exactly half
        
        // Invisible toggle over the LED area
        ampBypass.setBounds(ampTop.removeFromLeft(30).removeFromTop(30));
        
        // Push dropdowns down into the gold area or keep in top
        ampTop.removeFromTop(10); // Spacer
        ampTop.removeFromRight(10); // Spacer
        ampModelSelector.setBounds(ampTop.removeFromRight(150).reduced(0, 4));

        // Knob row in bottom gold half
        auto knobArea = ampArea.reduced(4, 4);
        int knobWidth = knobArea.getWidth() / 9;

        inputGain->setBounds(knobArea.removeFromLeft(knobWidth).reduced(2));
        ampGain->setBounds(knobArea.removeFromLeft(knobWidth).reduced(2));
        bass->setBounds(knobArea.removeFromLeft(knobWidth).reduced(2));
        mid->setBounds(knobArea.removeFromLeft(knobWidth).reduced(2));
        treble->setBounds(knobArea.removeFromLeft(knobWidth).reduced(2));
        presence->setBounds(knobArea.removeFromLeft(knobWidth).reduced(2));
        resonance->setBounds(knobArea.removeFromLeft(knobWidth).reduced(2));
        master->setBounds(knobArea.removeFromLeft(knobWidth).reduced(2));
        outputGain->setBounds(knobArea.removeFromLeft(knobWidth).reduced(2));

        // --- Cab Section Layout ---
        auto cabTop = cabArea.removeFromTop(32).reduced(4, 2);
        
        cabBypass.setBounds(cabTop.removeFromLeft(24));
        cabLabel.setBounds(cabTop.removeFromLeft(80));

        // Center the dropdowns nicely in the cab Area
        auto cabCenter = cabArea.reduced(10, 8);
        cabModelSelector.setBounds(cabCenter.removeFromTop(26).reduced(0, 2));
        cabCenter.removeFromTop(8); // gap
        micSelector.setBounds(cabCenter.removeFromTop(26).reduced(0, 2));
    }

private:
    juce::ComboBox ampModelSelector, cabModelSelector, micSelector;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> ampModelAttach, cabModelAttach, micAttach;

    std::unique_ptr<KnobComponent> inputGain, ampGain, bass, mid, treble, presence, resonance, master, outputGain;

    juce::ToggleButton ampBypass, cabBypass;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> ampBypassAttach, cabBypassAttach;

    juce::Label cabLabel;
    
    MarshallLookAndFeel marshallLookAndFeel;
};
