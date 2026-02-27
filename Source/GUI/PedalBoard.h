#pragma once
#include <JuceHeader.h>
#include "EffectSlot.h"
#include "ParameterEditor.h"

class PedalBoard : public juce::Component
{
public:
    PedalBoard(juce::AudioProcessorValueTreeState& apvts)
    {
        // === EFFECTS ROW 1 ===
        noiseGate = std::make_unique<EffectSlot>(
            "NOISE GATE", "NG", juce::Colour(0xFF00BFA5), apvts, "gateEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"THRESH", "gateThreshold"}, {"ATTACK", "gateAttack"}, {"RELEASE", "gateRelease"}
            });
        addAndMakeVisible(*noiseGate);

        compressor = std::make_unique<EffectSlot>(
            "COMPRESSOR", "CMP", juce::Colour(0xFFFF6F00), apvts, "compEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"THRESH", "compThreshold"}, {"RATIO", "compRatio"},
                {"ATTACK", "compAttack"}, {"MAKEUP", "compMakeup"}
            },
            "compModel", juce::StringArray{"VCA", "Optical", "FET"});
        addAndMakeVisible(*compressor);

        overdrive = std::make_unique<EffectSlot>(
            "OVERDRIVE", "OD", juce::Colour(0xFF4CAF50), apvts, "odEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"DRIVE", "odDrive"}, {"TONE", "odTone"}, {"LEVEL", "odLevel"}
            },
            "odModel", juce::StringArray{"Tube Screamer", "Blues Driver", "Klon"});
        addAndMakeVisible(*overdrive);

        distortion = std::make_unique<EffectSlot>(
            "DISTORTION", "DST", juce::Colour(0xFFF44336), apvts, "distEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"GAIN", "distGain"}, {"TONE", "distTone"}, {"LEVEL", "distLevel"}
            },
            "distModel", juce::StringArray{"DS-1", "RAT", "Metal Zone"});
        addAndMakeVisible(*distortion);

        highGain = std::make_unique<EffectSlot>(
            "HIGH GAIN", "HG", juce::Colour(0xFF9C27B0), apvts, "hgEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"GAIN", "hgGain"}, {"TONE", "hgTone"}, {"LEVEL", "hgLevel"}
            },
            "hgModel", juce::StringArray{"Rectifier", "5150", "Dual Rec", "Djent"});
        addAndMakeVisible(*highGain);

        autoWah = std::make_unique<EffectSlot>(
            "AUTO WAH", "WAH", juce::Colour(0xFFFFEB3B), apvts, "autoWahEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"SENS", "autoWahSens"}, {"ATK", "autoWahAttack"},
                {"REL", "autoWahRelease"}, {"RANGE", "autoWahRange"}
            });
        addAndMakeVisible(*autoWah);

        parametricEQ = std::make_unique<EffectSlot>(
            "PARA EQ", "PEQ", juce::Colour(0xFF795548), apvts, "peqEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"F1", "peqFreq0"}, {"G1", "peqGain0"},
                {"F2", "peqFreq1"}, {"G2", "peqGain1"}
            });
        addAndMakeVisible(*parametricEQ);

        graphicEQ = std::make_unique<EffectSlot>(
            "GRAPHIC EQ", "GEQ", juce::Colour(0xFF8BC34A), apvts, "geqEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"125", "geqBand2"}, {"500", "geqBand4"},
                {"2K", "geqBand6"}, {"8K", "geqBand8"}
            });
        addAndMakeVisible(*graphicEQ);

        // === EFFECTS ROW 2 ===
        talkBox = std::make_unique<EffectSlot>(
            "TALK BOX", "TLK", juce::Colour(0xFFE94560), apvts, "talkEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"VOWEL", "talkVowel"}, {"MIX", "talkMix"}
            });
        addAndMakeVisible(*talkBox);

        chorus = std::make_unique<EffectSlot>(
            "CHORUS", "CHO", juce::Colour(0xFF2196F3), apvts, "chorusEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"RATE", "chorusRate"}, {"DEPTH", "chorusDepth"}, {"MIX", "chorusMix"}
            });
        addAndMakeVisible(*chorus);

        flanger = std::make_unique<EffectSlot>(
            "FLANGER", "FLG", juce::Colour(0xFF00BCD4), apvts, "flangerEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"RATE", "flangerRate"}, {"DEPTH", "flangerDepth"},
                {"FB", "flangerFeedback"}, {"MIX", "flangerMix"}
            });
        addAndMakeVisible(*flanger);

        phaser = std::make_unique<EffectSlot>(
            "PHASER", "PHS", juce::Colour(0xFFAB47BC), apvts, "phaserEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"RATE", "phaserRate"}, {"DEPTH", "phaserDepth"},
                {"FB", "phaserFeedback"}, {"MIX", "phaserMix"}
            });
        addAndMakeVisible(*phaser);

        harmonizer = std::make_unique<EffectSlot>(
            "HARMONIZER", "HRM", juce::Colour(0xFFFF9800), apvts, "harmEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"MIX", "harmMix"}
            },
            "harmInterval", juce::StringArray{"Min 3rd", "Maj 3rd", "4th", "5th", "Oct Up", "Oct Down"});
        addAndMakeVisible(*harmonizer);

        stringSynth = std::make_unique<EffectSlot>(
            "STRING SYNTH", "SYN", juce::Colour(0xFFE91E63), apvts, "stringEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"ATTACK", "stringAttack"}, {"OCTAVE", "stringOctave"},
                {"BRIGHT", "stringBrightness"}, {"RES", "stringResonance"}, {"MIX", "stringMix"}
            });
        addAndMakeVisible(*stringSynth);

        delay = std::make_unique<EffectSlot>(
            "DELAY", "DLY", juce::Colour(0xFF3F51B5), apvts, "delayEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"TIME", "delayTime"}, {"FB", "delayFeedback"}, {"MIX", "delayMix"}, {"MOD", "delayMod"}
            },
            "delayModel", juce::StringArray{"Digital", "Analog", "Tape", "Ping-Pong"});
        addAndMakeVisible(*delay);

        reverb = std::make_unique<EffectSlot>(
            "REVERB", "RVB", juce::Colour(0xFF607D8B), apvts, "reverbEnabled",
            std::vector<std::pair<juce::String, juce::String>>{
                {"SIZE", "reverbSize"}, {"DAMP", "reverbDamping"},
                {"PRE", "reverbPreDelay"}, {"MIX", "reverbMix"}
            },
            "reverbModel", juce::StringArray{"Hall", "Room", "Plate", "Spring", "Cathedral"});
        addAndMakeVisible(*reverb);

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
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced(8);
        
        // Editor takes top space
        // Chain takes bottom space
        int rowHeight = 90; // enough for tall GP200 slots + dropdowns
        auto chainArea = bounds.removeFromBottom(rowHeight * 2 + 12);
        
        bounds.removeFromBottom(8); // Spacer between chain and editor
        editor->setBounds(bounds);

        // Row 1 (Effects 1-8)
        auto row1 = chainArea.removeFromTop(rowHeight).reduced(0, 3);
        int slotWidth = row1.getWidth() / 8;
        noiseGate->setBounds(row1.removeFromLeft(slotWidth).reduced(2,0));
        compressor->setBounds(row1.removeFromLeft(slotWidth).reduced(2,0));
        overdrive->setBounds(row1.removeFromLeft(slotWidth).reduced(2,0));
        distortion->setBounds(row1.removeFromLeft(slotWidth).reduced(2,0));
        highGain->setBounds(row1.removeFromLeft(slotWidth).reduced(2,0));
        autoWah->setBounds(row1.removeFromLeft(slotWidth).reduced(2,0));
        parametricEQ->setBounds(row1.removeFromLeft(slotWidth).reduced(2,0));
        graphicEQ->setBounds(row1.reduced(2,0));

        chainArea.removeFromTop(6); // gap

        // Row 2 (Effects 9-16)
        auto row2 = chainArea.removeFromTop(rowHeight).reduced(0, 3);
        talkBox->setBounds(row2.removeFromLeft(slotWidth).reduced(2,0));
        chorus->setBounds(row2.removeFromLeft(slotWidth).reduced(2,0));
        flanger->setBounds(row2.removeFromLeft(slotWidth).reduced(2,0));
        phaser->setBounds(row2.removeFromLeft(slotWidth).reduced(2,0));
        harmonizer->setBounds(row2.removeFromLeft(slotWidth).reduced(2,0));
        stringSynth->setBounds(row2.removeFromLeft(slotWidth).reduced(2,0));
        delay->setBounds(row2.removeFromLeft(slotWidth).reduced(2,0));
        reverb->setBounds(row2.reduced(2,0));
    }

private:
    // Pre-effects
    std::unique_ptr<EffectSlot> noiseGate, compressor, overdrive, distortion, highGain;
    // Post-effects
    std::unique_ptr<EffectSlot> chorus, flanger, phaser, harmonizer, stringSynth, delay, reverb, parametricEQ, graphicEQ, talkBox, autoWah;

    std::unique_ptr<ParameterEditor> editor;
    EffectSlot* activeSlot = nullptr;

    // juce::Label preLabel, modLabel, postLabel; // Deprecated Labels
};
