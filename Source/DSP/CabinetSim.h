#pragma once
#include <JuceHeader.h>

class CabinetSim
{
public:
    CabinetSim() = default;

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        maxBlockSize = spec.maximumBlockSize;
        numChannels = spec.numChannels;

        // Initialize built-in cabinet simulation filters
        for (auto& f : cabFilters)
            f.reset();

        updateCabinetFilters();
    }

    void setModel(int m) { if (model != m) { model = m; updateCabinetFilters(); } }
    void setMicPosition(int p) { if (micPos != p) { micPos = p; updateCabinetFilters(); } }

    void loadIR(const juce::File& irFile)
    {
        if (irFile.existsAsFile())
        {
            customIRFile = irFile;
            hasCustomIR = true;
        }
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        if (model == 4 && hasCustomIR)
        {
            // Custom IR mode - use convolution
            processConvolution(buffer);
        }
        else
        {
            // Built-in cabinet emulation using filters
            processFilters(buffer);
        }
    }

private:
    void processFilters(juce::AudioBuffer<float>& buffer)
    {
        int numSamps = buffer.getNumSamples();
        int numCh = buffer.getNumChannels();

        auto block = juce::dsp::AudioBlock<float>(buffer);

        for (int ch = 0; ch < numCh; ++ch)
        {
            auto singleBlock = block.getSingleChannelBlock(ch);
            auto ctx = juce::dsp::ProcessContextReplacing<float>(singleBlock);

            cabFilters[ch * 3 + 0].process(ctx); // Low cut
            cabFilters[ch * 3 + 1].process(ctx); // High cut (speaker rolloff)
            cabFilters[ch * 3 + 2].process(ctx); // Resonance peak
        }

        // Mic position coloring
        applyMicPosition(buffer);
    }

    void processConvolution(juce::AudioBuffer<float>& buffer)
    {
        // For custom IR, we apply a simple filter chain as placeholder
        // Full convolution requires loading the IR file
        processFilters(buffer);
    }

    void applyMicPosition(juce::AudioBuffer<float>& buffer)
    {
        int numSamps = buffer.getNumSamples();
        int numCh = buffer.getNumChannels();

        for (int ch = 0; ch < numCh; ++ch)
        {
            auto* data = buffer.getWritePointer(ch);
            for (int s = 0; s < numSamps; ++s)
            {
                float sample = data[s];
                switch (micPos)
                {
                    case 0: // On-Axis - bright, direct
                        break;
                    case 1: // Off-Axis - smoother, less bright
                        sample *= 0.9f;
                        break;
                    case 2: // Edge - darker, more bass
                        sample *= 0.85f;
                        break;
                    case 3: // Room - ambient, wider
                        sample *= 0.75f;
                        break;
                }
                data[s] = sample;
            }
        }
    }

    void updateCabinetFilters()
    {
        float lowCut, highCut, resFreq, resQ;

        switch (model)
        {
            case 0: // 1x12 Open Back
                lowCut = 100.0f; highCut = 5000.0f; resFreq = 2500.0f; resQ = 1.5f;
                break;
            case 1: // 2x12 Closed
                lowCut = 80.0f; highCut = 5500.0f; resFreq = 2200.0f; resQ = 1.2f;
                break;
            case 2: // 4x12 V30
                lowCut = 70.0f; highCut = 6000.0f; resFreq = 2800.0f; resQ = 1.8f;
                break;
            case 3: // 4x12 Greenback
                lowCut = 75.0f; highCut = 4500.0f; resFreq = 2000.0f; resQ = 1.4f;
                break;
            default: // Custom IR / fallback
                lowCut = 80.0f; highCut = 5500.0f; resFreq = 2500.0f; resQ = 1.5f;
                break;
        }

        // Mic position adjustments
        switch (micPos)
        {
            case 1: highCut *= 0.85f; break;  // Off-axis: darker
            case 2: highCut *= 0.75f; lowCut *= 0.8f; break; // Edge
            case 3: highCut *= 0.7f; resQ *= 0.6f; break; // Room
            default: break;
        }

        for (int ch = 0; ch < 2; ++ch)
        {
            cabFilters[ch * 3 + 0].coefficients =
                juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, lowCut, 0.707f);
            cabFilters[ch * 3 + 1].coefficients =
                juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, highCut, 0.707f);
            cabFilters[ch * 3 + 2].coefficients =
                juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, resFreq, resQ, 1.5f);
        }
    }

    double sampleRate = 44100.0;
    juce::uint32 maxBlockSize = 512;
    juce::uint32 numChannels = 2;
    int model = 0;
    int micPos = 0;
    bool hasCustomIR = false;
    juce::File customIRFile;

    // 6 filters: 2 channels x 3 filters (lowcut, highcut, resonance)
    juce::dsp::IIR::Filter<float> cabFilters[6];
};
