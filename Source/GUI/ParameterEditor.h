#include "BinaryData.h"
#include "EQVisualizer.h"

class ParameterEditor : public juce::Component
{
public:
    ParameterEditor(juce::AudioProcessorValueTreeState& vts) 
        : apvts(vts), eqVisualizer(vts)
    {
        titleFont = juce::Font(18.0f, juce::Font::bold);
        
        addAndMakeVisible(eqVisualizer);
        eqVisualizer.setVisible(false);

        // Load background image
        auto img = juce::ImageCache::getFromMemory(
            BinaryData::guitar_bg_png, BinaryData::guitar_bg_pngSize);
// ... lines 14-36 (kept same)
        if (img.isValid())
        {
            backgroundImage = img.createCopy();
            
            // "Hapus background hitam" - make black/dark pixels transparent
            juce::Image::BitmapData data(backgroundImage, juce::Image::BitmapData::readWrite);
            for (int y = 0; y < backgroundImage.getHeight(); ++y)
            {
                for (int x = 0; x < backgroundImage.getWidth(); ++x)
                {
                    auto pixel = data.getPixelColour(x, y);
                    // If the pixel is very dark, make it transparent
                    if (pixel.getBrightness() < 0.12f)
                    {
                        data.setPixelColour(x, y, juce::Colours::transparentBlack);
                    }
                    else
                    {
                        // Keep the guitar visible
                        data.setPixelColour(x, y, pixel.withAlpha(0.9f));
                    }
                }
            }
        }
    }

    void setEffectToEdit(const juce::String& name, 
                         const std::vector<std::pair<juce::String, juce::String>>& knobParams,
                         const juce::String& modelParamID = "",
                         const juce::StringArray& modelNames = {})
    {
        activeEffectName = name;
        knobs.clear();
        modelSelector.reset();
        modelAttach.reset();

        bool isEQ = (name == "PARA EQ" || name == "GRAPHIC EQ");
        eqVisualizer.setVisible(isEQ);
        if (isEQ) eqVisualizer.setMode(name);

        if (modelParamID.isNotEmpty() && modelNames.size() > 0)
        {
            modelSelector = std::make_unique<juce::ComboBox>();
            modelSelector->addItemList(modelNames, 1);
            addAndMakeVisible(*modelSelector);
            modelAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
                apvts, modelParamID, *modelSelector);
        }

        for (auto& [label, paramId] : knobParams)
        {
            auto knob = std::make_unique<KnobComponent>(label, apvts, paramId);
            addAndMakeVisible(*knob);
            knobs.push_back(std::move(knob));
        }

        resized();
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Rack background
        g.setColour(juce::Colour(0xFF1A1A2E));
        g.fillRoundedRectangle(bounds, 10.0f);

        // Draw background image if valid
        if (backgroundImage.isValid())
        {
            g.saveState();
            g.reduceClipRegion(getLocalBounds().reduced(2));
            
            // Draw image with decent opacity
            g.setOpacity(0.5f);
            
            float imgW = (float)backgroundImage.getWidth();
            float imgH = (float)backgroundImage.getHeight();
            float panelW = (float)getWidth();
            float panelH = (float)getHeight();
            
            // Rotation for landscape (90 degrees)
            auto transform = juce::AffineTransform::rotation(juce::MathConstants<float>::halfPi, imgW / 2.0f, imgH / 2.0f);
            
            // Move to center of drawing area before scaling
            transform = transform.translated(-imgW / 2.0f, -imgH / 2.0f);
            
            // Scale to fit nicely ("80% visible")
            // Since it's rotated 90 deg, height of image becomes width
            float scale = (panelW / imgH) * 1.1f; 
            transform = transform.scaled(scale);
            
            // Move it to a balanced "landscape" position
            transform = transform.translated(panelW * 0.45f, panelH * 0.55f);

            g.drawImageTransformed(backgroundImage, transform);
            g.restoreState();
        }

        // Header line
        g.setColour(juce::Colour(0xFFE94560));
        g.fillRect(bounds.removeFromTop(4).reduced(20, 0));

        // Title
        g.setColour(juce::Colours::white);
        g.setFont(titleFont);
        g.drawText(activeEffectName, 20, 15, getWidth() - 40, 30, juce::Justification::centredTop);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced(20, 10);
        bounds.removeFromTop(40); // Title space

        if (eqVisualizer.isVisible())
        {
            eqVisualizer.setBounds(bounds.removeFromTop(80).reduced(20, 0));
            bounds.removeFromTop(10);
        }

        if (modelSelector != nullptr)
        {
            modelSelector->setBounds(bounds.removeFromTop(24).reduced(bounds.getWidth() * 0.3f, 0));
            bounds.removeFromTop(10);
        }

        if (!knobs.empty())
        {
            int knobWidth = bounds.getWidth() / (int)knobs.size();
            for (auto& knob : knobs)
            {
                knob->setBounds(bounds.removeFromLeft(knobWidth).reduced(5));
            }
        }
    }

private:
    juce::AudioProcessorValueTreeState& apvts;
    juce::String activeEffectName;
    juce::Font titleFont;
    juce::Image backgroundImage;
    EQVisualizer eqVisualizer;

    std::unique_ptr<juce::ComboBox> modelSelector;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modelAttach;
    std::vector<std::unique_ptr<KnobComponent>> knobs;
};

