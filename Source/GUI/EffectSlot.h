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
        auto bounds = getLocalBounds().toFloat().reduced(2.0f);

        // 1. Dark Background Structure
        juce::Colour bgColor = isSelected ? juce::Colour(0xFF252535) : juce::Colour(0xFF1E1E28);
        g.setColour(bgColor);
        g.fillRoundedRectangle(bounds, 4.0f);

        // 2. Colored Border (Thin)
        // If it's selected, draw a brighter outline to show it's being edited
        g.setColour(isSelected ? juce::Colours::white : accent.withAlpha(0.7f));
        g.drawRoundedRectangle(bounds.reduced(1.0f), 4.0f, isSelected ? 1.5f : 1.0f);

        // 3. Compact Mode Drawing (GP-200 Slot Style)
        if (isCompact)
        {
            // The Bypass LED indicator at the top
            bool isOn = bypassBtn.getToggleState();
            juce::Colour ledColor = isOn ? juce::Colours::limegreen : juce::Colour(0xFF333333);
            
            float ledShadow = isOn ? 8.0f : 0.0f;
            float ledSize = 6.0f;
            juce::Rectangle<float> ledArea(bounds.getCentreX() - ledSize/2, bounds.getY() + 8.0f, ledSize, ledSize);
            
            // LED Glow
            if (isOn)
            {
                g.setColour(ledColor.withAlpha(0.3f));
                g.fillEllipse(ledArea.expanded(ledShadow));
            }
            
            // LED Core
            g.setColour(ledColor);
            g.fillEllipse(ledArea);

            // Effect Name Text at bottom
            g.setColour(juce::Colours::white.withAlpha(isOn ? 0.9f : 0.5f));
            g.setFont(juce::Font(11.0f, juce::Font::bold));
            auto textBounds = bounds.withTrimmedTop(bounds.getHeight() * 0.4f); 
            g.drawText(effectName, textBounds, juce::Justification::centred);

            // Draw a subtle colored bar at the very top (optional aesthetic)
            g.setColour(accent.withAlpha(0.5f));
            g.fillRoundedRectangle(bounds.getX() + 6.0f, bounds.getY() + 1.5f, bounds.getWidth() - 12.0f, 2.0f, 1.0f);
        }
        else 
        {
            // Full editor mode backing drawing
            if (isSelected)
            {
                g.setColour(juce::Colours::white.withAlpha(0.05f));
                g.fillRoundedRectangle(bounds, 4.0f);
            }
        }
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced(6);

        if (isCompact)
        {
            // Make bypass button invisible but cover the entire top half of the slot
            // so the user can click anywhere on the upper half to bypass
            bypassBtn.setAlpha(0.0f); 
            bypassBtn.setBounds(bounds.removeFromTop(bounds.getHeight() / 2));
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
