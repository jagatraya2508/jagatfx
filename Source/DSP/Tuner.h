#pragma once
#include <JuceHeader.h>
#include <vector>
#include <cmath>

class Tuner
{
public:
    Tuner() = default;

    void prepare(double newSampleRate)
    {
        sampleRate = newSampleRate;
        bufferSize = 4096; // Good for down to ~20Hz at 44.1kHz
        circularBuffer.resize(bufferSize, 0.0f);
        bufferIndex = 0;
        currentFrequency = 0.0f;
    }

    void processBlock(const float* input, int numSamples)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            circularBuffer[bufferIndex] = input[i];
            bufferIndex = (bufferIndex + 1) % bufferSize;
        }
        
        calculateFrequency();
    }

    float getFrequency() const
    {
        return currentFrequency;
    }

private:
    void calculateFrequency()
    {
        int minLag = static_cast<int>(sampleRate / 1000.0f); // Max ~1000 Hz
        int maxLag = static_cast<int>(sampleRate / 60.0f);   // Min ~60 Hz
        if (maxLag > bufferSize / 2) maxLag = bufferSize / 2;

        // Unroll circular buffer for analysis
        std::vector<float> linearBuffer(bufferSize);
        for(int i = 0; i < bufferSize; ++i) {
            linearBuffer[i] = circularBuffer[(bufferIndex + i) % bufferSize];
        }

        // Check level
        float rms = 0.0f;
        for(int i = 0; i < bufferSize; ++i) rms += linearBuffer[i] * linearBuffer[i];
        rms = std::sqrt(rms / bufferSize);
        
        if (rms < 0.005f) { // Silence threshold
            currentFrequency = 0.0f; // Silence, reset needle
            return;
        }

        // Autocorrelation
        float maxCorr = 0;
        int bestLag = 0;
        std::vector<float> corr(maxLag, 0.0f);

        for (int lag = minLag; lag < maxLag; ++lag)
        {
            float sum = 0;
            for (int i = 0; i < maxLag; ++i)
            {
                sum += linearBuffer[i] * linearBuffer[i + lag];
            }
            corr[lag] = sum;
            if (sum > maxCorr)
            {
                maxCorr = sum;
                bestLag = lag;
            }
        }

        // Quadratic interpolation for better precision
        if (bestLag > minLag && bestLag < maxLag - 1)
        {
            float alpha = corr[bestLag - 1];
            float beta = corr[bestLag];
            float gamma = corr[bestLag + 1];
            
            float p = 0.5f * (alpha - gamma) / (alpha - 2.0f * beta + gamma);
            float refinedLag = bestLag + p;
            
            float freq = static_cast<float>(sampleRate) / refinedLag;
            
            // Smooth frequency if not wildly different, otherwise jump
            if (currentFrequency == 0.0f || std::abs(freq - currentFrequency) > 50.0f)
                currentFrequency = freq;
            else
                currentFrequency = currentFrequency * 0.7f + freq * 0.3f;
        }
    }

    double sampleRate = 44100.0;
    std::vector<float> circularBuffer;
    int bufferSize = 4096;
    int bufferIndex = 0;
    float currentFrequency = 0.0f;
};
