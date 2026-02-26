#pragma once
#include <JuceHeader.h>
#include "DSP/NoiseGate.h"
#include "DSP/Preamp.h"
#include "DSP/ToneStack.h"
#include "DSP/PowerAmp.h"
#include "DSP/CabinetSim.h"
#include "DSP/Overdrive.h"
#include "DSP/Distortion.h"
#include "DSP/HighGainDist.h"
#include "DSP/Delay.h"
#include "DSP/ReverbEffect.h"
#include "DSP/Chorus.h"
#include "DSP/Flanger.h"
#include "DSP/Phaser.h"
#include "DSP/Harmonizer.h"
#include "DSP/StringSynth.h"
#include "DSP/Compressor.h"
#include "DSP/ParametricEQ.h"
#include "DSP/GraphicEQ.h"
#include "DSP/TalkBox.h"
#include "DSP/AutoWah.h"
#include "DSP/Tuner.h"
#include <atomic>

class GuitarMultiFXProcessor : public juce::AudioProcessor
{
public:
    GuitarMultiFXProcessor();
    ~GuitarMultiFXProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    // DSP Modules - accessible for GUI
    NoiseGate noiseGate;
    Compressor compressor;
    Overdrive overdrive;
    Distortion distortion;
    HighGainDist highGainDist;
    Preamp preamp;
    ToneStack toneStack;
    PowerAmp powerAmp;
    CabinetSim cabinetSim;
    Chorus chorus;
    Flanger flanger;
    Phaser phaser;
    Harmonizer harmonizer;
    StringSynth stringSynth;
    DelayEffect delay;
    ReverbEffect reverb;
    ParametricEQ parametricEQ;
    GraphicEQ graphicEQ;
    TalkBox talkBox;
    AutoWah autoWah;
    Tuner tuner;

    std::atomic<float> currentTunerFreq { 0.0f };

private:
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GuitarMultiFXProcessor)
};
