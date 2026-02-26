#pragma once
#include <JuceHeader.h>

/**
 * AutoWah - Memberikan efek wah otomatis yang "kental" (vocal character).
 * Menggunakan cascaded resonant filters dan internal saturation.
 */
class AutoWah
{
public:
    AutoWah() = default;

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        for (int i = 0; i < 2; ++i)
        {
            filterStage1[i].prepare(spec);
            filterStage2[i].prepare(spec);
            filterStage1[i].setType(juce::dsp::StateVariableTPTFilterType::bandpass);
            filterStage2[i].setType(juce::dsp::StateVariableTPTFilterType::bandpass);
        }
        
        envelope = 0.0f;
    }

    void setParameters(float sensitivity, float attackMs, float releaseMs, float baseFreq)
    {
        this->sensitivity = sensitivity;
        attackCoeff = std::exp(-1.0f / (juce::jmax(1.0f, attackMs) * 0.001f * sampleRate));
        releaseCoeff = std::exp(-1.0f / (juce::jmax(1.0f, releaseMs) * 0.001f * sampleRate));
        this->baseFreq = baseFreq;
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        auto numChannels = buffer.getNumChannels();
        auto numSamples = buffer.getNumSamples();

        for (int i = 0; i < numSamples; ++i)
        {
            // Detect envelope (smooth follower)
            float envIn = 0.0f;
            for (int ch = 0; ch < numChannels; ++ch)
                envIn = std::max(envIn, std::abs(buffer.getSample(ch, i)));

            if (envIn > envelope)
                envelope = envIn + attackCoeff * (envelope - envIn);
            else
                envelope = envIn + releaseCoeff * (envelope - envIn);

            // Calculate target frequency with nonlinear response
            float sweep = std::pow(envelope * sensitivity, 0.7f); // Curves for more "vocal" response
            float cutoff = baseFreq + (sweep * 6000.0f);
            cutoff = juce::jlimit(100.0f, 10000.0f, cutoff);

            // Dynamic resonance (higher resonance at higher frequency)
            float res = 2.0f + (envelope * sensitivity * 4.0f); 

            // Update cascaded filters
            for (int ch = 0; ch < juce::jmin(2, (int)numChannels); ++ch)
            {
                filterStage1[ch].setCutoffFrequency(cutoff);
                filterStage2[ch].setCutoffFrequency(cutoff);
                filterStage1[ch].setResonance(res);
                filterStage2[ch].setResonance(res);
            }

            // Process samples with saturation
            for (int ch = 0; ch < numChannels; ++ch)
            {
                float in = buffer.getSample(ch, i);
                
                // Cascade for steeper slope (24dB style)
                float s1 = filterStage1[ch % 2].processSample(ch % 2, in);
                float s2 = filterStage2[ch % 2].processSample(ch % 2, s1);
                
                // Add internal saturation to make it "kental"
                float saturated = std::tanh(s2 * 2.5f);
                
                // Mix: mostly wet for wah character
                buffer.setSample(ch, i, saturated * 0.9f + in * 0.1f);
            }
        }
    }

private:
    double sampleRate = 44100.0;
    float sensitivity = 0.5f;
    float baseFreq = 400.0f;
    float attackCoeff = 0.99f;
    float releaseCoeff = 0.999f;
    float envelope = 0.0f;

    // Use cascaded SVF filters for steeper (kental) response
    juce::dsp::StateVariableTPTFilter<float> filterStage1[2];
    juce::dsp::StateVariableTPTFilter<float> filterStage2[2];
};
