#include "PluginProcessor.h"
#include "PluginEditor.h"

GuitarMultiFXProcessor::GuitarMultiFXProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

GuitarMultiFXProcessor::~GuitarMultiFXProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout GuitarMultiFXProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // ===== MASTER =====
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("inputGain", 1), "Input Gain", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("outputGain", 1), "Output Gain", juce::NormalisableRange<float>(-60.0f, 12.0f, 0.1f), 0.0f));

    // ===== TUNER =====
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("tunerEnabled", 1), "Tuner On", false));

    // ===== NOISE GATE =====
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("gateEnabled", 1), "Gate On", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("gateThreshold", 1), "Gate Threshold", juce::NormalisableRange<float>(-80.0f, 0.0f, 0.1f), -40.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("gateAttack", 1), "Gate Attack", juce::NormalisableRange<float>(0.1f, 50.0f, 0.1f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("gateRelease", 1), "Gate Release", juce::NormalisableRange<float>(1.0f, 500.0f, 1.0f), 50.0f));

    // ===== COMPRESSOR =====
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("compEnabled", 1), "Comp On", false));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("compModel", 1), "Comp Model", juce::StringArray{"VCA", "Optical", "FET"}, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compThreshold", 1), "Comp Threshold", juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f), -20.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compRatio", 1), "Comp Ratio", juce::NormalisableRange<float>(1.0f, 20.0f, 0.1f), 4.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compAttack", 1), "Comp Attack", juce::NormalisableRange<float>(0.1f, 100.0f, 0.1f), 10.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compRelease", 1), "Comp Release", juce::NormalisableRange<float>(10.0f, 1000.0f, 1.0f), 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compMakeup", 1), "Comp Makeup", juce::NormalisableRange<float>(0.0f, 24.0f, 0.1f), 0.0f));

    // ===== OVERDRIVE =====
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("odEnabled", 1), "OD On", false));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("odModel", 1), "OD Model", juce::StringArray{"Tube Screamer", "Blues Driver", "Klon"}, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("odDrive", 1), "OD Drive", juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f), 5.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("odTone", 1), "OD Tone", juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f), 5.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("odLevel", 1), "OD Level", juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f), 5.0f));

    // ===== DISTORTION =====
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("distEnabled", 1), "Dist On", false));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("distModel", 1), "Dist Model", juce::StringArray{"DS-1", "RAT", "Metal Zone"}, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("distGain", 1), "Dist Gain", juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f), 5.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("distTone", 1), "Dist Tone", juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f), 5.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("distLevel", 1), "Dist Level", juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f), 5.0f));

    // ===== HIGH GAIN DISTORTION =====
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("hgEnabled", 1), "HG On", false));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("hgModel", 1), "HG Model", juce::StringArray{"Rectifier", "5150", "Dual Rec", "Djent"}, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("hgGain", 1), "HG Gain", juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f), 7.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("hgTone", 1), "HG Tone", juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f), 5.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("hgLevel", 1), "HG Level", juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f), 5.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("hgTight", 1), "HG Tight", true));

    // ===== PREAMP =====
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("ampEnabled", 1), "Amp On", true));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("ampModel", 1), "Amp Model",
        juce::StringArray{"Clean", "Crunch", "High Gain", "Metal", "Fender Twin", "Marshall JCM", "Mesa Rectifier"}, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("ampGain", 1), "Amp Gain", juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f), 5.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("ampChannel", 1), "Amp Channel Vol", juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f), 5.0f));

    // ===== TONE STACK =====
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("tsBass", 1), "Bass", juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f), 5.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("tsMid", 1), "Mid", juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f), 5.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("tsTreble", 1), "Treble", juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f), 5.0f));

    // ===== POWER AMP =====
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("paPresence", 1), "Presence", juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f), 5.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("paResonance", 1), "Resonance", juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f), 5.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("paMaster", 1), "Master", juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f), 5.0f));

    // ===== CABINET =====
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("cabEnabled", 1), "Cab On", true));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("cabModel", 1), "Cab Model",
        juce::StringArray{"1x12 Open Back", "2x12 Closed", "4x12 V30", "4x12 Greenback", "Custom IR"}, 0));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("cabMic", 1), "Mic Position",
        juce::StringArray{"On-Axis", "Off-Axis", "Edge", "Room"}, 0));

    // ===== DELAY =====
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("delayEnabled", 1), "Delay On", false));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("delayModel", 1), "Delay Model",
        juce::StringArray{"Digital", "Analog", "Tape", "Ping-Pong"}, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("delayTime", 1), "Delay Time", juce::NormalisableRange<float>(1.0f, 2000.0f, 1.0f), 400.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("delayFeedback", 1), "Delay Feedback", juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f), 0.4f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("delayMix", 1), "Delay Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("delayMod", 1), "Delay Modulation", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));

    // ===== REVERB =====
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("reverbEnabled", 1), "Reverb On", false));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("reverbModel", 1), "Reverb Model",
        juce::StringArray{"Hall", "Room", "Plate", "Spring", "Cathedral"}, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("reverbSize", 1), "Reverb Size", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("reverbDamping", 1), "Reverb Damping", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("reverbPreDelay", 1), "Reverb PreDelay", juce::NormalisableRange<float>(0.0f, 200.0f, 1.0f), 20.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("reverbMix", 1), "Reverb Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.3f));

    // ===== CHORUS =====
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("chorusEnabled", 1), "Chorus On", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("chorusRate", 1), "Chorus Rate", juce::NormalisableRange<float>(0.1f, 10.0f, 0.01f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("chorusDepth", 1), "Chorus Depth", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("chorusMix", 1), "Chorus Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));

    // ===== FLANGER =====
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("flangerEnabled", 1), "Flanger On", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("flangerRate", 1), "Flanger Rate", juce::NormalisableRange<float>(0.05f, 5.0f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("flangerDepth", 1), "Flanger Depth", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("flangerFeedback", 1), "Flanger Feedback", juce::NormalisableRange<float>(-0.95f, 0.95f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("flangerMix", 1), "Flanger Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));

    // ===== PHASER =====
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("phaserEnabled", 1), "Phaser On", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("phaserRate", 1), "Phaser Rate", juce::NormalisableRange<float>(0.05f, 5.0f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("phaserDepth", 1), "Phaser Depth", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("phaserFeedback", 1), "Phaser Feedback", juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("phaserStages", 1), "Phaser Stages", juce::StringArray{"4", "8", "12"}, 1));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("phaserMix", 1), "Phaser Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));

    // ===== HARMONIZER =====
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("harmEnabled", 1), "Harmonizer On", false));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("harmInterval", 1), "Harm Interval",
        juce::StringArray{"Minor 3rd", "Major 3rd", "4th", "5th", "Octave Up", "Octave Down"}, 1));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("harmMix", 1), "Harm Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));

    // ===== STRING SYNTH =====
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("stringEnabled", 1), "String Synth On", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("stringAttack", 1), "String Attack", juce::NormalisableRange<float>(10.0f, 1000.0f, 1.0f), 200.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("stringOctave", 1), "String Octave", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("stringBrightness", 1), "String Brightness", juce::NormalisableRange<float>(500.0f, 10000.0f, 1.0f, 0.3f), 2000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("stringResonance", 1), "String Resonance", juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("stringMix", 1), "String Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));

    // ===== PARAMETRIC EQ =====
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("peqEnabled", 1), "PEQ On", false));
    for (int i = 0; i < 4; ++i)
    {
        auto idx = juce::String(i);
        float defaultFreqs[] = { 100.0f, 500.0f, 2000.0f, 8000.0f };
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("peqFreq" + idx, 1), "PEQ Freq " + idx,
            juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f), defaultFreqs[i]));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("peqGain" + idx, 1), "PEQ Gain " + idx,
            juce::NormalisableRange<float>(-18.0f, 18.0f, 0.1f), 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("peqQ" + idx, 1), "PEQ Q " + idx,
            juce::NormalisableRange<float>(0.1f, 10.0f, 0.01f), 1.0f));
    }

    // ===== GRAPHIC EQ =====
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("geqEnabled", 1), "GEQ On", false));
    float geqFreqs[] = { 31.0f, 63.0f, 125.0f, 250.0f, 500.0f, 1000.0f, 2000.0f, 4000.0f, 8000.0f, 16000.0f };
    for (int i = 0; i < 10; ++i)
    {
        auto idx = juce::String(i);
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("geqBand" + idx, 1), "GEQ " + juce::String((int)geqFreqs[i]) + "Hz",
            juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f));
    }

    // ===== TALK BOX =====
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("talkEnabled", 1), "Talk On", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("talkVowel", 1), "Vowel", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("talkMix", 1), "Talk Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 1.0f));

    // ===== AUTO WAH =====
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("autoWahEnabled", 1), "Wah On", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("autoWahSens", 1), "Wah Sens", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("autoWahAttack", 1), "Wah Attack", juce::NormalisableRange<float>(1.0f, 100.0f, 1.0f), 10.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("autoWahRelease", 1), "Wah Release", juce::NormalisableRange<float>(10.0f, 500.0f, 1.0f), 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("autoWahRange", 1), "Wah Range", juce::NormalisableRange<float>(100.0f, 2000.0f, 1.0f, 0.3f), 400.0f));

    return { params.begin(), params.end() };
}

const juce::String GuitarMultiFXProcessor::getName() const { return JucePlugin_Name; }
bool GuitarMultiFXProcessor::acceptsMidi() const { return false; }
bool GuitarMultiFXProcessor::producesMidi() const { return false; }
bool GuitarMultiFXProcessor::isMidiEffect() const { return false; }
double GuitarMultiFXProcessor::getTailLengthSeconds() const { return 2.0; }
int GuitarMultiFXProcessor::getNumPrograms() { return 1; }
int GuitarMultiFXProcessor::getCurrentProgram() { return 0; }
void GuitarMultiFXProcessor::setCurrentProgram(int) {}
const juce::String GuitarMultiFXProcessor::getProgramName(int) { return {}; }
void GuitarMultiFXProcessor::changeProgramName(int, const juce::String&) {}

void GuitarMultiFXProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

    noiseGate.prepare(spec);
    compressor.prepare(spec);
    overdrive.prepare(spec);
    distortion.prepare(spec);
    highGainDist.prepare(spec);
    preamp.prepare(spec);
    toneStack.prepare(spec);
    powerAmp.prepare(spec);
    cabinetSim.prepare(spec);
    chorus.prepare(spec);
    flanger.prepare(spec);
    phaser.prepare(spec);
    harmonizer.prepare(spec);
    stringSynth.prepare(spec);
    delay.prepare(spec);
    reverb.prepare(spec);
    parametricEQ.prepare(spec);
    graphicEQ.prepare(spec);
    talkBox.prepare(spec);
    autoWah.prepare(spec);
    tuner.prepare(sampleRate);
}

void GuitarMultiFXProcessor::releaseResources() {}

bool GuitarMultiFXProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void GuitarMultiFXProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Jika input Mono dan output Stereo, copy sinyal channel 0 ke channel 1 agar efek stereo berfungsi
    if (totalNumInputChannels == 1 && totalNumOutputChannels == 2)
    {
        buffer.copyFrom(1, 0, buffer, 0, 0, buffer.getNumSamples());
    }
    else
    {
        // Bersihkan channel output sisanya agar tidak bising
        for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
            buffer.clear(i, 0, buffer.getNumSamples());
    }

    // Process tuner independently of input gain and effects
    if (totalNumInputChannels > 0)
    {
        tuner.processBlock(buffer.getReadPointer(0), buffer.getNumSamples());
        currentTunerFreq.store(tuner.getFrequency(), std::memory_order_relaxed);
    }

    // Check if tuner is enabled to mute everything else
    if (*apvts.getRawParameterValue("tunerEnabled") > 0.5f)
    {
        for (auto i = 0; i < totalNumOutputChannels; ++i)
            buffer.clear(i, 0, buffer.getNumSamples());
        return; // Don't process other effects, mute sound
    }

    // === Input Gain ===
    float inputGainDb = *apvts.getRawParameterValue("inputGain");
    float inputGain = juce::Decibels::decibelsToGain(inputGainDb);
    buffer.applyGain(inputGain);

    // === 1. NOISE GATE (with hold time for sustain) ===
    if (*apvts.getRawParameterValue("gateEnabled") > 0.5f)
    {
        noiseGate.setThreshold(*apvts.getRawParameterValue("gateThreshold"));
        noiseGate.setAttack(*apvts.getRawParameterValue("gateAttack"));
        noiseGate.setRelease(*apvts.getRawParameterValue("gateRelease"));
        noiseGate.process(buffer);
    }

    // === 2. PRE-EFFECTS: Compressor ===
    if (*apvts.getRawParameterValue("compEnabled") > 0.5f)
    {
        compressor.setModel(static_cast<int>(*apvts.getRawParameterValue("compModel")));
        compressor.setThreshold(*apvts.getRawParameterValue("compThreshold"));
        compressor.setRatio(*apvts.getRawParameterValue("compRatio"));
        compressor.setAttack(*apvts.getRawParameterValue("compAttack"));
        compressor.setRelease(*apvts.getRawParameterValue("compRelease"));
        compressor.setMakeup(*apvts.getRawParameterValue("compMakeup"));
        compressor.process(buffer);
    }

    // === 2. PRE-EFFECTS: Overdrive ===
    if (*apvts.getRawParameterValue("odEnabled") > 0.5f)
    {
        overdrive.setModel(static_cast<int>(*apvts.getRawParameterValue("odModel")));
        overdrive.setDrive(*apvts.getRawParameterValue("odDrive"));
        overdrive.setTone(*apvts.getRawParameterValue("odTone"));
        overdrive.setLevel(*apvts.getRawParameterValue("odLevel"));
        overdrive.process(buffer);
    }

    // === 2. PRE-EFFECTS: Distortion ===
    if (*apvts.getRawParameterValue("distEnabled") > 0.5f)
    {
        distortion.setModel(static_cast<int>(*apvts.getRawParameterValue("distModel")));
        distortion.setGain(*apvts.getRawParameterValue("distGain"));
        distortion.setTone(*apvts.getRawParameterValue("distTone"));
        distortion.setLevel(*apvts.getRawParameterValue("distLevel"));
        distortion.process(buffer);
    }

    // === 2. PRE-EFFECTS: High Gain Distortion ===
    if (*apvts.getRawParameterValue("hgEnabled") > 0.5f)
    {
        highGainDist.setModel(static_cast<int>(*apvts.getRawParameterValue("hgModel")));
        highGainDist.setGain(*apvts.getRawParameterValue("hgGain"));
        highGainDist.setTone(*apvts.getRawParameterValue("hgTone"));
        highGainDist.setLevel(*apvts.getRawParameterValue("hgLevel"));
        highGainDist.setTight(*apvts.getRawParameterValue("hgTight") > 0.5f);
        highGainDist.process(buffer);
    }

    // === 3. PREAMP (Waveshaper) ===
    if (*apvts.getRawParameterValue("ampEnabled") > 0.5f)
    {
        preamp.setModel(static_cast<int>(*apvts.getRawParameterValue("ampModel")));
        preamp.setGain(*apvts.getRawParameterValue("ampGain"));
        preamp.setChannelVolume(*apvts.getRawParameterValue("ampChannel"));
        preamp.process(buffer);
    }

    // === 4. TONE STACK ===
    toneStack.setBass(*apvts.getRawParameterValue("tsBass"));
    toneStack.setMid(*apvts.getRawParameterValue("tsMid"));
    toneStack.setTreble(*apvts.getRawParameterValue("tsTreble"));
    toneStack.process(buffer);

    // === 5. POWER AMP ===
    powerAmp.setPresence(*apvts.getRawParameterValue("paPresence"));
    powerAmp.setResonance(*apvts.getRawParameterValue("paResonance"));
    powerAmp.setMaster(*apvts.getRawParameterValue("paMaster"));
    powerAmp.process(buffer);

    // === 6. CABINET (IR Convolution) ===
    if (*apvts.getRawParameterValue("cabEnabled") > 0.5f)
    {
        cabinetSim.setModel(static_cast<int>(*apvts.getRawParameterValue("cabModel")));
        cabinetSim.setMicPosition(static_cast<int>(*apvts.getRawParameterValue("cabMic")));
        cabinetSim.process(buffer);
    }

    // === 7. POST-EFFECTS: Parametric EQ ===
    if (*apvts.getRawParameterValue("peqEnabled") > 0.5f)
    {
        for (int i = 0; i < 4; ++i)
        {
            auto idx = juce::String(i);
            parametricEQ.setBand(i,
                *apvts.getRawParameterValue("peqFreq" + idx),
                *apvts.getRawParameterValue("peqGain" + idx),
                *apvts.getRawParameterValue("peqQ" + idx));
        }
        parametricEQ.process(buffer);
    }

    // === 7. POST-EFFECTS: Graphic EQ ===
    if (*apvts.getRawParameterValue("geqEnabled") > 0.5f)
    {
        for (int i = 0; i < 10; ++i)
        {
            graphicEQ.setBand(i, *apvts.getRawParameterValue("geqBand" + juce::String(i)));
        }
        graphicEQ.process(buffer);
    }

    // === 7. POST-EFFECTS: Talk Box ===
    if (*apvts.getRawParameterValue("talkEnabled") > 0.5f)
    {
        talkBox.setVowel(*apvts.getRawParameterValue("talkVowel"));
        talkBox.setMix(*apvts.getRawParameterValue("talkMix"));
        talkBox.process(buffer);
    }

    // === 7. POST-EFFECTS: Auto Wah ===
    if (*apvts.getRawParameterValue("autoWahEnabled") > 0.5f)
    {
        autoWah.setParameters(
            *apvts.getRawParameterValue("autoWahSens"),
            *apvts.getRawParameterValue("autoWahAttack"),
            *apvts.getRawParameterValue("autoWahRelease"),
            *apvts.getRawParameterValue("autoWahRange"));
        autoWah.process(buffer);
    }

    // === 7. POST-EFFECTS: Chorus ===
    if (*apvts.getRawParameterValue("chorusEnabled") > 0.5f)
    {
        chorus.setRate(*apvts.getRawParameterValue("chorusRate"));
        chorus.setDepth(*apvts.getRawParameterValue("chorusDepth"));
        chorus.setMix(*apvts.getRawParameterValue("chorusMix"));
        chorus.process(buffer);
    }

    // === 7. POST-EFFECTS: Flanger ===
    if (*apvts.getRawParameterValue("flangerEnabled") > 0.5f)
    {
        flanger.setRate(*apvts.getRawParameterValue("flangerRate"));
        flanger.setDepth(*apvts.getRawParameterValue("flangerDepth"));
        flanger.setFeedback(*apvts.getRawParameterValue("flangerFeedback"));
        flanger.setMix(*apvts.getRawParameterValue("flangerMix"));
        flanger.process(buffer);
    }

    // === 7. POST-EFFECTS: Phaser ===
    if (*apvts.getRawParameterValue("phaserEnabled") > 0.5f)
    {
        phaser.setRate(*apvts.getRawParameterValue("phaserRate"));
        phaser.setDepth(*apvts.getRawParameterValue("phaserDepth"));
        phaser.setFeedback(*apvts.getRawParameterValue("phaserFeedback"));
        phaser.setStages(static_cast<int>(*apvts.getRawParameterValue("phaserStages")));
        phaser.setMix(*apvts.getRawParameterValue("phaserMix"));
        phaser.process(buffer);
    }

    // === 7. POST-EFFECTS: Harmonizer ===
    if (*apvts.getRawParameterValue("harmEnabled") > 0.5f)
    {
        harmonizer.setInterval(static_cast<int>(*apvts.getRawParameterValue("harmInterval")));
        harmonizer.setMix(*apvts.getRawParameterValue("harmMix"));
        harmonizer.process(buffer);
    }

    // === 7. POST-EFFECTS: String Synth ===
    if (*apvts.getRawParameterValue("stringEnabled") > 0.5f)
    {
        stringSynth.setAttack(*apvts.getRawParameterValue("stringAttack"));
        stringSynth.setOctaveMix(*apvts.getRawParameterValue("stringOctave"));
        stringSynth.setBrightness(*apvts.getRawParameterValue("stringBrightness"));
        stringSynth.setResonance(*apvts.getRawParameterValue("stringResonance"));
        stringSynth.setMix(*apvts.getRawParameterValue("stringMix"));
        stringSynth.process(buffer);
    }

    // === 7. POST-EFFECTS: Delay ===
    if (*apvts.getRawParameterValue("delayEnabled") > 0.5f)
    {
        delay.setModel(static_cast<int>(*apvts.getRawParameterValue("delayModel")));
        delay.setTime(*apvts.getRawParameterValue("delayTime"));
        delay.setFeedback(*apvts.getRawParameterValue("delayFeedback"));
        delay.setMix(*apvts.getRawParameterValue("delayMix"));
        delay.setModulation(*apvts.getRawParameterValue("delayMod"));
        delay.process(buffer);
    }

    // === 7. POST-EFFECTS: Reverb ===
    if (*apvts.getRawParameterValue("reverbEnabled") > 0.5f)
    {
        reverb.setModel(static_cast<int>(*apvts.getRawParameterValue("reverbModel")));
        reverb.setSize(*apvts.getRawParameterValue("reverbSize"));
        reverb.setDamping(*apvts.getRawParameterValue("reverbDamping"));
        reverb.setPreDelay(*apvts.getRawParameterValue("reverbPreDelay"));
        reverb.setMix(*apvts.getRawParameterValue("reverbMix"));
        reverb.process(buffer);
    }

    // === Output Gain ===
    float outputGainDb = *apvts.getRawParameterValue("outputGain");
    float outputGain = juce::Decibels::decibelsToGain(outputGainDb);
    buffer.applyGain(outputGain);
}

juce::AudioProcessorEditor* GuitarMultiFXProcessor::createEditor()
{
    return new GuitarMultiFXEditor(*this);
}

bool GuitarMultiFXProcessor::hasEditor() const { return true; }

void GuitarMultiFXProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void GuitarMultiFXProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GuitarMultiFXProcessor();
}
