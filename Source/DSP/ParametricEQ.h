#pragma once
#include <JuceHeader.h>

class ParametricEQ
{
public:
    ParametricEQ() = default;

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        for (int i = 0; i < 4; ++i)
            for (int ch = 0; ch < 2; ++ch)
                filters[i][ch].reset();
    }

    void setBand(int band, float freq, float gainDb, float q)
    {
        if (band < 0 || band >= 4) return;
        bands[band] = { freq, gainDb, q };
        updateBand(band);
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        int numChannels = buffer.getNumChannels();
        auto block = juce::dsp::AudioBlock<float>(buffer);

        for (int band = 0; band < 4; ++band)
        {
            for (int ch = 0; ch < numChannels; ++ch)
            {
                auto sb = block.getSingleChannelBlock(ch);
                auto ctx = juce::dsp::ProcessContextReplacing<float>(sb);
                filters[band][ch].process(ctx);
            }
        }
    }

private:
    void updateBand(int band)
    {
        auto& b = bands[band];
        juce::dsp::IIR::Coefficients<float>::Ptr coeffs;

        if (band == 0) // Low shelf
            coeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
                sampleRate, b.freq, b.q, juce::Decibels::decibelsToGain(b.gainDb));
        else if (band == 3) // High shelf
            coeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
                sampleRate, b.freq, b.q, juce::Decibels::decibelsToGain(b.gainDb));
        else // Peak
            coeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
                sampleRate, b.freq, b.q, juce::Decibels::decibelsToGain(b.gainDb));

        for (int ch = 0; ch < 2; ++ch)
            *filters[band][ch].coefficients = *coeffs;
    }

    struct BandParams { float freq = 1000.0f, gainDb = 0.0f, q = 1.0f; };

    double sampleRate = 44100.0;
    BandParams bands[4] = {
        { 100.0f, 0.0f, 1.0f },
        { 500.0f, 0.0f, 1.0f },
        { 2000.0f, 0.0f, 1.0f },
        { 8000.0f, 0.0f, 1.0f }
    };
    juce::dsp::IIR::Filter<float> filters[4][2]; // 4 bands x 2 channels
};
