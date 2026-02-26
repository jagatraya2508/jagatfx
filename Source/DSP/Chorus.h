#pragma once
#include <JuceHeader.h>

class Chorus
{
public:
    Chorus() = default;

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        int maxDelay = static_cast<int>(sampleRate * 0.05); // 50ms max
        delayBufL.resize(maxDelay, 0.0f);
        delayBufR.resize(maxDelay, 0.0f);
        writePos = 0;
        lfoPhase = 0.0f;
    }

    void setRate(float r) { rate = r; }
    void setDepth(float d) { depth = d; }
    void setMix(float m) { mix = m; }

    void process(juce::AudioBuffer<float>& buffer)
    {
        int numSamples = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();
        int maxDelay = (int)delayBufL.size();

        float baseDelay = 7.0f; // 7ms base delay
        float maxModDelay = depth * 5.0f; // up to 5ms modulation

        for (int s = 0; s < numSamples; ++s)
        {
            // LFO
            float lfoL = std::sin(lfoPhase);
            float lfoR = std::sin(lfoPhase + juce::MathConstants<float>::pi * 0.5f); // 90° offset for stereo

            lfoPhase += 2.0f * juce::MathConstants<float>::pi * rate / (float)sampleRate;
            if (lfoPhase > 2.0f * juce::MathConstants<float>::pi)
                lfoPhase -= 2.0f * juce::MathConstants<float>::pi;

            // Calculate delay in samples
            float delayMsL = baseDelay + lfoL * maxModDelay;
            float delayMsR = baseDelay + lfoR * maxModDelay;
            float delaySamplesL = (delayMsL / 1000.0f) * (float)sampleRate;
            float delaySamplesR = (delayMsR / 1000.0f) * (float)sampleRate;

            delaySamplesL = juce::jlimit(1.0f, (float)(maxDelay - 2), delaySamplesL);
            delaySamplesR = juce::jlimit(1.0f, (float)(maxDelay - 2), delaySamplesR);

            // Read with interpolation
            float wetL = readInterp(delayBufL, writePos, delaySamplesL, maxDelay);
            float wetR = (numChannels > 1) ? readInterp(delayBufR, writePos, delaySamplesR, maxDelay) : wetL;

            // Write input to buffer
            float dryL = buffer.getSample(0, s);
            float dryR = (numChannels > 1) ? buffer.getSample(1, s) : dryL;

            delayBufL[writePos] = dryL;
            delayBufR[writePos] = dryR;

            // Mix
            buffer.setSample(0, s, dryL * (1.0f - mix) + wetL * mix);
            if (numChannels > 1)
                buffer.setSample(1, s, dryR * (1.0f - mix) + wetR * mix);

            writePos = (writePos + 1) % maxDelay;
        }
    }

private:
    float readInterp(const std::vector<float>& buf, int wp, float delay, int maxSize)
    {
        int readIdx = (int)delay;
        float frac = delay - (float)readIdx;
        int p1 = (wp - readIdx + maxSize) % maxSize;
        int p2 = (p1 - 1 + maxSize) % maxSize;
        return buf[p1] * (1.0f - frac) + buf[p2] * frac;
    }

    double sampleRate = 44100.0;
    float rate = 1.0f, depth = 0.5f, mix = 0.5f;
    float lfoPhase = 0.0f;
    std::vector<float> delayBufL, delayBufR;
    int writePos = 0;
};
