#pragma once
#include <JuceHeader.h>
#include <cmath>
#include <vector>

class EQVisualizer : public juce::Component, public juce::AudioProcessorValueTreeState::Listener
{
public:
    EQVisualizer(juce::AudioProcessorValueTreeState& vts) : apvts(vts)
    {
        setInterceptsMouseClicks(false, false);
    }

    ~EQVisualizer()
    {
        stopListening();
    }

    void setMode(const juce::String& effectName)
    {
        stopListening();
        mode = effectName;
        
        if (mode == "PARA EQ")
        {
            for (int i = 0; i < 4; ++i)
            {
                apvts.addParameterListener("peqFreq" + juce::String(i), this);
                apvts.addParameterListener("peqGain" + juce::String(i), this);
                apvts.addParameterListener("peqQ" + juce::String(i), this);
            }
        }
        else if (mode == "GRAPHIC EQ")
        {
            for (int i = 0; i < 10; ++i)
            {
                apvts.addParameterListener("geqBand" + juce::String(i), this);
            }
        }
        
        createPath();
        repaint();
    }

    void parameterChanged(const juce::String& parameterID, float newValue) override
    {
        juce::MessageManager::callAsync([this] { 
            createPath(); 
            repaint(); 
        });
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Background for the graph
        g.setColour(juce::Colours::black.withAlpha(0.4f));
        g.fillRoundedRectangle(bounds, 6.0f);
        
        // Draw grid
        g.setColour(juce::Colours::white.withAlpha(0.1f));
        
        // Horizontal lines (dB)
        for (int db = -18; db <= 18; db += 6)
        {
            float y = juce::jmap((float)db, -18.0f, 18.0f, bounds.getBottom(), bounds.getY());
            g.drawHorizontalLine((int)y, bounds.getX(), bounds.getRight());
            
            if (db != 0)
            {
                g.setFont(10.0f);
                g.drawText(juce::String(db), bounds.getX() + 2, y - 10, 20, 10, juce::Justification::left);
            }
        }

        // Vertical lines (log frequency)
        float freqs[] = { 100, 1000, 10000 };
        for (float f : freqs)
        {
            float x = getXForFreq(f);
            g.drawVerticalLine((int)x, bounds.getY(), bounds.getBottom());
            
            g.setFont(10.0f);
            juce::String label = f >= 1000 ? juce::String(f/1000, 0) + "k" : juce::String(f, 0);
            g.drawText(label, x + 2, bounds.getBottom() - 12, 30, 10, juce::Justification::left);
        }

        // Draw response curve
        g.setColour(juce::Colour(0xFFE94560));
        g.strokePath(responsePath, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        
        // Fill area under curve
        juce::Path fillPath = responsePath;
        fillPath.lineTo(bounds.getRight(), bounds.getBottom());
        fillPath.lineTo(bounds.getX(), bounds.getBottom());
        fillPath.closeSubPath();
        
        juce::ColourGradient gradient(juce::Colour(0xFFE94560).withAlpha(0.3f), 0, bounds.getY(),
                                     juce::Colour(0xFFE94560).withAlpha(0.0f), 0, bounds.getBottom(), false);
        g.setGradientFill(gradient);
        g.fillPath(fillPath);
    }

    void resized() override
    {
        createPath();
    }

private:
    void stopListening()
    {
        if (mode == "PARA EQ")
        {
            for (int i = 0; i < 4; ++i)
            {
                apvts.removeParameterListener("peqFreq" + juce::String(i), this);
                apvts.removeParameterListener("peqGain" + juce::String(i), this);
                apvts.removeParameterListener("peqQ" + juce::String(i), this);
            }
        }
        else if (mode == "GRAPHIC EQ")
        {
            for (int i = 0; i < 10; ++i)
            {
                apvts.removeParameterListener("geqBand" + juce::String(i), this);
            }
        }
    }

    float getXForFreq(float freq)
    {
        return juce::jmap(std::log10(freq), std::log10(20.0f), std::log10(20000.0f), 0.0f, (float)getWidth());
    }

    void createPath()
    {
        responsePath.clear();
        auto bounds = getLocalBounds().toFloat();
        if (bounds.isEmpty()) return;

        const int numPoints = getWidth();
        double sampleRate = 44100.0;

        std::vector<double> magnitudes(numPoints, 1.0);

        if (mode == "PARA EQ")
        {
            for (int i = 0; i < 4; ++i)
            {
                float f = *apvts.getRawParameterValue("peqFreq" + juce::String(i));
                float g = *apvts.getRawParameterValue("peqGain" + juce::String(i));
                float q = *apvts.getRawParameterValue("peqQ" + juce::String(i));
                float gain = juce::Decibels::decibelsToGain(g);

                juce::dsp::IIR::Coefficients<float>::Ptr coeffs;
                if (i == 0) coeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, f, q, gain);
                else if (i == 3) coeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, f, q, gain);
                else coeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, f, q, gain);

                for (int p = 0; p < numPoints; ++p)
                {
                    float freq = std::pow(10.0f, juce::jmap((float)p / numPoints, 0.0f, 1.0f, std::log10(20.0f), std::log10(20000.0f)));
                    magnitudes[p] *= coeffs->getMagnitudeForFrequency(freq, sampleRate);
                }
            }
        }
        else if (mode == "GRAPHIC EQ")
        {
            float geqFreqs[] = { 31.0f, 63.0f, 125.0f, 250.0f, 500.0f, 1000.0f, 2000.0f, 4000.0f, 8000.0f, 16000.0f };
            for (int i = 0; i < 10; ++i)
            {
                float g = *apvts.getRawParameterValue("geqBand" + juce::String(i));
                float gain = juce::Decibels::decibelsToGain(g);

                auto coeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, geqFreqs[i], 1.4f, gain);

                for (int p = 0; p < numPoints; ++p)
                {
                    float freq = std::pow(10.0f, juce::jmap((float)p / numPoints, 0.0f, 1.0f, std::log10(20.0f), std::log10(20000.0f)));
                    magnitudes[p] *= coeffs->getMagnitudeForFrequency(freq, sampleRate);
                }
            }
        }

        for (int p = 0; p < numPoints; ++p)
        {
            float magDb = juce::Decibels::gainToDecibels((float)magnitudes[p]);
            float y = juce::jmap(magDb, -18.0f, 18.0f, bounds.getBottom(), bounds.getY());
            
            if (p == 0) responsePath.startNewSubPath(0, y);
            else responsePath.lineTo((float)p, y);
        }
    }

    juce::AudioProcessorValueTreeState& apvts;
    juce::String mode;
    juce::Path responsePath;
};
