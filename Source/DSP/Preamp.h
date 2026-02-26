#pragma once
#include <JuceHeader.h>

class Preamp
{
public:
    Preamp() = default;

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;

        // Anti-aliasing lowpass before waveshaping (per channel)
        for (auto& f : antiAlias)
        {
            f.reset();
            f.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 10000.0f);
        }

        // High pass to remove DC and hum (per channel)
        for (auto& db : dcBlocker)
        {
            db.reset();
            db.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 30.0f);
        }
    }

    void setModel(int m) { model = m; }
    void setGain(float g) { gain = g; }
    void setChannelVolume(float v) { channelVol = v; }

    void process(juce::AudioBuffer<float>& buffer)
    {
        int numSamples = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();

        // Gain staging per model
        float preGain = getPreGainForModel();
        float postGain = channelVol / 10.0f;

        auto block = juce::dsp::AudioBlock<float>(buffer);

        // Anti-aliasing filter before waveshaping (per channel)
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto singleBlock = block.getSingleChannelBlock(ch);
            auto ctx = juce::dsp::ProcessContextReplacing<float>(singleBlock);
            antiAlias[ch].process(ctx);
        }

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* data = buffer.getWritePointer(ch);

            for (int s = 0; s < numSamples; ++s)
            {
                float sample = data[s] * preGain;

                // Apply waveshaper based on model
                sample = applyWaveshaper(sample);

                // Post gain (channel volume)
                sample *= postGain;

                data[s] = sample;
            }
        }

        // DC blocker after waveshaping (per channel)
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto singleBlock = block.getSingleChannelBlock(ch);
            auto ctx = juce::dsp::ProcessContextReplacing<float>(singleBlock);
            dcBlocker[ch].process(ctx);
        }
    }

private:
    float applyWaveshaper(float x)
    {
        switch (model)
        {
            case 0: // Clean
                return cleanWaveshaper(x);
            case 1: // Crunch
                return crunchWaveshaper(x);
            case 2: // High Gain
                return highGainWaveshaper(x);
            case 3: // Metal
                return metalWaveshaper(x);
            case 4: // Fender Twin
                return fenderWaveshaper(x);
            case 5: // Marshall JCM
                return marshallWaveshaper(x);
            case 6: // Mesa Rectifier
                return mesaWaveshaper(x);
            case 7: // Soldano Lead
                return soldanoWaveshaper(x);
            default:
                return cleanWaveshaper(x);
        }
    }

    // Clean: soft saturation (12AX7 preamp tube sim)
    float cleanWaveshaper(float x)
    {
        // Asymmetric soft clip (tube-like)
        if (x > 0.0f)
            return std::tanh(x * 0.8f);
        else
            return std::tanh(x * 0.9f) * 0.95f;
    }

    // Crunch: moderate clipping
    float crunchWaveshaper(float x)
    {
        float asymmetry = 0.1f;
        x += asymmetry;
        float out = (2.0f / juce::MathConstants<float>::pi) * std::atan(x * 2.5f);
        return out - asymmetry * 0.5f;
    }

    // High Gain: aggressive tube saturation
    float highGainWaveshaper(float x)
    {
        // Multi-stage clipping
        float stage1 = std::tanh(x * 3.0f);
        float stage2 = std::tanh(stage1 * 2.0f);
        return stage2 * 0.9f;
    }

    // Metal: extreme saturation with tight response
    float metalWaveshaper(float x)
    {
        // Hard clip with tube warmth
        float stage1 = std::tanh(x * 5.0f);
        float stage2 = std::tanh(stage1 * 3.0f);
        float hardClip = juce::jlimit(-0.95f, 0.95f, stage2);
        return hardClip;
    }

    // Fender Twin: warm, bright clean
    float fenderWaveshaper(float x)
    {
        // Soft asymmetric clipping (12AX7 + 6L6 character)
        if (x >= 0.0f)
            return (1.0f - std::exp(-x * 1.5f)) * 0.85f;
        else
            return -(1.0f - std::exp(x * 1.2f)) * 0.9f;
    }

    // Marshall JCM: EL34 crunch
    float marshallWaveshaper(float x)
    {
        float k = 2.0f * gain / 10.0f + 1.0f;
        float out = (1.0f + k) * x / (1.0f + k * std::abs(x));
        return std::tanh(out * 1.5f);
    }

    // Mesa Rectifier: heavy saturation
    float mesaWaveshaper(float x)
    {
        // Cascaded gain stages with asymmetric clipping
        float s1 = std::tanh(x * 4.0f);
        float s2 = (2.0f / juce::MathConstants<float>::pi) * std::atan(s1 * 3.0f);
        float s3 = std::tanh(s2 * 2.5f);
        return s3 * 0.85f;
    }

    // Soldano Lead: smooth, articulate high gain with massive sustain
    float soldanoWaveshaper(float x)
    {
        // Smooth multi-stage cascaded clipping
        float s1 = std::tanh(x * 4.5f);
        // Slightly asymmetric for tube warmth, very smooth transition
        float s2 = (s1 > 0) ? std::tanh(s1 * 2.5f) : std::tanh(s1 * 2.2f);
        float s3 = std::tanh(s2 * 3.0f);
        return s3 * 0.9f;
    }

    float getPreGainForModel()
    {
        float normalized = gain / 10.0f;
        switch (model)
        {
            case 0: return 0.5f + normalized * 1.5f;        // Clean: low gain
            case 1: return 1.0f + normalized * 4.0f;        // Crunch
            case 2: return 2.0f + normalized * 10.0f;       // High Gain
            case 3: return 3.0f + normalized * 15.0f;       // Metal
            case 4: return 0.4f + normalized * 1.2f;        // Fender
            case 5: return 1.5f + normalized * 6.0f;        // Marshall
            case 6: return 2.5f + normalized * 12.0f;       // Mesa
            case 7: return 3.0f + normalized * 14.0f;       // Soldano (massive gain)
            default: return 1.0f + normalized * 3.0f;
        }
    }

    double sampleRate = 44100.0;
    int model = 0;
    float gain = 5.0f;
    float channelVol = 5.0f;

    juce::dsp::IIR::Filter<float> antiAlias[2];  // Stereo anti-aliasing
    juce::dsp::IIR::Filter<float> dcBlocker[2];  // Stereo DC blockers
};
