#pragma once
#include <JuceHeader.h>
#include "KnobComponent.h"

class EffectSlot : public juce::Component
{
public:
    EffectSlot(const juce::String& name, juce::Colour accentColor,
               juce::AudioProcessorValueTreeState& apvts,
               const juce::String& enableParamID,
               const std::vector<std::pair<juce::String, juce::String>>& knobParams,
               const juce::String& modelParamID = "",
               const juce::StringArray& modelNames = {})
        : effectName(name), accent(accentColor)
    {
        // Bypass toggle
        bypassBtn.setClickingTogglesState(true);
        addAndMakeVisible(bypassBtn);
        bypassAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            apvts, enableParamID, bypassBtn);

        // Model selector
        if (modelParamID.isNotEmpty() && modelNames.size() > 0)
        {
            modelSelector.addItemList(modelNames, 1);
            addAndMakeVisible(modelSelector);
            modelAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
                apvts, modelParamID, modelSelector);
            hasModel = true;
        }

        // Knobs
        for (auto& [label, paramId] : knobParams)
        {
            auto knob = std::make_unique<KnobComponent>(label, apvts, paramId);
            addAndMakeVisible(*knob);
            knobs.push_back(std::move(knob));
        }
    }

    void setCompactMode(bool compact) 
    { 
        isCompact = compact; 
        if (compact) bypassBtn.setButtonText("");
        else bypassBtn.setButtonText(effectName);

        for (auto& k : knobs) k->setVisible(!compact);
        if (hasModel) modelSelector.setVisible(!compact);
        resized();
    }

    void setSelected(bool selected) { isSelected = selected; repaint(); }
    bool getSelected() const { return isSelected; }

    std::function<void()> onClick;

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (onClick) onClick();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2);

        // Pedal background (semi-transparent)
        juce::ColourGradient gradient(
            juce::Colour(0xB01E1E3A), bounds.getX(), bounds.getY(),
            juce::Colour(0xB0151530), bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(bounds, 6.0f);

        // Accent border
        g.setColour(isSelected ? juce::Colours::white : accent.withAlpha(0.6f));
        g.drawRoundedRectangle(bounds.reduced(1), 6.0f, isSelected ? 2.5f : 1.5f);

        // Top accent strip
        g.setColour(accent);
        g.fillRoundedRectangle(bounds.getX() + 1, bounds.getY() + 1, bounds.getWidth() - 2, 3.0f, 2.0f);

        if (isCompact)
        {
            g.setColour(juce::Colour(0xFFAABBCC));
            g.setFont(juce::Font(12.0f, juce::Font::bold));
            // Draw text centered below the LED
            auto textBounds = bounds.withTrimmedTop(25); 
            g.drawText(effectName, textBounds, juce::Justification::centred);
        }

        if (isSelected)
        {
            g.setColour(juce::Colours::white.withAlpha(0.1f));
            g.fillRoundedRectangle(bounds, 6.0f);
        }
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced(6);

        if (isCompact)
        {
            // Bypass button specifically small and at top center
            bypassBtn.setBounds(bounds.removeFromTop(20).withSizeKeepingCentre(60, 20));
            return;
        }

        // Toggle at top
        bypassBtn.setBounds(bounds.removeFromTop(28).reduced(2));

        // Model selector if present
        if (hasModel)
            modelSelector.setBounds(bounds.removeFromTop(22).reduced(2));

        // Knobs
        if (!knobs.empty())
        {
            int knobWidth = bounds.getWidth() / juce::jmax(1, (int)knobs.size());
            for (auto& knob : knobs)
                knob->setBounds(bounds.removeFromLeft(knobWidth));
        }
    }

private:
    juce::String effectName;
    juce::Colour accent;
    bool hasModel = false;
    bool isCompact = false;
    bool isSelected = false;

    juce::ToggleButton bypassBtn;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttach;

    juce::ComboBox modelSelector;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modelAttach;

    std::vector<std::unique_ptr<KnobComponent>> knobs;
};
