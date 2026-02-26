#pragma once
#include <JuceHeader.h>

class PowerAmp
{
public:
    PowerAmp() = default;

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;

        // Presence filter (high shelf) - one per channel for stereo
        for (auto& f : presenceFilter)
        {
            f.reset();
            f.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
                sampleRate, 3000.0f, 0.707f, 1.0f);
        }

        // Resonance filter (low shelf) - one per channel for stereo
        for (auto& f : resonanceFilter)
        {
            f.reset();
            f.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
                sampleRate, 100.0f, 0.707f, 1.0f);
        }
    }

    void setPresence(float p) { presence = p; }
    void setResonance(float r) { resonance = r; }
    void setMaster(float m) { master = m; }

    void process(juce::AudioBuffer<float>& buffer)
    {
        float masterGain = master / 10.0f;

        // Update presence filter coefficients
        float presenceGain = juce::Decibels::decibelsToGain((presence / 10.0f - 0.5f) * 12.0f);
        auto presenceCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            sampleRate, 3000.0f, 0.707f, presenceGain);

        // Update resonance filter coefficients
        float resonanceGain = juce::Decibels::decibelsToGain((resonance / 10.0f - 0.5f) * 12.0f);
        auto resonanceCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
            sampleRate, 100.0f, 0.707f, resonanceGain);

        for (auto& f : presenceFilter)
            *f.coefficients = *presenceCoeffs;
        for (auto& f : resonanceFilter)
            *f.coefficients = *resonanceCoeffs;

        int numSamples = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();

        // Drive amount for power tube saturation
        float drive = masterGain * 2.0f;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* data = buffer.getWritePointer(ch);

            for (int s = 0; s < numSamples; ++s)
            {
                float sample = data[s];

                // Power tube saturation (soft, warm compression)
                if (drive > 0.1f)
                {
                    sample *= drive;
                    // Asymmetric soft saturation (power tube character)
                    if (sample > 0.0f)
                        sample = std::tanh(sample * 0.7f);
                    else
                        sample = std::tanh(sample * 0.8f) * 0.95f;

                    // Output level control
                    sample *= masterGain;
                }
                else
                {
                    sample *= masterGain;
                }

                data[s] = sample;
            }
        }

        // Apply presence and resonance per channel
        auto block = juce::dsp::AudioBlock<float>(buffer);
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto singleBlock = block.getSingleChannelBlock(ch);
            auto ctx = juce::dsp::ProcessContextReplacing<float>(singleBlock);
            presenceFilter[ch].process(ctx);
            resonanceFilter[ch].process(ctx);
        }
    }

private:
    double sampleRate = 44100.0;
    float presence = 5.0f;
    float resonance = 5.0f;
    float master = 5.0f;

    juce::dsp::IIR::Filter<float> presenceFilter[2];  // Stereo
    juce::dsp::IIR::Filter<float> resonanceFilter[2]; // Stereo
};
