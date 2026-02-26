#pragma once
#include <JuceHeader.h>

class Distortion
{
public:
    Distortion() = default;

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        for (int ch = 0; ch < 2; ++ch)
        {
            toneFilter[ch].reset();
            toneFilter[ch].coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 4000.0f);
            hpFilter[ch].reset();
            hpFilter[ch].coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 60.0f);
            // Mid-boost filter for body/thickness
            midBoost[ch].reset();
            midBoost[ch].coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 800.0f, 1.0f, 2.5f);
            // Smooth out harsh fizz
            smoothFilter[ch].reset();
            smoothFilter[ch].coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 6000.0f);
        }
    }

    void setModel(int m) { model = m; }
    void setGain(float g) { gain = g; }
    void setTone(float t) { tone = t; }
    void setLevel(float l) { level = l; }

    void process(juce::AudioBuffer<float>& buffer)
    {
        float gainAmount = 1.0f + gain * 18.0f;
        float toneFreq = 600.0f + (tone / 10.0f) * 4000.0f;
        float outputLevel = level / 10.0f;

        auto toneCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, toneFreq);
        for (auto& f : toneFilter)
            *f.coefficients = *toneCoeffs;

        int numSamples = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();

        auto block = juce::dsp::AudioBlock<float>(buffer);

        // Pre-filter: mid boost for thickness before clipping
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto sb = block.getSingleChannelBlock(ch);
            auto ctx = juce::dsp::ProcessContextReplacing<float>(sb);
            midBoost[ch].process(ctx);
        }

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* data = buffer.getWritePointer(ch);
            for (int s = 0; s < numSamples; ++s)
            {
                float sample = data[s];
                sample = applyModel(sample, gainAmount);
                data[s] = sample * outputLevel;
            }
        }

        // Post-filter: tone + smooth + HP
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto sb = block.getSingleChannelBlock(ch);
            auto ctx = juce::dsp::ProcessContextReplacing<float>(sb);
            smoothFilter[ch].process(ctx);
            toneFilter[ch].process(ctx);
            hpFilter[ch].process(ctx);
        }
    }

private:
    float applyModel(float x, float gainAmount)
    {
        switch (model)
        {
            case 0: return ds1(x, gainAmount);
            case 1: return rat(x, gainAmount);
            case 2: return metalZone(x, gainAmount);
            default: return ds1(x, gainAmount);
        }
    }

    // DS-1: Warm clipping with body
    float ds1(float x, float g)
    {
        x *= g * 1.2f;
        // Multi-stage soft clipping for thick sustain
        float s1 = std::tanh(x * 1.8f);
        float s2 = std::tanh(s1 * 2.0f);
        // Asymmetric warmth
        if (s2 > 0.0f) return s2 * 0.85f;
        return s2 * 0.9f;
    }

    // RAT: Fat, heavy, warm clipping
    float rat(float x, float g)
    {
        x *= g * 2.0f;
        // Cascaded soft stages for thick compression
        float s1 = std::tanh(x * 2.0f);
        float s2 = (2.0f / juce::MathConstants<float>::pi) * std::atan(s1 * 2.5f);
        // Asymmetric for tube warmth
        if (s2 > 0.0f) return s2 * 0.8f;
        return s2 * 0.92f;
    }

    // Metal Zone: Heavy, thick cascaded gain
    float metalZone(float x, float g)
    {
        x *= g * 1.5f;
        // Three-stage cascade for maximum thickness
        float s1 = std::tanh(x * 2.0f);
        float s2 = std::tanh(s1 * 2.5f);
        float s3 = std::tanh(s2 * 1.5f);
        return s3 * 0.85f;
    }

    double sampleRate = 44100.0;
    int model = 0;
    float gain = 5.0f, tone = 5.0f, level = 5.0f;
    juce::dsp::IIR::Filter<float> toneFilter[2];
    juce::dsp::IIR::Filter<float> hpFilter[2];
    juce::dsp::IIR::Filter<float> midBoost[2];     // Body/thickness
    juce::dsp::IIR::Filter<float> smoothFilter[2];  // Remove harsh fizz
};
