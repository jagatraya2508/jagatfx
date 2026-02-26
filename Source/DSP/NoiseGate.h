#pragma once
#include <JuceHeader.h>

class NoiseGate
{
public:
    NoiseGate() = default;

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        envelope = 0.0f;
        gainReduction = 1.0f;
        gateOpen = true;
        holdCounter = 0;
    }

    void setThreshold(float threshDb) { thresholdDb = threshDb; }
    void setAttack(float attackMs) { attackTime = attackMs; }
    void setRelease(float releaseMs) { releaseTime = releaseMs; }
    void setHoldTime(float holdMs) { holdTime = holdMs; }

    void process(juce::AudioBuffer<float>& buffer)
    {
        float threshold = juce::Decibels::decibelsToGain(thresholdDb);
        float closeThreshold = juce::Decibels::decibelsToGain(thresholdDb - 6.0f);
        int holdSamples = static_cast<int>(sampleRate * holdTime * 0.001f);

        float envAttackCoeff = std::exp(-1.0f / (sampleRate * 0.0002f));
        float envReleaseCoeff = std::exp(-1.0f / (sampleRate * 0.015f));
        float gainOpenCoeff = std::exp(-1.0f / (sampleRate * attackTime * 0.001f));
        float gainCloseCoeff = std::exp(-1.0f / (sampleRate * releaseTime * 0.001f));

        int numSamples = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();

        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Peak detect
            float inputLevel = 0.0f;
            for (int ch = 0; ch < numChannels; ++ch)
                inputLevel = std::max(inputLevel, std::abs(buffer.getSample(ch, sample)));

            // Envelope follower
            if (inputLevel > envelope)
                envelope = envAttackCoeff * envelope + (1.0f - envAttackCoeff) * inputLevel;
            else
                envelope = envReleaseCoeff * envelope + (1.0f - envReleaseCoeff) * inputLevel;

            // Gate decision with hysteresis + hold time
            if (envelope > threshold)
            {
                gateOpen = true;
                holdCounter = holdSamples; // Reset hold timer
            }
            else if (envelope < closeThreshold)
            {
                if (holdCounter > 0)
                    holdCounter--; // Count down hold time
                else
                    gateOpen = false; // Hold expired, close gate
            }

            float targetGain = gateOpen ? 1.0f : 0.0f;

            // Smooth gain transitions
            if (targetGain > gainReduction)
                gainReduction = gainOpenCoeff * gainReduction + (1.0f - gainOpenCoeff) * targetGain;
            else
                gainReduction = gainCloseCoeff * gainReduction + (1.0f - gainCloseCoeff) * targetGain;

            // Apply
            for (int ch = 0; ch < numChannels; ++ch)
                buffer.setSample(ch, sample, buffer.getSample(ch, sample) * gainReduction);
        }
    }

private:
    float sampleRate = 44100.0f;
    float thresholdDb = -40.0f;
    float attackTime = 1.0f;     // ms
    float releaseTime = 50.0f;   // ms
    float holdTime = 150.0f;     // ms - keeps gate open after signal drops
    float envelope = 0.0f;
    float gainReduction = 1.0f;
    bool gateOpen = true;
    int holdCounter = 0;
};
