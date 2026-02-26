#pragma once
#include <JuceHeader.h>

class ReverbEffect
{
public:
    ReverbEffect() = default;

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        juce::dsp::ProcessSpec monoSpec = spec;
        monoSpec.numChannels = 1;

        reverb.reset();
        reverb.setSampleRate(sampleRate);

        // Pre-delay buffers
        int maxPreDelay = static_cast<int>(sampleRate * 0.3); // max 300ms
        preDelayBufL.resize(maxPreDelay, 0.0f);
        preDelayBufR.resize(maxPreDelay, 0.0f);
        preDelayWritePos = 0;
    }

    void setModel(int m) { model = m; }
    void setSize(float s) { roomSize = s; }
    void setDamping(float d) { damping = d; }
    void setPreDelay(float ms) { preDelayMs = ms; }
    void setMix(float m) { mix = m; }

    void process(juce::AudioBuffer<float>& buffer)
    {
        // Configure reverb based on model
        juce::Reverb::Parameters params;

        switch (model)
        {
            case 0: // Hall
                params.roomSize = 0.3f + roomSize * 0.7f;
                params.damping = damping * 0.6f;
                params.width = 1.0f;
                params.wetLevel = mix;
                params.dryLevel = 1.0f - mix;
                params.freezeMode = 0.0f;
                break;

            case 1: // Room
                params.roomSize = 0.1f + roomSize * 0.4f;
                params.damping = 0.3f + damping * 0.5f;
                params.width = 0.7f;
                params.wetLevel = mix;
                params.dryLevel = 1.0f - mix;
                params.freezeMode = 0.0f;
                break;

            case 2: // Plate
                params.roomSize = 0.4f + roomSize * 0.5f;
                params.damping = damping * 0.3f;
                params.width = 1.0f;
                params.wetLevel = mix;
                params.dryLevel = 1.0f - mix;
                params.freezeMode = 0.0f;
                break;

            case 3: // Spring
                params.roomSize = 0.2f + roomSize * 0.3f;
                params.damping = 0.4f + damping * 0.4f;
                params.width = 0.5f;
                params.wetLevel = mix * 1.2f;
                params.dryLevel = 1.0f - mix;
                params.freezeMode = 0.0f;
                break;

            case 4: // Cathedral
                params.roomSize = 0.6f + roomSize * 0.4f;
                params.damping = damping * 0.2f;
                params.width = 1.0f;
                params.wetLevel = mix;
                params.dryLevel = 1.0f - mix;
                params.freezeMode = 0.0f;
                break;

            default:
                params.roomSize = 0.5f;
                params.damping = 0.5f;
                params.width = 1.0f;
                params.wetLevel = mix;
                params.dryLevel = 1.0f - mix;
                params.freezeMode = 0.0f;
                break;
        }

        reverb.setParameters(params);

        // Apply pre-delay
        if (preDelayMs > 0.0f)
        {
            int preDelaySamples = static_cast<int>((preDelayMs / 1000.0f) * sampleRate);
            preDelaySamples = juce::jmin(preDelaySamples, (int)preDelayBufL.size() - 1);
            applyPreDelay(buffer, preDelaySamples);
        }

        // Process reverb
        if (buffer.getNumChannels() >= 2)
            reverb.processStereo(buffer.getWritePointer(0), buffer.getWritePointer(1), buffer.getNumSamples());
        else
            reverb.processMono(buffer.getWritePointer(0), buffer.getNumSamples());
    }

private:
    void applyPreDelay(juce::AudioBuffer<float>& buffer, int delaySamples)
    {
        int numSamples = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();
        int maxDelay = (int)preDelayBufL.size();

        for (int s = 0; s < numSamples; ++s)
        {
            int readPos = (preDelayWritePos - delaySamples + maxDelay) % maxDelay;

            float inputL = buffer.getSample(0, s);
            buffer.setSample(0, s, preDelayBufL[readPos]);
            preDelayBufL[preDelayWritePos] = inputL;

            if (numChannels > 1)
            {
                float inputR = buffer.getSample(1, s);
                buffer.setSample(1, s, preDelayBufR[readPos]);
                preDelayBufR[preDelayWritePos] = inputR;
            }

            preDelayWritePos = (preDelayWritePos + 1) % maxDelay;
        }
    }

    double sampleRate = 44100.0;
    int model = 0;
    float roomSize = 0.5f;
    float damping = 0.5f;
    float preDelayMs = 20.0f;
    float mix = 0.3f;

    juce::Reverb reverb;
    std::vector<float> preDelayBufL, preDelayBufR;
    int preDelayWritePos = 0;
};
