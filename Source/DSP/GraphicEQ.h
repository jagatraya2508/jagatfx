#pragma once
#include <JuceHeader.h>

class GraphicEQ
{
public:
    GraphicEQ() = default;

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        for (int i = 0; i < numBands; ++i)
            for (int ch = 0; ch < 2; ++ch)
                filters[i][ch].reset();

        updateAllBands();
    }

    void setBand(int band, float gainDb)
    {
        if (band < 0 || band >= numBands) return;
        bandGains[band] = gainDb;
        updateBand(band);
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        int numChannels = buffer.getNumChannels();
        auto block = juce::dsp::AudioBlock<float>(buffer);

        for (int band = 0; band < numBands; ++band)
        {
            if (std::abs(bandGains[band]) < 0.1f) continue; // Skip flat bands

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
        float gain = juce::Decibels::decibelsToGain(bandGains[band]);
        auto coeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            sampleRate, centerFreqs[band], 1.4f, gain);

        for (int ch = 0; ch < 2; ++ch)
            *filters[band][ch].coefficients = *coeffs;
    }

    void updateAllBands()
    {
        for (int i = 0; i < numBands; ++i)
            updateBand(i);
    }

    static constexpr int numBands = 10;
    double sampleRate = 44100.0;

    // ISO standard frequencies
    float centerFreqs[numBands] = {
        31.0f, 63.0f, 125.0f, 250.0f, 500.0f,
        1000.0f, 2000.0f, 4000.0f, 8000.0f, 16000.0f
    };

    float bandGains[numBands] = {}; // dB, initialized to 0
    juce::dsp::IIR::Filter<float> filters[numBands][2]; // 10 bands x 2 channels
};
