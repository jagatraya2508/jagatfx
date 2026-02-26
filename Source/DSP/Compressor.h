#pragma once
#include <JuceHeader.h>

class Compressor
{
public:
    Compressor() = default;

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        envelope = 0.0f;
    }

    void setModel(int m) { model = m; }
    void setThreshold(float t) { threshold = t; }
    void setRatio(float r) { ratio = r; }
    void setAttack(float a) { attack = a; }
    void setRelease(float r) { release = r; }
    void setMakeup(float m) { makeup = m; }

    void process(juce::AudioBuffer<float>& buffer)
    {
        float attackCoeff = std::exp(-1.0f / (sampleRate * attack * 0.001f));
        float releaseCoeff = std::exp(-1.0f / (sampleRate * release * 0.001f));
        float makeupGainDb = makeup;

        int numSamples = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();

        for (int s = 0; s < numSamples; ++s)
        {
            // Detect peak across channels in linear scale
            float inputLevel = 0.0f;
            for (int ch = 0; ch < numChannels; ++ch)
                inputLevel = std::max(inputLevel, std::abs(buffer.getSample(ch, s)));

            // Envelope follower with model-specific behavior (Linear domain)
            float targetLin = inputLevel;

            switch (model)
            {
                case 0: // VCA - fast, precise
                    if (targetLin > envelope)
                        envelope = attackCoeff * envelope + (1.0f - attackCoeff) * targetLin;
                    else
                        envelope = releaseCoeff * envelope + (1.0f - releaseCoeff) * targetLin;
                    break;

                case 1: // Optical - slow, smooth
                {
                    float optAttack = std::exp(-1.0f / (sampleRate * attack * 0.003f));
                    float optRelease = std::exp(-1.0f / (sampleRate * release * 0.005f));
                    if (targetLin > envelope)
                        envelope = optAttack * envelope + (1.0f - optAttack) * targetLin;
                    else
                        envelope = optRelease * envelope + (1.0f - optRelease) * targetLin;
                    break;
                }

                case 2: // FET - aggressive, punchy
                {
                    float fetAttack = std::exp(-1.0f / (sampleRate * attack * 0.0005f));
                    float fetRelease = std::exp(-1.0f / (sampleRate * release * 0.002f));
                    if (targetLin > envelope)
                        envelope = fetAttack * envelope + (1.0f - fetAttack) * targetLin;
                    else
                        envelope = fetRelease * envelope + (1.0f - fetRelease) * targetLin;
                    break;
                }
            }

            // Convert enveloped linear value to dB
            float envelopeDb = juce::Decibels::gainToDecibels(envelope + 1e-6f);

            // Calculate gain reduction
            float gainReductionDb = 0.0f;
            
            // Soft-knee width
            float kneeWidth = 4.0f; 
            
            if (envelopeDb > threshold + kneeWidth / 2.0f)
            {
                // Hard knee region
                float excess = envelopeDb - threshold;
                gainReductionDb = excess * (1.0f - 1.0f / ratio);
            }
            else if (envelopeDb > threshold - kneeWidth / 2.0f)
            {
                // Soft knee region
                float excess = envelopeDb - threshold + kneeWidth / 2.0f;
                gainReductionDb = (1.0f - 1.0f / ratio) * (excess * excess) / (2.0f * kneeWidth);
            }

            // Convert to linear gain multiplier
            float gainDb = makeupGainDb - gainReductionDb;
            float gain = juce::Decibels::decibelsToGain(gainDb);

            // Apply gain
            for (int ch = 0; ch < numChannels; ++ch)
                buffer.setSample(ch, s, buffer.getSample(ch, s) * gain);
        }
    }

private:
    double sampleRate = 44100.0;
    int model = 0;
    float threshold = -20.0f;
    float ratio = 4.0f;
    float attack = 10.0f;   // ms
    float release = 100.0f; // ms
    float makeup = 0.0f;    // dB
    float envelope = 0.0f;
};
