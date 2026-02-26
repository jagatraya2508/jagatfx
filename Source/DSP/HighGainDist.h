#pragma once
#include <JuceHeader.h>

class HighGainDist
{
public:
    HighGainDist() = default;

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        for (int ch = 0; ch < 2; ++ch)
        {
            toneFilter[ch].reset();
            // Lower cutoff for warmer, less fizzy tone
            toneFilter[ch].coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 4000.0f);
            tightFilter[ch].reset();
            tightFilter[ch].coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 80.0f);
            presenceFilter[ch].reset();
            presenceFilter[ch].coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 2500.0f, 1.5f, 1.8f);
            // Mid-body boost for thick, chunky tone
            midBody[ch].reset();
            midBody[ch].coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 600.0f, 0.8f, 3.0f);
            // Anti-fizz filter
            smoothFilter[ch].reset();
            smoothFilter[ch].coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 5500.0f);
        }
    }

    void setModel(int m) { model = m; }
    void setGain(float g) { gain = g; }
    void setTone(float t) { tone = t; }
    void setLevel(float l) { level = l; }
    void setTight(bool t) { tight = t; }

    void process(juce::AudioBuffer<float>& buffer)
    {
        // High gain but not excessive — keep it musical
        float gainAmount = 2.0f + gain * 20.0f;

        float toneFreq = 800.0f + (tone / 10.0f) * 3500.0f;
        float outputLevel = level / 10.0f;

        auto toneCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, toneFreq);
        for (auto& f : toneFilter)
            *f.coefficients = *toneCoeffs;

        auto tightCoeffs = tight
            ? juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 100.0f, 1.0f)
            : juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 50.0f, 0.7f);
        for (auto& f : tightFilter)
            *f.coefficients = *tightCoeffs;

        int numSamples = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();

        auto block = juce::dsp::AudioBlock<float>(buffer);

        // Pre-filter: tight + mid body boost
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto sb = block.getSingleChannelBlock(ch);
            auto ctx = juce::dsp::ProcessContextReplacing<float>(sb);
            tightFilter[ch].process(ctx);
            midBody[ch].process(ctx);
        }

        // Waveshaping
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

        // Post-filter: smooth + tone + presence
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto sb = block.getSingleChannelBlock(ch);
            auto ctx = juce::dsp::ProcessContextReplacing<float>(sb);
            smoothFilter[ch].process(ctx);
            toneFilter[ch].process(ctx);
            presenceFilter[ch].process(ctx);
        }
    }

private:
    float applyModel(float x, float g)
    {
        switch (model)
        {
            case 0: return rectifier(x, g);
            case 1: return fiveOneFifty(x, g);
            case 2: return dualRec(x, g);
            case 3: return djent(x, g);
            default: return rectifier(x, g);
        }
    }

    // Rectifier: Mesa-style thick multi-stage
    float rectifier(float x, float g)
    {
        x *= g;
        // Three tube stages for thick, saturated tone
        float s1 = std::tanh(x * 1.5f);
        float s2 = std::tanh(s1 * g * 0.3f);
        float s3 = std::tanh(s2 * 1.5f);
        // Asymmetric for warmth
        return (s3 > 0.0f) ? s3 * 0.85f : s3 * 0.92f;
    }

    // 5150: Tight, punchy high gain
    float fiveOneFifty(float x, float g)
    {
        x *= g * 1.3f;
        // Cascaded stages with controlled gain
        float s1 = std::tanh(x * 2.5f);
        float s2 = std::tanh(s1 * 2.0f);
        // Asymmetric for punch
        if (s2 > 0.0f) return s2 * 0.82f;
        return s2 * 0.9f;
    }

    // Dual Rec: Scooped, massive
    float dualRec(float x, float g)
    {
        x *= g * 1.5f;
        float s1 = std::tanh(x * 2.0f);
        float s2 = (2.0f / juce::MathConstants<float>::pi) * std::atan(s1 * 3.5f);
        float s3 = std::tanh(s2 * 2.0f);
        return s3 * 0.85f;
    }

    // Djent: Very tight, percussive
    float djent(float x, float g)
    {
        x *= g * 2.0f;
        // Tight clipping with sustain
        float s1 = std::tanh(x * 3.0f);
        float s2 = std::tanh(s1 * 2.5f);
        return s2 * 0.88f;
    }

    double sampleRate = 44100.0;
    int model = 0;
    float gain = 7.0f, tone = 5.0f, level = 5.0f;
    bool tight = true;
    juce::dsp::IIR::Filter<float> toneFilter[2];
    juce::dsp::IIR::Filter<float> tightFilter[2];
    juce::dsp::IIR::Filter<float> presenceFilter[2];
    juce::dsp::IIR::Filter<float> midBody[2];       // Mid body for thickness
    juce::dsp::IIR::Filter<float> smoothFilter[2];   // Anti-fizz
};
