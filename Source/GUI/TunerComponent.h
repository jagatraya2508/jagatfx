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
        // 1. Digital Screen Bezel & Background
        auto bounds = getLocalBounds().toFloat();
        
        // Outer dark bezel
        g.setColour(juce::Colour(0xFF0D0D15));
        g.fillRoundedRectangle(bounds, 8.0f);
        
        // Inner screen glow
        auto screenBounds = bounds.reduced(2.0f);
        juce::ColourGradient screenGrad(juce::Colour(0xFF151A25), screenBounds.getCentreX(), screenBounds.getY(),
                                        juce::Colour(0xFF0A0C12), screenBounds.getCentreX(), screenBounds.getBottom(), false);
        g.setGradientFill(screenGrad);
        g.fillRoundedRectangle(screenBounds, 6.0f);
        
        // Inner shadow effect
        g.setColour(juce::Colour(0x60000000));
        g.drawRoundedRectangle(screenBounds.reduced(1.0f), 5.0f, 2.0f);

        auto contentArea = screenBounds.reduced(8.0f);
        contentArea.removeFromLeft(60); // Space for button

        float scaleWidth = contentArea.getWidth() * 0.9f;
        float startX = contentArea.getCentreX() - scaleWidth / 2.0f;

        if (hasSignal)
        {
            float targetCents = currentCents;
            bool isInTune = std::abs(targetCents) < 5.0f;
            
            // Tuner colors
            juce::Colour baseColor = isInTune ? juce::Colour(0xFF00FFAA) : 
                                     (targetCents < 0 ? juce::Colour(0xFFFFB347) : juce::Colour(0xFFFF5555));
            juce::Colour darkColor = baseColor.withAlpha(0.2f);
            
            // Layout
            auto topHalf = contentArea.removeFromTop(contentArea.getHeight() * 0.45f);
            auto bottomHalf = contentArea;

            // --- NOTE NAME (Large Glowing Text) ---
            g.setFont(juce::Font(topHalf.getHeight() * 1.8f, juce::Font::bold));
            
            // Glow layer
            g.setColour(baseColor.withAlpha(0.4f));
            for (float offset = 1.0f; offset <= 3.0f; offset += 1.0f)
            {
                g.drawText(currentNoteName, topHalf.translated(0, offset).toNearestInt(), juce::Justification::centredRight);
                g.drawText(currentNoteName, topHalf.translated(0, -offset).toNearestInt(), juce::Justification::centredRight);
                g.drawText(currentNoteName, topHalf.translated(offset, 0).toNearestInt(), juce::Justification::centredRight);
                g.drawText(currentNoteName, topHalf.translated(-offset, 0).toNearestInt(), juce::Justification::centredRight);
            }
            // Core text
            g.setColour(baseColor);
            g.drawText(currentNoteName, topHalf.toNearestInt(), juce::Justification::centredRight);

            // --- CENTS READOUT ---
            g.setFont(juce::Font(topHalf.getHeight() * 0.5f, juce::Font::bold));
            juce::String centsStr = (targetCents > 0 ? "+" : "") + juce::String(targetCents, 1);
            if (isInTune) centsStr = "IN TUNE";
            g.setColour(isInTune ? baseColor : juce::Colours::white.withAlpha(0.8f));
            g.drawText(centsStr, topHalf.toNearestInt(), juce::Justification::centredLeft);

            // --- LED METER SCALE ---
            float midX = bottomHalf.getCentreX();
            float yPos = bottomHalf.getCentreY();
            
            // Draw background tick marks (-50 to +50)
            int numTicks = 21; // every 5 cents
            for (int i = 0; i < numTicks; ++i)
            {
                float normalized = (float)i / (numTicks - 1);
                float x = startX + normalized * scaleWidth;
                
                // Bigger tick for center and edges
                bool isCenter = (i == numTicks / 2);
                float h = isCenter ? 8.0f : 4.0f;
                
                g.setColour(juce::Colour(0x608888AA));
                g.fillRoundedRectangle(x - 1.0f, yPos - h/2.0f, 2.0f, h, 1.0f);
            }
            
            // --- GLOWING INDICATOR NEEDLE ---
            float mappedX = juce::jmap(displayCents, -50.0f, 50.0f, startX, startX + scaleWidth);
            mappedX = juce::jlimit(startX, startX + scaleWidth, mappedX);
            
            // Dynamic width based on how out of tune
            float needleWidth = isInTune ? 14.0f : 8.0f;
            
            juce::ColourGradient needleGrad(baseColor, mappedX, yPos - 8.0f, 
                                            baseColor.withAlpha(0.0f), mappedX, yPos + 8.0f, true);
            g.setGradientFill(needleGrad);
            g.fillRoundedRectangle(mappedX - needleWidth/2.0f, yPos - 12.0f, needleWidth, 24.0f, 4.0f);
            
            // Solid center
            g.setColour(baseColor.brighter(0.5f));
            g.fillRoundedRectangle(mappedX - 2.0f, yPos - 6.0f, 4.0f, 12.0f, 2.0f);
            
            // Small arrows pointing towards center if out of tune
            if (!isInTune)
            {
                g.setColour(baseColor);
                if (targetCents < 0) {
                    juce::Path arrow; arrow.addTriangle(mappedX + 12, yPos - 3, mappedX + 12, yPos + 3, mappedX + 18, yPos);
                    g.fillPath(arrow);
                } else {
                    juce::Path arrow; arrow.addTriangle(mappedX - 12, yPos - 3, mappedX - 12, yPos + 3, mappedX - 18, yPos);
                    g.fillPath(arrow);
                }
            }
        }
        else
        {
            // Idle "AWAITING SIGNAL" state
            // Use time to create a slow pulsing glow
            float pulse = std::abs(std::sin(juce::Time::getMillisecondCounterHiRes() * 0.002f));
            
            g.setFont(juce::Font(16.0f, juce::Font::bold));
            g.setColour(juce::Colour(0xFF4A5568).interpolatedWith(juce::Colour(0xFF718096), pulse));
            g.drawText("AWAITING SIGNAL...", contentArea.toNearestInt(), juce::Justification::centred);
            
            // Draw dimmed scale
            float yPos = contentArea.getBottom() - 15.0f;
            g.setColour(juce::Colour(0x208888AA));
            g.drawHorizontalLine((int)yPos, startX, startX + scaleWidth);
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
