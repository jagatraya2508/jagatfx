#pragma once
#include <JuceHeader.h>

class Overdrive
{
public:
    Overdrive() = default;

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        for (int ch = 0; ch < 2; ++ch)
        {
            toneFilter[ch].reset();
            toneFilter[ch].coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 5000.0f);
            hpFilter[ch].reset();
            hpFilter[ch].coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 80.0f);
        }
    }

    void setModel(int m) { model = m; }
    void setDrive(float d) { drive = d; }
    void setTone(float t) { tone = t; }
    void setLevel(float l) { level = l; }

    void process(juce::AudioBuffer<float>& buffer)
    {
        // Increased drive scaler for much more sustain
        float driveAmount = 1.0f + drive * 10.0f; 
        
        // Lower tone filter range for a rounder, warmer tone
        float toneFreq = 500.0f + (tone / 10.0f) * 4500.0f; 
        float outputLevel = level / 10.0f;

        auto toneCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, toneFreq);
        for (auto& f : toneFilter)
            *f.coefficients = *toneCoeffs;

        int numSamples = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* data = buffer.getWritePointer(ch);
            for (int s = 0; s < numSamples; ++s)
            {
                float sample = data[s];
                sample = applyModel(sample, driveAmount);
                data[s] = sample * outputLevel;
            }
        }

        // Apply tone filter per channel
        auto block = juce::dsp::AudioBlock<float>(buffer);
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto sb = block.getSingleChannelBlock(ch);
            auto ctx = juce::dsp::ProcessContextReplacing<float>(sb);
            toneFilter[ch].process(ctx);
            hpFilter[ch].process(ctx);
        }
    }

private:
    float applyModel(float x, float driveAmount)
    {
        switch (model)
        {
            case 0: return tubeScreamer(x, driveAmount);
            case 1: return bluesDriver(x, driveAmount);
            case 2: return klonCentaur(x, driveAmount);
            default: return tubeScreamer(x, driveAmount);
        }
    }

    // Tube Screamer: mid-hump, soft clipping via op-amp + diode
    float tubeScreamer(float x, float d)
    {
        x *= d;
        // Smoother asymmetric clipping curve (tube/op-amp character)
        if (x > 0.0f)
            return std::tanh(x * x * 0.1f + x) * 0.8f;
        else
            return std::tanh(x * 0.9f) * 0.85f;
    }

    // Blues Driver: open, dynamic overdrive
    float bluesDriver(float x, float d)
    {
        x *= d * 1.5f; // More sustain
        // Smoother asymptotic curve with warmth
        return (2.0f / juce::MathConstants<float>::pi) * std::atan(x * 2.5f + x * x * 0.1f) * 0.9f;
    }

    // Klon Centaur: transparent overdrive
    float klonCentaur(float x, float d)
    {
        float clean = x * 0.4f;
        float driven = std::tanh(x * d * 1.2f) * 0.8f;
        return clean + driven; // Clean blend
    }

    double sampleRate = 44100.0;
    int model = 0;
    float drive = 5.0f, tone = 5.0f, level = 5.0f;
    juce::dsp::IIR::Filter<float> toneFilter[2]; // Stereo
    juce::dsp::IIR::Filter<float> hpFilter[2];   // Stereo
};
