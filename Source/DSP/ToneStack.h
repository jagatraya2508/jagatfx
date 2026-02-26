#pragma once
#include <JuceHeader.h>

class ToneStack
{
public:
    ToneStack() = default;

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        updateFilters();
    }

    void setBass(float b) { bass = b; filtersNeedUpdate = true; }
    void setMid(float m) { mid = m; filtersNeedUpdate = true; }
    void setTreble(float t) { treble = t; filtersNeedUpdate = true; }

    void process(juce::AudioBuffer<float>& buffer)
    {
        if (filtersNeedUpdate)
        {
            updateFilters();
            filtersNeedUpdate = false;
        }

        int numChannels = buffer.getNumChannels();
        int numSamples = buffer.getNumSamples();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* data = buffer.getWritePointer(ch);
            for (int s = 0; s < numSamples; ++s)
            {
                float sample = data[s];

                // Process through each filter stage
                float lowOut = processFilter(lowState[ch], lowCoeffs, sample);
                float midOut = processFilter(midState[ch], midCoeffs, sample);
                float highOut = processFilter(highState[ch], highCoeffs, sample);

                // Mix filtered bands with knob values
                float bassGain = (bass / 10.0f) * 2.0f - 1.0f;  // -1 to +1
                float midGain = (mid / 10.0f) * 2.0f - 1.0f;
                float trebleGain = (treble / 10.0f) * 2.0f - 1.0f;

                // Reconstruct signal
                data[s] = sample
                    + lowOut * bassGain * 0.8f
                    + midOut * midGain * 0.6f
                    + highOut * trebleGain * 0.7f;
            }
        }
    }

private:
    struct BiquadCoeffs { float b0, b1, b2, a1, a2; };
    struct BiquadState { float x1 = 0, x2 = 0, y1 = 0, y2 = 0; };

    float processFilter(BiquadState& state, const BiquadCoeffs& c, float x)
    {
        float y = c.b0 * x + c.b1 * state.x1 + c.b2 * state.x2
                  - c.a1 * state.y1 - c.a2 * state.y2;
        state.x2 = state.x1;
        state.x1 = x;
        state.y2 = state.y1;
        state.y1 = y;
        return y;
    }

    void computePeakCoeffs(BiquadCoeffs& c, float freq, float gain, float q)
    {
        float w0 = 2.0f * juce::MathConstants<float>::pi * freq / (float)sampleRate;
        float A = std::pow(10.0f, gain / 40.0f);
        float alpha = std::sin(w0) / (2.0f * q);

        float b0 = 1.0f + alpha * A;
        float b1 = -2.0f * std::cos(w0);
        float b2 = 1.0f - alpha * A;
        float a0 = 1.0f + alpha / A;
        float a1 = -2.0f * std::cos(w0);
        float a2 = 1.0f - alpha / A;

        c.b0 = b0 / a0; c.b1 = b1 / a0; c.b2 = b2 / a0;
        c.a1 = a1 / a0; c.a2 = a2 / a0;
    }

    void updateFilters()
    {
        // Bass: centered at 120Hz
        computePeakCoeffs(lowCoeffs, 120.0f, 12.0f, 0.7f);
        // Mid: centered at 800Hz
        computePeakCoeffs(midCoeffs, 800.0f, 12.0f, 0.8f);
        // Treble: centered at 3500Hz
        computePeakCoeffs(highCoeffs, 3500.0f, 12.0f, 0.7f);
    }

    double sampleRate = 44100.0;
    float bass = 5.0f, mid = 5.0f, treble = 5.0f;
    bool filtersNeedUpdate = true;

    BiquadCoeffs lowCoeffs{}, midCoeffs{}, highCoeffs{};
    BiquadState lowState[2], midState[2], highState[2]; // Stereo
};
