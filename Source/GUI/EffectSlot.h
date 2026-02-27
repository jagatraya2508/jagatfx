#pragma once
#include <JuceHeader.h>
#include "KnobComponent.h"

class EffectSlot : public juce::Component
{
public:
    EffectSlot(const juce::String& name, const juce::String& shortLbl, juce::Colour accentColor,
               juce::AudioProcessorValueTreeState& apvts,
               const juce::String& enableParamID,
               const std::vector<std::pair<juce::String, juce::String>>& knobParams,
               const juce::String& modelParamID = "",
               const juce::StringArray& modelNames = {})
        : effectName(name), shortName(shortLbl), accent(accentColor)
    {
        // Bypass toggle
        bypassBtn.setClickingTogglesState(true);
        addAndMakeVisible(bypassBtn);
        bypassAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            apvts, enableParamID, bypassBtn);
            
        bypassBtn.onClick = [this] { if (onClick) onClick(); };

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
        // Hide model selector in compact mode
        if (hasModel) modelSelector.setVisible(!compact);
        resized();
    }

    void setSelected(bool selected) { isSelected = selected; repaint(); }
    bool getSelected() const { return isSelected; }

    std::function<void()> onClick;

    void mouseDown(const juce::MouseEvent& e) override
    {
        // When the user clicks the pedal body (not the bypass toggle area)
        // trigger the selection callback.
        if (onClick) onClick();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2.0f);

        if (isCompact)
        {
            // --- GP-200 ICON STYLE ---
            bool isOn = bypassBtn.getToggleState();

            // 1. Category Pill (Top)
            juce::Rectangle<float> pillArea(bounds.getCentreX() - 16.0f, bounds.getY(), 32.0f, 14.0f);
            g.setColour(isOn ? juce::Colour(0xFF1E88E5) : juce::Colour(0xFF333344));
            g.fillRoundedRectangle(pillArea, 4.0f);
            g.setColour(isOn ? juce::Colours::white : juce::Colour(0xFF888899));
            g.setFont(juce::Font(10.0f, juce::Font::bold));
            g.drawText(shortName, pillArea, juce::Justification::centred);

            // 2. Pedal Mini-Icon (Middle)
            float iconTempW = 34.0f;
            float iconTempH = 48.0f;
            juce::Rectangle<float> iconArea(bounds.getCentreX() - iconTempW/2, pillArea.getBottom() + 6.0f, iconTempW, iconTempH);
            
            // Selection Glow
            if (isSelected)
            {
                g.setColour(juce::Colour(0xFF4488FF).withAlpha(0.6f));
                g.fillRoundedRectangle(iconArea.expanded(4.0f), 4.0f);
            }

            // Pedal Base Color
            g.setColour(accent.withAlpha(isOn ? 1.0f : 0.4f)); // Dim when bypassed
            g.fillRoundedRectangle(iconArea, 4.0f);
            
            // Pedal Top Highlight / 3D feel
            g.setColour(juce::Colours::white.withAlpha(isOn ? 0.2f : 0.05f));
            g.fillRoundedRectangle(iconArea.removeFromTop(8.0f), 4.0f);

            // Fake LED on pedal
            juce::Rectangle<float> ledArea(iconArea.getCentreX() - 2.0f, iconArea.getY() + 4.0f, 4.0f, 4.0f);
            g.setColour(isOn ? juce::Colours::red : juce::Colour(0xFF331111));
            if (isOn) { g.fillEllipse(ledArea.expanded(2.0f)); }
            g.fillEllipse(ledArea);

            // Fake Footswitch
            g.setColour(juce::Colour(0xFF222222));
            g.fillEllipse(iconArea.getCentreX() - 4.0f, iconArea.getBottom() - 10.0f, 8.0f, 8.0f);

            // Effect Short Label on the pedal
            g.setColour(juce::Colours::white.withAlpha(isOn ? 0.9f : 0.4f));
            g.setFont(juce::Font(9.0f, juce::Font::bold));
            g.drawText(shortName, iconArea.withTrimmedBottom(14.0f), juce::Justification::centredBottom);
        }
        else 
        {
            // Full editor mode backing
            juce::Colour bgColor = isSelected ? juce::Colour(0xFF252535) : juce::Colour(0xFF1E1E28);
            g.setColour(bgColor);
            g.fillRoundedRectangle(bounds, 4.0f);

            g.setColour(isSelected ? juce::Colours::white : accent.withAlpha(0.7f));
            g.drawRoundedRectangle(bounds.reduced(1.0f), 4.0f, isSelected ? 1.5f : 1.0f);

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
            // Position bypass button ONLY over the top pill area
            juce::Rectangle<float> fBounds = getLocalBounds().toFloat().reduced(2.0f);
            juce::Rectangle<float> pillArea(fBounds.getCentreX() - 16.0f, fBounds.getY(), 32.0f, 14.0f);
            
            bypassBtn.setAlpha(0.0f); 
            bypassBtn.setBounds(pillArea.toNearestInt());
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
    juce::String shortName;
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
