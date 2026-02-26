#pragma once
#include <JuceHeader.h>

class Flanger
{
public:
    Flanger() = default;

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        int maxDelay = static_cast<int>(sampleRate * 0.02); // 20ms max
        delayBufL.resize(maxDelay, 0.0f);
        delayBufR.resize(maxDelay, 0.0f);
        writePos = 0;
        lfoPhase = 0.0f;
    }

    void setRate(float r) { rate = r; }
    void setDepth(float d) { depth = d; }
    void setFeedback(float fb) { feedback = fb; }
    void setMix(float m) { mix = m; }

    void process(juce::AudioBuffer<float>& buffer)
    {
        int numSamples = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();
        int maxDelay = (int)delayBufL.size();

        float baseDelay = 1.0f; // 1ms base
        float maxModDelay = depth * 8.0f; // up to 8ms sweep

        for (int s = 0; s < numSamples; ++s)
        {
            float lfo = (std::sin(lfoPhase) + 1.0f) * 0.5f; // 0 to 1
            lfoPhase += 2.0f * juce::MathConstants<float>::pi * rate / (float)sampleRate;
            if (lfoPhase > 2.0f * juce::MathConstants<float>::pi)
                lfoPhase -= 2.0f * juce::MathConstants<float>::pi;

            float delayMs = baseDelay + lfo * maxModDelay;
            float delaySamples = (delayMs / 1000.0f) * (float)sampleRate;
            delaySamples = juce::jlimit(1.0f, (float)(maxDelay - 2), delaySamples);

            // Read with interpolation
            float wetL = readInterp(delayBufL, writePos, delaySamples, maxDelay);
            float wetR = (numChannels > 1) ? readInterp(delayBufR, writePos, delaySamples, maxDelay) : wetL;

            float dryL = buffer.getSample(0, s);
            float dryR = (numChannels > 1) ? buffer.getSample(1, s) : dryL;

            // Write with feedback
            delayBufL[writePos] = dryL + wetL * feedback;
            delayBufR[writePos] = dryR + wetR * feedback;

            // Clamp
            delayBufL[writePos] = juce::jlimit(-2.0f, 2.0f, delayBufL[writePos]);
            delayBufR[writePos] = juce::jlimit(-2.0f, 2.0f, delayBufR[writePos]);

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
    float rate = 0.5f, depth = 0.5f, feedback = 0.5f, mix = 0.5f;
    float lfoPhase = 0.0f;
    std::vector<float> delayBufL, delayBufR;
    int writePos = 0;
};
