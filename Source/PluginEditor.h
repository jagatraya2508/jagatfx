#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LicenseManager.h"
#include "GUI/CustomLookAndFeel.h"
#include "GUI/AmpSection.h"
#include "GUI/PedalBoard.h"
#include "GUI/PresetPanel.h"
#include "GUI/TunerComponent.h"
#include "GUI/ActivationDialog.h"

class GuitarMultiFXEditor : public juce::AudioProcessorEditor
{
public:
    explicit GuitarMultiFXEditor(GuitarMultiFXProcessor&);
    ~GuitarMultiFXEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void parentHierarchyChanged() override;

private:
    void checkLicenseStatus();
    void showActivationDialog();
    void hideActivationDialog();
    void showMainUI(bool visible);

    GuitarMultiFXProcessor& processorRef;

    CustomLookAndFeel customLookAndFeel;
    PresetPanel presetPanel;
    TunerComponent tunerComponent;
    AmpSection ampSection;
    PedalBoard pedalBoard;

    // License system
    std::unique_ptr<ActivationDialog> activationDialog;
    bool isLicensed = false;
    bool isTrialMode = false;

    // Background image
    juce::Image backgroundImage;

    // Trial banner
    juce::Label trialBanner;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GuitarMultiFXEditor)
};
