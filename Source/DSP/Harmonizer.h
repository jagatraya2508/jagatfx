#pragma once
#include <JuceHeader.h>

class Harmonizer
{
public:
    Harmonizer() = default;

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        // Circular buffer for pitch shifting (granular approach)
        int bufSize = static_cast<int>(sampleRate * 0.1); // 100ms window
        grainBuffer.resize(bufSize, 0.0f);
        writePos = 0;
        readPos = 0.0f;
        crossfadePos = 0.0f;
    }

    void setInterval(int i) { interval = i; }
    void setMix(float m) { mix = m; }

    void process(juce::AudioBuffer<float>& buffer)
    {
        float pitchRatio = getPitchRatio();
        int numSamples = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();
        int bufSize = (int)grainBuffer.size();

        for (int s = 0; s < numSamples; ++s)
        {
            float dry = buffer.getSample(0, s);

            // Write to grain buffer
            grainBuffer[writePos] = dry;

            // Read at pitch-shifted rate
            int readInt = (int)readPos;
            float readFrac = readPos - (float)readInt;
            int r1 = readInt % bufSize;
            int r2 = (readInt + 1) % bufSize;
            float shifted = grainBuffer[r1] * (1.0f - readFrac) + grainBuffer[r2] * readFrac;

            // Granular crossfade to avoid clicks
            float halfBuf = (float)bufSize * 0.5f;
            float dist = std::fmod(std::abs((float)writePos - readPos + bufSize), (float)bufSize);
            float window = 1.0f;
            if (dist < 64.0f)
                window = dist / 64.0f;

            shifted *= window;

            // Advance positions
            readPos += pitchRatio;
            if (readPos >= (float)bufSize) readPos -= (float)bufSize;
            if (readPos < 0.0f) readPos += (float)bufSize;

            writePos = (writePos + 1) % bufSize;

            // Apply to all channels
            for (int ch = 0; ch < numChannels; ++ch)
            {
                float dryCh = buffer.getSample(ch, s);
                buffer.setSample(ch, s, dryCh * (1.0f - mix * 0.5f) + shifted * mix);
            }
        }
    }

private:
    float getPitchRatio()
    {
        // Interval-to-semitone mapping
        float semitones = 0.0f;
        switch (interval)
        {
            case 0: semitones = 3.0f;   break; // Minor 3rd
            case 1: semitones = 4.0f;   break; // Major 3rd
            case 2: semitones = 5.0f;   break; // 4th
            case 3: semitones = 7.0f;   break; // 5th
            case 4: semitones = 12.0f;  break; // Octave Up
            case 5: semitones = -12.0f; break; // Octave Down
        }
        return std::pow(2.0f, semitones / 12.0f);
    }

    double sampleRate = 44100.0;
    int interval = 1; // Major 3rd default
    float mix = 0.5f;

    std::vector<float> grainBuffer;
    int writePos = 0;
    float readPos = 0.0f;
    float crossfadePos = 0.0f;
};
