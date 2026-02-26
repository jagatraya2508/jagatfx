#pragma once
#include <JuceHeader.h>

class TalkBox
{
public:
    TalkBox() = default;

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        for (int i = 0; i < 3; ++i)
        {
            for (int ch = 0; ch < 2; ++ch)
                filters[i][ch].reset();
        }
        updateFilters();
    }

    void setVowel(float vowelPos) // 0.0 to 1.0 (A, E, I, O, U)
    {
        vowelPosition = vowelPos;
        updateFilters();
    }

    void setMix(float newMix)
    {
        mix = newMix;
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        if (mix <= 0.01f) return;

        juce::AudioBuffer<float> dryBuffer;
        dryBuffer.makeCopyOf(buffer);

        auto block = juce::dsp::AudioBlock<float>(buffer);
        int numChannels = buffer.getNumChannels();

        for (int i = 0; i < 3; ++i)
        {
            for (int ch = 0; ch < numChannels; ++ch)
            {
                auto sb = block.getSingleChannelBlock(ch);
                auto ctx = juce::dsp::ProcessContextReplacing<float>(sb);
                filters[i][ch].process(ctx);
            }
        }

        // Apply mix
        for (int ch = 0; ch < numChannels; ++ch)
        {
            buffer.applyGainRamp(ch, 0, buffer.getNumSamples(), mix, mix);
            buffer.addFromWithRamp(ch, 0, dryBuffer.getReadPointer(ch), buffer.getNumSamples(), 1.0f - mix, 1.0f - mix);
        }
    }

private:
    void updateFilters()
    {
        // Formant data for vowels: F1, F2, F3
        // Values from literature (approximate for male voice)
        const float formants[5][3] = {
            { 730.0f, 1090.0f, 2440.0f }, // A
            { 270.0f, 2290.0f, 3010.0f }, // E
            { 270.0f, 1990.0f, 2550.0f }, // I
            { 570.0f,  840.0f, 2410.0f }, // O
            { 300.0f,  870.0f, 2240.0f }  // U
        };

        // Interpolate between vowels based on vowelPosition
        float scaledPos = vowelPosition * 4.0f;
        int idx1 = juce::jlimit(0, 3, (int)std::floor(scaledPos));
        int idx2 = idx1 + 1;
        float frac = scaledPos - (float)idx1;

        float f[3];
        for (int i = 0; i < 3; ++i)
        {
            f[i] = formants[idx1][i] + frac * (formants[idx2][i] - formants[idx1][i]);
            
            // Fixed Q for resonances
            float q = 8.0f; // Narrow filters for speech-like character
            float gain = 10.0f; // Boost the formants

            auto coeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
                sampleRate, f[i], q, juce::Decibels::decibelsToGain(gain));

            for (int ch = 0; ch < 2; ++ch)
                *filters[i][ch].coefficients = *coeffs;
        }
    }

    double sampleRate = 44100.0;
    float vowelPosition = 0.0f;
    float mix = 1.0f;
    juce::dsp::IIR::Filter<float> filters[3][2]; // 3 resonant peaks x 2 channels
};
