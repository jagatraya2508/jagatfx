#pragma once
#include <JuceHeader.h>

class Phaser
{
public:
    Phaser() = default;

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        lfoPhase = 0.0f;

        for (int i = 0; i < maxStages; ++i)
            for (int ch = 0; ch < 2; ++ch)
                allpassState[ch][i] = 0.0f;
    }

    void setRate(float r) { rate = r; }
    void setDepth(float d) { depth = d; }
    void setFeedback(float fb) { feedback = fb; }
    void setStages(int s) { stages = s; } // 0=4, 1=8, 2=12
    void setMix(float m) { mix = m; }

    void process(juce::AudioBuffer<float>& buffer)
    {
        int numSamples = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();

        int numStages = 4;
        switch (stages)
        {
            case 0: numStages = 4; break;
            case 1: numStages = 8; break;
            case 2: numStages = 12; break;
        }

        for (int s = 0; s < numSamples; ++s)
        {
            // LFO
            float lfo = (std::sin(lfoPhase) + 1.0f) * 0.5f; // 0 to 1
            lfoPhase += 2.0f * juce::MathConstants<float>::pi * rate / (float)sampleRate;
            if (lfoPhase > 2.0f * juce::MathConstants<float>::pi)
                lfoPhase -= 2.0f * juce::MathConstants<float>::pi;

            // Modulated frequency range for allpass
            float minFreq = 200.0f;
            float maxFreq = 4000.0f;
            float freq = minFreq + lfo * depth * (maxFreq - minFreq);

            // Calculate allpass coefficient
            float w = 2.0f * juce::MathConstants<float>::pi * freq / (float)sampleRate;
            float coeff = (1.0f - std::tan(w * 0.5f)) / (1.0f + std::tan(w * 0.5f));

            for (int ch = 0; ch < numChannels; ++ch)
            {
                float input = buffer.getSample(ch, s);
                float processed = input + lastOutput[ch] * feedback;

                // Cascade of allpass filters
                for (int stage = 0; stage < numStages; ++stage)
                {
                    float temp = coeff * processed + allpassState[ch][stage];
                    allpassState[ch][stage] = processed - coeff * temp;
                    processed = temp;
                }

                lastOutput[ch] = processed;

                // Mix
                buffer.setSample(ch, s, input * (1.0f - mix) + processed * mix);
            }
        }
    }

private:
    static constexpr int maxStages = 12;

    double sampleRate = 44100.0;
    float rate = 0.5f, depth = 0.5f, feedback = 0.5f, mix = 0.5f;
    int stages = 1; // 0=4, 1=8, 2=12
    float lfoPhase = 0.0f;
    float allpassState[2][maxStages] = {};
    float lastOutput[2] = {};
};
