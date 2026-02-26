#pragma once
#include <JuceHeader.h>

/**
 * StringSynth - Mengubah sinyal gitar menjadi suara mirip organ/harmonika.
 * Menggunakan waveshaping, multi-layer pitch shifting, ensemble/vibrato, dan breath noise.
 */
class StringSynth
{
public:
    StringSynth() = default;

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        
        // Pitch shifter buffer
        int bufSize = static_cast<int>(sampleRate * 0.3); 
        grainBuffer.resize(bufSize, 0.0f);
        writePos = 0;
        
        for (int i = 0; i < 4; ++i) 
        {
            readPos[i] = 0.0f;
            readPosDetune[i] = 0.5f; 
        }

        // Modulation states
        lfoPhase1 = 0.0f;
        lfoPhase2 = 0.0f;
        
        for (int i = 0; i < 4; ++i) filterState[i] = 0.0f;
        
        envelope = 0.0f;
        random.setSeed(42);
        pitchScoop = 0.0f;
    }

    void setAttack(float attackMs) { attackTime = juce::jmax(10.0f, attackMs); }
    void setOctaveMix(float mix) { octaveMix = mix; } 
    void setBrightness(float freq) { brightness = freq; }
    void setResonance(float res) { resonance = juce::jlimit(0.0f, 0.95f, res); }
    void setMix(float m) { mix = m; }

    void process(juce::AudioBuffer<float>& buffer)
    {
        int numSamples = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();
        int bufSize = (int)grainBuffer.size();

        for (int s = 0; s < numSamples; ++s)
        {
            float input = buffer.getSample(0, s);
            
            // 1. Envelope Follower & Pitch Scoop
            float absIn = std::abs(input);
            float attackAlpha = 1.0f / (attackTime * sampleRate * 0.001f + 1.0f);
            
            if (absIn > envelope)
            {
                // Trigger pitch scoop (momentary flat pitch on attack like harmonica)
                if (absIn > 0.05f && envelope < 0.01f) 
                    pitchScoop = -0.05f; 
                
                envelope += (absIn - envelope) * attackAlpha;
            }
            else
            {
                envelope += (absIn - envelope) * 0.0003f;
            }
            
            // Decay pitch scoop back to unison
            pitchScoop *= 0.999f;

            // 2. Modulations (Ensemble + Hand Vibrato)
            // LFO 1: Slow Ensemble (0.5 Hz)
            lfoPhase1 += (0.5f * 2.0f * juce::MathConstants<float>::pi) / (float)sampleRate;
            if (lfoPhase1 >= juce::MathConstants<float>::twoPi) lfoPhase1 -= juce::MathConstants<float>::twoPi;
            
            // LFO 2: Fast Vibrato (6.0 Hz for harmonica/hand)
            lfoPhase2 += (6.0f * 2.0f * juce::MathConstants<float>::pi) / (float)sampleRate;
            if (lfoPhase2 >= juce::MathConstants<float>::twoPi) lfoPhase2 -= juce::MathConstants<float>::twoPi;

            float ensembleMod = std::sin(lfoPhase1) * 0.004f;
            float vibratoMod = std::sin(lfoPhase2) * 0.008f * (octaveMix > 0.5f ? 1.0f : 0.2f);
            float totalPitchOffset = ensembleMod + vibratoMod + pitchScoop;

            // 3. Multi-Layer Synth (Organ/Harmonica Reed Logic)
            grainBuffer[writePos] = input;
            
            float synthSum = 0.0f;
            float ratios[4] = { 0.5f, 1.0f, 2.0f, 3.0f }; // Sub, Fund, Oct, 12th (Harmonica often has 3rd harmonic)
            float gains[4] =  { 0.3f, 0.4f, 0.3f, 0.2f }; 

            for (int i = 0; i < 4; ++i)
            {
                float layer = getGrainSample(readPos[i], bufSize);
                float layerDetune = getGrainSample(readPosDetune[i], bufSize);
                
                synthSum += (layer + layerDetune) * gains[i] * 0.5f;

                readPos[i] += ratios[i] + totalPitchOffset;
                readPosDetune[i] += ratios[i] - (totalPitchOffset * 0.5f);
                
                if (readPos[i] >= (float)bufSize) readPos[i] -= (float)bufSize;
                if (readPosDetune[i] >= (float)bufSize) readPosDetune[i] -= (float)bufSize;
            }

            writePos = (writePos + 1) % bufSize;

            // 4. Breath Noise & Reedy Saturation
            float breath = (random.nextFloat() * 2.0f - 1.0f) * 0.05f * std::pow(envelope, 0.5f);
            float reedySource = synthSum + breath;
            
            // Asymmetric saturation for reedy/serak sound (even+odd harmonics)
            float saturated = std::tanh(reedySource * 4.0f);
            if (saturated > 0) saturated = std::pow(saturated, 0.8f); // Softening positive peak
            
            // 5. Ladder Filter
            float cutoff = brightness / (sampleRate * 0.5f);
            cutoff = juce::jlimit(0.01f, 0.99f, cutoff);
            float g = resonance * 3.7f;
            float inputToFilter = saturated - g * filterState[3];
            
            for (int i = 0; i < 4; ++i)
            {
                float prev = (i == 0) ? inputToFilter : filterState[i-1];
                filterState[i] += cutoff * (std::tanh(prev) - std::tanh(filterState[i]));
            }
            
            float synthOutput = filterState[3] * envelope;

            for (int ch = 0; ch < numChannels; ++ch)
            {
                float dry = buffer.getSample(ch, s);
                buffer.setSample(ch, s, dry * (1.0f - mix) + synthOutput * mix);
            }
        }
    }

private:
    float getGrainSample(float pos, int bufSize)
    {
        int p1 = (int)pos % bufSize;
        int p2 = (p1 + 1) % bufSize;
        float frac = pos - (float)((int)pos);
        return grainBuffer[p1] * (1.0f - frac) + grainBuffer[p2] * frac;
    }

    double sampleRate = 44100.0;
    float attackTime = 150.0f;
    float octaveMix = 0.5f;
    float brightness = 2000.0f;
    float resonance = 0.5f;
    float mix = 0.5f;

    std::vector<float> grainBuffer;
    int writePos = 0;
    float readPos[4];
    float readPosDetune[4];

    float lfoPhase1 = 0.0f, lfoPhase2 = 0.0f;
    float filterState[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    float envelope = 0.0f;
    float pitchScoop = 0.0f;
    juce::Random random;
};
