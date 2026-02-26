#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"

class TunerComponent : public juce::Component, public juce::Timer
{
public:
    TunerComponent(GuitarMultiFXProcessor& p) : processor(p)
    {
        tunerButton.setButtonText("TUNER");
        tunerButton.setClickingTogglesState(true);
        addAndMakeVisible(tunerButton);
        
        tunerAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            processor.getAPVTS(), "tunerEnabled", tunerButton);

        startTimerHz(30); // 30 fps refresh
    }

    ~TunerComponent() override
    {
        stopTimer();
    }

    void timerCallback() override
    {
        float freq = processor.currentTunerFreq.load(std::memory_order_relaxed);
        
        if (freq > 20.0f && freq < 2000.0f)
        {
            // Calculate note and cents
            // MIDI note 69 is A4 (440 Hz)
            float pitchInSemitones = 12.0f * std::log2(freq / 440.0f) + 69.0f;
            int midiNote = juce::roundToInt(pitchInSemitones);
            currentCents = (pitchInSemitones - midiNote) * 100.0f;
            
            // Map to note name
            const char* notes[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
            int noteIndex = midiNote % 12;
            if (noteIndex < 0) noteIndex += 12; // Safety
            
            currentNoteName = notes[noteIndex];
            hasSignal = true;

            // Smooth the needle
            displayCents = displayCents * 0.7f + currentCents * 0.3f;
        }
        else
        {
            hasSignal = false;
            displayCents = displayCents * 0.9f; // Drift back to center
        }

        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        // Background
        g.setColour(juce::Colour(0xFF16213E));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 6.0f);
        
        g.setColour(juce::Colour(0xFF333355));
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1), 6.0f, 1.0f);

        auto bounds = getLocalBounds().reduced(4);
        bounds.removeFromLeft(70); // Space for the toggle button

        if (hasSignal)
        {
            // Draw note name
            g.setFont(juce::Font(24.0f, juce::Font::bold));
            
            // Color based on tuning accuracy (green if within 5 cents, else red/yellow)
            juce::Colour textColour = std::abs(currentCents) < 5.0f ? 
                                      juce::Colour(0xFF00FF88) : juce::Colour(0xFFFF5555);
            
            g.setColour(textColour);
            g.drawText(currentNoteName, bounds.removeFromTop(30), juce::Justification::centred);

            // Draw tuning meter
            auto meterBounds = bounds.reduced(10, 5);
            float midX = meterBounds.getCentreX();
            float yPos = meterBounds.getCentreY();
            
            // Center line
            g.setColour(juce::Colour(0xFF8888AA));
            g.drawLine(midX, meterBounds.getY(), midX, meterBounds.getBottom(), 2.0f);

            // Needle
            // Map -50..+50 cents to the width of the meter
            float mappedX = juce::jmap(displayCents, -50.0f, 50.0f, 
                                       (float)meterBounds.getX(), (float)meterBounds.getRight());
            
            // Clamp
            mappedX = juce::jlimit((float)meterBounds.getX(), (float)meterBounds.getRight(), mappedX);

            g.setColour(textColour);
            g.fillEllipse(mappedX - 4.0f, yPos - 4.0f, 8.0f, 8.0f);
            g.drawLine(mappedX, meterBounds.getY(), mappedX, meterBounds.getBottom(), 3.0f);
        }
        else
        {
            g.setFont(juce::Font(14.0f));
            g.setColour(juce::Colour(0xFF555577));
            g.drawText("AWAITING SIGNAL...", bounds, juce::Justification::centred);
        }
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced(4);
        tunerButton.setBounds(bounds.removeFromLeft(60).reduced(2));
    }

private:
    GuitarMultiFXProcessor& processor;

    juce::String currentNoteName{ "-" };
    float currentCents{ 0.0f };
    float displayCents{ 0.0f }; // Smoothed for needle
    bool hasSignal{ false };

    juce::ToggleButton tunerButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> tunerAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TunerComponent)
};
