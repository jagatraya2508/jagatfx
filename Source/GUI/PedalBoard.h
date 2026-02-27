#pragma once
#include <JuceHeader.h>
#include "EffectSlot.h"
#include "ParameterEditor.h"

class PedalBoard : public juce::Component
{
public:
    PedalBoard(juce::AudioProcessorValueTreeState& apvts)
    {
        // === PRE-EFFECTS ROW ===

        // Noise Gate
        noiseGate = std::make_unique<EffectSlot>(
            "NOISE GATE", juce::Colour(0xFF00BFA5), apvts, "gateEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"THRESH", "gateThreshold"}, {"ATTACK", "gateAttack"}, {"RELEASE", "gateRelease"}
            });
        addAndMakeVisible(*noiseGate);

        // Compressor
        compressor = std::make_unique<EffectSlot>(
            "COMPRESSOR", juce::Colour(0xFFFF6F00), apvts, "compEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"THRESH", "compThreshold"}, {"RATIO", "compRatio"},
                {"ATTACK", "compAttack"}, {"MAKEUP", "compMakeup"}
            },
            "compModel", juce::StringArray{"VCA", "Optical", "FET"});
        addAndMakeVisible(*compressor);

        // Overdrive
        overdrive = std::make_unique<EffectSlot>(
            "OVERDRIVE", juce::Colour(0xFF4CAF50), apvts, "odEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"DRIVE", "odDrive"}, {"TONE", "odTone"}, {"LEVEL", "odLevel"}
            },
            "odModel", juce::StringArray{"Tube Screamer", "Blues Driver", "Klon"});
        addAndMakeVisible(*overdrive);

        // Distortion
        distortion = std::make_unique<EffectSlot>(
            "DISTORTION", juce::Colour(0xFFF44336), apvts, "distEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"GAIN", "distGain"}, {"TONE", "distTone"}, {"LEVEL", "distLevel"}
            },
            "distModel", juce::StringArray{"DS-1", "RAT", "Metal Zone"});
        addAndMakeVisible(*distortion);

        // High Gain
        highGain = std::make_unique<EffectSlot>(
            "HIGH GAIN", juce::Colour(0xFF9C27B0), apvts, "hgEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"GAIN", "hgGain"}, {"TONE", "hgTone"}, {"LEVEL", "hgLevel"}
            },
            "hgModel", juce::StringArray{"Rectifier", "5150", "Dual Rec", "Djent"});
        addAndMakeVisible(*highGain);

        // === POST-EFFECTS ROW ===

        // Chorus
        chorus = std::make_unique<EffectSlot>(
            "CHORUS", juce::Colour(0xFF2196F3), apvts, "chorusEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"RATE", "chorusRate"}, {"DEPTH", "chorusDepth"}, {"MIX", "chorusMix"}
            });
        addAndMakeVisible(*chorus);

        // Flanger
        flanger = std::make_unique<EffectSlot>(
            "FLANGER", juce::Colour(0xFF00BCD4), apvts, "flangerEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"RATE", "flangerRate"}, {"DEPTH", "flangerDepth"},
                {"FB", "flangerFeedback"}, {"MIX", "flangerMix"}
            });
        addAndMakeVisible(*flanger);

        // Phaser
        phaser = std::make_unique<EffectSlot>(
            "PHASER", juce::Colour(0xFFAB47BC), apvts, "phaserEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"RATE", "phaserRate"}, {"DEPTH", "phaserDepth"},
                {"FB", "phaserFeedback"}, {"MIX", "phaserMix"}
            });
        addAndMakeVisible(*phaser);

        // Harmonizer
        harmonizer = std::make_unique<EffectSlot>(
            "HARMONIZER", juce::Colour(0xFFFF9800), apvts, "harmEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"MIX", "harmMix"}
            },
            "harmInterval", juce::StringArray{"Min 3rd", "Maj 3rd", "4th", "5th", "Oct Up", "Oct Down"});
        addAndMakeVisible(*harmonizer);

        // String Synth
        stringSynth = std::make_unique<EffectSlot>(
            "STRING SYNTH", juce::Colour(0xFFE91E63), apvts, "stringEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"ATTACK", "stringAttack"}, {"OCTAVE", "stringOctave"},
                {"BRIGHT", "stringBrightness"}, {"RES", "stringResonance"}, {"MIX", "stringMix"}
            });
        addAndMakeVisible(*stringSynth);

        // Delay
        delay = std::make_unique<EffectSlot>(
            "DELAY", juce::Colour(0xFF3F51B5), apvts, "delayEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"TIME", "delayTime"}, {"FB", "delayFeedback"}, {"MIX", "delayMix"}, {"MOD", "delayMod"}
            },
            "delayModel", juce::StringArray{"Digital", "Analog", "Tape", "Ping-Pong"});
        addAndMakeVisible(*delay);

        // Reverb
        reverb = std::make_unique<EffectSlot>(
            "REVERB", juce::Colour(0xFF607D8B), apvts, "reverbEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"SIZE", "reverbSize"}, {"DAMP", "reverbDamping"},
                {"PRE", "reverbPreDelay"}, {"MIX", "reverbMix"}
            },
            "reverbModel", juce::StringArray{"Hall", "Room", "Plate", "Spring", "Cathedral"});
        addAndMakeVisible(*reverb);

        // Parametric EQ
        parametricEQ = std::make_unique<EffectSlot>(
            "PARA EQ", juce::Colour(0xFF795548), apvts, "peqEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"F1", "peqFreq0"}, {"G1", "peqGain0"},
                {"F2", "peqFreq1"}, {"G2", "peqGain1"}
            });
        addAndMakeVisible(*parametricEQ);

        // Graphic EQ
        graphicEQ = std::make_unique<EffectSlot>(
            "GRAPHIC EQ", juce::Colour(0xFF8BC34A), apvts, "geqEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"125", "geqBand2"}, {"500", "geqBand4"},
                {"2K", "geqBand6"}, {"8K", "geqBand8"}
            });
        addAndMakeVisible(*graphicEQ);

        // Talk Box
        talkBox = std::make_unique<EffectSlot>(
            "TALK BOX", juce::Colour(0xFFE94560), apvts, "talkEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"VOWEL", "talkVowel"}, {"MIX", "talkMix"}
            });
        addAndMakeVisible(*talkBox);

        // Auto Wah
        autoWah = std::make_unique<EffectSlot>(
            "AUTO WAH", juce::Colour(0xFFFFEB3B), apvts, "autoWahEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"SENS", "autoWahSens"}, {"ATK", "autoWahAttack"},
                {"REL", "autoWahRelease"}, {"RANGE", "autoWahRange"}
            });
        addAndMakeVisible(*autoWah);

        // Section labels
        preLabel.setText("PRE-EFFECTS", juce::dontSendNotification);
        preLabel.setFont(juce::Font(12.0f, juce::Font::bold));
        preLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF8899AA));
        addAndMakeVisible(preLabel);

        postLabel.setText("POST-EFFECTS", juce::dontSendNotification);
        postLabel.setFont(juce::Font(12.0f, juce::Font::bold));
        postLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF8899AA));
        addAndMakeVisible(postLabel);

        modLabel.setText("MOD & SYNTH", juce::dontSendNotification);
        modLabel.setFont(juce::Font(12.0f, juce::Font::bold));
        modLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF8899AA));
        addAndMakeVisible(modLabel);

        // Parameter Editor
        editor = std::make_unique<ParameterEditor>(apvts);
        addAndMakeVisible(*editor);

        // Setup Selection Callbacks
        setupSelection(noiseGate.get(), "NOISE GATE", {{"THRESH", "gateThreshold"}, {"ATTACK", "gateAttack"}, {"RELEASE", "gateRelease"}});
        setupSelection(compressor.get(), "COMPRESSOR", {{"THRESH", "compThreshold"}, {"RATIO", "compRatio"}, {"ATTACK", "compAttack"}, {"MAKEUP", "compMakeup"}}, "compModel", {"VCA", "Optical", "FET"});
        setupSelection(overdrive.get(), "OVERDRIVE", {{"DRIVE", "odDrive"}, {"TONE", "odTone"}, {"LEVEL", "odLevel"}}, "odModel", {"Tube Screamer", "Blues Driver", "Klon"});
        setupSelection(distortion.get(), "DISTORTION", {{"GAIN", "distGain"}, {"TONE", "distTone"}, {"LEVEL", "distLevel"}}, "distModel", {"DS-1", "RAT", "Metal Zone"});
        setupSelection(highGain.get(), "HIGH GAIN", {{"GAIN", "hgGain"}, {"TONE", "hgTone"}, {"LEVEL", "hgLevel"}}, "hgModel", {"Rectifier", "5150", "Dual Rec", "Djent"});
        
        setupSelection(chorus.get(), "CHORUS", {{"RATE", "chorusRate"}, {"DEPTH", "chorusDepth"}, {"MIX", "chorusMix"}});
        setupSelection(flanger.get(), "FLANGER", {{"RATE", "flangerRate"}, {"DEPTH", "flangerDepth"}, {"FB", "flangerFeedback"}, {"MIX", "flangerMix"}});
        setupSelection(phaser.get(), "PHASER", {{"RATE", "phaserRate"}, {"DEPTH", "phaserDepth"}, {"FB", "phaserFeedback"}, {"MIX", "phaserMix"}});
        setupSelection(harmonizer.get(), "HARMONIZER", {{"MIX", "harmMix"}}, "harmInterval", {"Min 3rd", "Maj 3rd", "4th", "5th", "Oct Up", "Oct Down"});
        setupSelection(stringSynth.get(), "STRING SYNTH", {{"ATTACK", "stringAttack"}, {"OCTAVE", "stringOctave"}, {"BRIGHT", "stringBrightness"}, {"RES", "stringResonance"}, {"MIX", "stringMix"}});
        
        setupSelection(delay.get(), "DELAY", {{"TIME", "delayTime"}, {"FB", "delayFeedback"}, {"MIX", "delayMix"}, {"MOD", "delayMod"}}, "delayModel", {"Digital", "Analog", "Tape", "Ping-Pong"});
        setupSelection(reverb.get(), "REVERB", {{"SIZE", "reverbSize"}, {"DAMP", "reverbDamping"}, {"PRE", "reverbPreDelay"}, {"MIX", "reverbMix"}}, "reverbModel", {"Hall", "Room", "Plate", "Spring", "Cathedral"});
        setupSelection(parametricEQ.get(), "PARA EQ", {{"F1", "peqFreq0"}, {"G1", "peqGain0"}, {"F2", "peqFreq1"}, {"G2", "peqGain1"}});
        setupSelection(graphicEQ.get(), "GRAPHIC EQ", {{"125", "geqBand2"}, {"500", "geqBand4"}, {"2K", "geqBand6"}, {"8K", "geqBand8"}});
        setupSelection(talkBox.get(), "TALK BOX", {{"VOWEL", "talkVowel"}, {"MIX", "talkMix"}});
        setupSelection(autoWah.get(), "AUTO WAH", {{"SENS", "autoWahSens"}, {"ATK", "autoWahAttack"}, {"REL", "autoWahRelease"}, {"RANGE", "autoWahRange"}});

        // Select first effect by default
        noiseGate->onClick();
    }

    void setupSelection(EffectSlot* slot, const juce::String& name, 
                        std::vector<std::pair<juce::String, juce::String>> params,
                        juce::String modelId = "", juce::StringArray models = {})
    {
        slot->setCompactMode(true);
        slot->onClick = [this, slot, name, params, modelId, models]()
        {
            if (activeSlot) activeSlot->setSelected(false);
            activeSlot = slot;
            activeSlot->setSelected(true);
            editor->setEffectToEdit(name, params, modelId, models);
        };
    }

    void paint(juce::Graphics& g) override
    {
        // Pedalboard background (semi-transparent to show background image)
        g.setColour(juce::Colour(0xB00D0D1A));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 8.0f);

        // Signal flow arrows between rows
        g.setColour(juce::Colour(0xFF333355));
        int midY = getHeight() / 2;
        g.drawArrow(juce::Line<float>(20, (float)midY - 5, 20, (float)midY + 5), 1.5f, 6, 6);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced(8);
        
        // --- Bottom: The Evolution Chain (The Chain) ---
        // Give the slots a sleeker GP-200 fixed height
        int rowHeight = 45;
        auto chainArea = bounds.removeFromBottom(rowHeight * 3 + 12); // Add a bit more total room for gaps
        
        // Row 3 (Post Effects)
        auto row3 = chainArea.removeFromBottom(rowHeight).reduced(0, 3);
        postLabel.setBounds(row3.removeFromLeft(90));
        int slotWidthRow3 = row3.getWidth() / 6;
        delay->setBounds(row3.removeFromLeft(slotWidthRow3).reduced(1,0));
        reverb->setBounds(row3.removeFromLeft(slotWidthRow3).reduced(1,0));
        parametricEQ->setBounds(row3.removeFromLeft(slotWidthRow3).reduced(1,0));
        graphicEQ->setBounds(row3.removeFromLeft(slotWidthRow3).reduced(1,0));
        talkBox->setBounds(row3.removeFromLeft(slotWidthRow3).reduced(1,0));
        autoWah->setBounds(row3.reduced(1,0));

        // Row 2 (Mod & Synth)
        auto row2 = chainArea.removeFromBottom(rowHeight).reduced(0, 3);
        modLabel.setBounds(row2.removeFromLeft(90));
        int slotWidth5 = row2.getWidth() / 5;
        chorus->setBounds(row2.removeFromLeft(slotWidth5).reduced(1,0));
        flanger->setBounds(row2.removeFromLeft(slotWidth5).reduced(1,0));
        phaser->setBounds(row2.removeFromLeft(slotWidth5).reduced(1,0));
        harmonizer->setBounds(row2.removeFromLeft(slotWidth5).reduced(1,0));
        stringSynth->setBounds(row2.reduced(1,0));

        // Row 1 (Pre Effects)
        auto row1 = chainArea.removeFromBottom(rowHeight).reduced(0, 3);
        preLabel.setBounds(row1.removeFromLeft(90));
        noiseGate->setBounds(row1.removeFromLeft(slotWidth5).reduced(1,0));
        compressor->setBounds(row1.removeFromLeft(slotWidth5).reduced(1,0));
        overdrive->setBounds(row1.removeFromLeft(slotWidth5).reduced(1,0));
        distortion->setBounds(row1.removeFromLeft(slotWidth5).reduced(1,0));
        highGain->setBounds(row1.reduced(1,0));

        bounds.removeFromBottom(8); // Spacer between chain and editor

        // --- Top: The Inspector (The Editor) ---
        // Editor takes all remaining space at the top
        editor->setBounds(bounds);
    }

private:
    // Pre-effects
    std::unique_ptr<EffectSlot> noiseGate, compressor, overdrive, distortion, highGain;
    // Post-effects
    std::unique_ptr<EffectSlot> chorus, flanger, phaser, harmonizer, stringSynth, delay, reverb, parametricEQ, graphicEQ, talkBox, autoWah;

    std::unique_ptr<ParameterEditor> editor;
    EffectSlot* activeSlot = nullptr;

    juce::Label preLabel, modLabel, postLabel;
};
