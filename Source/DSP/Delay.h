#pragma once
#include <JuceHeader.h>

class DelayEffect
{
public:
    DelayEffect() = default;

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        maxDelaySamples = static_cast<int>(sampleRate * 2.5); // Max 2.5 sec

        delayBufferL.resize(maxDelaySamples, 0.0f);
        delayBufferR.resize(maxDelaySamples, 0.0f);
        writePos = 0;

        modPhase = 0.0f;

        // Analog warmth filter
        lpFilter.reset();
        lpFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 3500.0f);
    }

    void setModel(int m) { model = m; }
    void setTime(float ms) { delayTimeMs = ms; }
    void setFeedback(float fb) { feedback = fb; }
    void setMix(float m) { mix = m; }
    void setModulation(float mod) { modAmount = mod; }

    void process(juce::AudioBuffer<float>& buffer)
    {
        int numSamples = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();

        float delaySamples = (delayTimeMs / 1000.0f) * (float)sampleRate;

        for (int s = 0; s < numSamples; ++s)
        {
            // Modulation LFO
            float modOffset = 0.0f;
            if (modAmount > 0.0f)
            {
                modOffset = std::sin(modPhase) * modAmount * 10.0f;
                modPhase += 2.0f * juce::MathConstants<float>::pi * 0.5f / (float)sampleRate;
                if (modPhase > 2.0f * juce::MathConstants<float>::pi)
                    modPhase -= 2.0f * juce::MathConstants<float>::pi;
            }

            float readDelay = delaySamples + modOffset;
            if (readDelay < 1.0f) readDelay = 1.0f;
            if (readDelay >= maxDelaySamples - 1) readDelay = (float)(maxDelaySamples - 2);

            // Read from delay buffer with linear interpolation
            int readIdx = (int)readDelay;
            float frac = readDelay - (float)readIdx;
            int pos1 = (writePos - readIdx + maxDelaySamples) % maxDelaySamples;
            int pos2 = (pos1 - 1 + maxDelaySamples) % maxDelaySamples;

            float delayedL = delayBufferL[pos1] * (1.0f - frac) + delayBufferL[pos2] * frac;
            float delayedR = (numChannels > 1) ?
                delayBufferR[pos1] * (1.0f - frac) + delayBufferR[pos2] * frac : delayedL;

            // Apply model-specific processing
            switch (model)
            {
                case 1: // Analog: warm, dark repeats
                    delayedL = analogProcess(delayedL);
                    delayedR = analogProcess(delayedR);
                    break;
                case 2: // Tape: warble, saturation
                    delayedL = tapeProcess(delayedL);
                    delayedR = tapeProcess(delayedR);
                    break;
                case 3: // Ping-Pong
                {
                    float tempL = delayedL;
                    delayedL = delayedR;
                    delayedR = tempL;
                    break;
                }
                default: // Digital: clean
                    break;
            }

            // Get input
            float inputL = buffer.getSample(0, s);
            float inputR = (numChannels > 1) ? buffer.getSample(1, s) : inputL;

            // Write to delay buffer (input + feedback)
            delayBufferL[writePos] = inputL + delayedL * feedback;
            delayBufferR[writePos] = inputR + delayedR * feedback;

            // Clamp feedback to prevent runaway
            delayBufferL[writePos] = juce::jlimit(-2.0f, 2.0f, delayBufferL[writePos]);
            delayBufferR[writePos] = juce::jlimit(-2.0f, 2.0f, delayBufferR[writePos]);

            // Mix dry/wet
            buffer.setSample(0, s, inputL * (1.0f - mix) + delayedL * mix);
            if (numChannels > 1)
                buffer.setSample(1, s, inputR * (1.0f - mix) + delayedR * mix);

            writePos = (writePos + 1) % maxDelaySamples;
        }
    }

private:
    float analogProcess(float x)
    {
        // Warm, slightly saturated
        return std::tanh(x * 0.9f) * 0.95f;
    }

    float tapeProcess(float x)
    {
        // Tape saturation + slight wobble
        float saturated = std::tanh(x * 1.1f);
        return saturated * 0.9f;
    }

    double sampleRate = 44100.0;
    int maxDelaySamples = 0;
    int model = 0;
    float delayTimeMs = 400.0f;
    float feedback = 0.4f;
    float mix = 0.3f;
    float modAmount = 0.0f;
    float modPhase = 0.0f;

    std::vector<float> delayBufferL, delayBufferR;
    int writePos = 0;

    juce::dsp::IIR::Filter<float> lpFilter;
};
