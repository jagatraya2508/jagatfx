#include "PluginEditor.h"
#include "BinaryData.h"

GuitarMultiFXEditor::GuitarMultiFXEditor(GuitarMultiFXProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      presetPanel(p.getAPVTS()),
      tunerComponent(p),
      ampSection(p.getAPVTS()),
      pedalBoard(p.getAPVTS())
{
    setLookAndFeel(&customLookAndFeel);

    addAndMakeVisible(presetPanel);
    addAndMakeVisible(tunerComponent);
    addAndMakeVisible(ampSection);
    addAndMakeVisible(pedalBoard);

    presetPanel.onLicenseClicked = [this]() { showActivationDialog(); };

    // Trial banner (hidden by default)
    trialBanner.setFont(juce::Font(12.0f, juce::Font::bold));
    trialBanner.setJustificationType(juce::Justification::centred);
    trialBanner.setColour(juce::Label::backgroundColourId, juce::Colour(0xCCE94560));
    trialBanner.setColour(juce::Label::textColourId, juce::Colours::white);
    trialBanner.setVisible(false);
    addAndMakeVisible(trialBanner);

    // Load background image
    backgroundImage = juce::ImageCache::getFromMemory(
        BinaryData::guitar_bg_png, BinaryData::guitar_bg_pngSize);

    // Set window size
    setSize(1280, 720);
    setResizable(true, true);
    setResizeLimits(1024, 600, 1920, 1080);

    // Check license on startup
    checkLicenseStatus();
}

GuitarMultiFXEditor::~GuitarMultiFXEditor()
{
    setLookAndFeel(nullptr);
}

void GuitarMultiFXEditor::parentHierarchyChanged()
{
    if (auto* dw = dynamic_cast<juce::DocumentWindow*>(getTopLevelComponent()))
    {
        // Add minimize, maximize, and close buttons to the title bar
        dw->setTitleBarButtonsRequired(juce::DocumentWindow::minimiseButton |
                                       juce::DocumentWindow::maximiseButton |
                                       juce::DocumentWindow::closeButton, false);
    }
}

void GuitarMultiFXEditor::checkLicenseStatus()
{
    auto status = LicenseManager::getStatus();

    switch (status)
    {
        case LicenseManager::LicenseStatus::Licensed:
            isLicensed = true;
            isTrialMode = false;
            showMainUI(true);
            trialBanner.setVisible(false);
            break;

        case LicenseManager::LicenseStatus::Trial:
        case LicenseManager::LicenseStatus::NotActivated:
        {
            isLicensed = false;
            isTrialMode = true;
            showMainUI(true);

            int days = LicenseManager::getTrialDaysRemaining();
            trialBanner.setText("TRIAL MODE - Sisa " + juce::String(days) +
                                " hari | Klik di sini untuk aktivasi",
                                juce::dontSendNotification);
            trialBanner.setVisible(true);

            // Make banner clickable
            trialBanner.setInterceptsMouseClicks(true, false);
            trialBanner.addMouseListener(this, false);

            // Also show activation dialog overlay
            showActivationDialog();
            break;
        }

        case LicenseManager::LicenseStatus::TrialExpired:
            isLicensed = false;
            isTrialMode = false;
            showMainUI(false);
            showActivationDialog();
            break;

        case LicenseManager::LicenseStatus::Expired:
            isLicensed = false;
            isTrialMode = false;
            showMainUI(false);
            showActivationDialog();
            break;
    }
}

void GuitarMultiFXEditor::showActivationDialog()
{
    activationDialog = std::make_unique<ActivationDialog>();

    activationDialog->onActivationSuccess = [this]()
    {
        isLicensed = true;
        isTrialMode = false;
        hideActivationDialog();
        showMainUI(true);
        trialBanner.setVisible(false);
        repaint();
    };

    activationDialog->onTrialContinue = [this]()
    {
        hideActivationDialog();
        showMainUI(true);
    };

    activationDialog->onCloseClicked = [this]()
    {
        hideActivationDialog();
        showMainUI(true);
    };

    addAndMakeVisible(activationDialog.get());
    activationDialog->setBounds(getLocalBounds());
    activationDialog->toFront(true);

    // Bring trial banner above dialog too
    if (trialBanner.isVisible())
        trialBanner.toFront(false);
}

void GuitarMultiFXEditor::hideActivationDialog()
{
    if (activationDialog)
    {
        removeChildComponent(activationDialog.get());
        activationDialog.reset();
    }
}

void GuitarMultiFXEditor::showMainUI(bool visible)
{
    presetPanel.setVisible(visible);
    tunerComponent.setVisible(visible);
    ampSection.setVisible(visible);
    pedalBoard.setVisible(visible);
}

void GuitarMultiFXEditor::paint(juce::Graphics& g)
{
    // Dark gradient background
    juce::ColourGradient bgGradient(
        juce::Colour(0xFF0A0A1A), 0, 0,
        juce::Colour(0xFF16213E), 0, (float)getHeight(), false);
    g.setGradientFill(bgGradient);
    g.fillAll();

    // Subtle pattern overlay
    g.setColour(juce::Colour(0x08FFFFFF));
    for (int y = 0; y < getHeight(); y += 4)
        g.drawHorizontalLine(y, 0, (float)getWidth());

    if (isLicensed || isTrialMode)
    {
        // Signal flow indicator between sections
        auto ampBottom = ampSection.getBottom();
        auto pedalTop = pedalBoard.getY();
        int midX = getWidth() / 2;

        g.setColour(juce::Colour(0xFFE94560));
        g.drawArrow(
            juce::Line<float>((float)midX, (float)ampBottom + 2, (float)midX, (float)pedalTop - 2),
            2.0f, 8.0f, 8.0f);

        // Signal flow label
        g.setFont(9.0f);
        g.setColour(juce::Colour(0xFF555577));
        g.drawText("SIGNAL FLOW", midX - 40, (ampBottom + pedalTop) / 2 - 6, 80, 12,
                   juce::Justification::centred);
    }

    // If not licensed and not trial, show message
    if (!isLicensed && !isTrialMode)
    {
        g.setFont(16.0f);
        g.setColour(juce::Colour(0xFF888888));
        g.drawText("Silakan aktivasi lisensi untuk menggunakan JAGAT MULTI FX",
                   getLocalBounds(), juce::Justification::centred);
    }
}

void GuitarMultiFXEditor::resized()
{
    auto bounds = getLocalBounds();

    // Trial banner at the very top
    if (trialBanner.isVisible())
    {
        trialBanner.setBounds(bounds.removeFromTop(24));
    }

    // Activation dialog covers everything
    if (activationDialog)
        activationDialog->setBounds(getLocalBounds());

    bounds = bounds.reduced(8);

    // Top: Preset panel (fixed height)
    presetPanel.setBounds(bounds.removeFromTop(40));
    bounds.removeFromTop(6); // spacing

    // Top-Mid: Tuner Component (small band)
    tunerComponent.setBounds(bounds.removeFromTop(45).withSizeKeepingCentre(400, 45));
    bounds.removeFromTop(6);

    // Middle: Amp section (~30% of remaining to give PedalBoard more space)
    int ampHeight = (int)((float)bounds.getHeight() * 0.30f);
    ampSection.setBounds(bounds.removeFromTop(ampHeight));
    bounds.removeFromTop(6); // spacing for signal flow arrow

    // Bottom: Pedal board (remaining space)
    pedalBoard.setBounds(bounds);
}
