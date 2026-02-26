#pragma once
#include <JuceHeader.h>
#include "../LicenseManager.h"

class ActivationDialog : public juce::Component
{
public:
    ActivationDialog()
    {
        // ==================== Title ====================
        titleLabel.setText("JAGAT MULTI FX", juce::dontSendNotification);
        titleLabel.setFont(juce::Font(28.0f, juce::Font::bold));
        titleLabel.setJustificationType(juce::Justification::centred);
        titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFE94560));
        addAndMakeVisible(titleLabel);

        subtitleLabel.setText("License Activation", juce::dontSendNotification);
        subtitleLabel.setFont(juce::Font(14.0f));
        subtitleLabel.setJustificationType(juce::Justification::centred);
        subtitleLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFAAAAAA));
        addAndMakeVisible(subtitleLabel);

        // ==================== Hardware ID ====================
        hwIdTitleLabel.setText("Hardware ID (kirim ke developer):", juce::dontSendNotification);
        hwIdTitleLabel.setFont(juce::Font(12.0f));
        hwIdTitleLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF888888));
        addAndMakeVisible(hwIdTitleLabel);

        hardwareId = LicenseManager::getHardwareId();
        hwIdLabel.setText(hardwareId, juce::dontSendNotification);
        hwIdLabel.setFont(juce::Font(18.0f, juce::Font::bold));
        hwIdLabel.setJustificationType(juce::Justification::centred);
        hwIdLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF00D4FF));
        hwIdLabel.setColour(juce::Label::backgroundColourId, juce::Colour(0xFF1A1A2E));
        hwIdLabel.setEditable(false);
        addAndMakeVisible(hwIdLabel);

        copyButton.setButtonText("Copy Hardware ID");
        copyButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2D2D5E));
        copyButton.setColour(juce::TextButton::textColourOnId, juce::Colour(0xFF00D4FF));
        copyButton.onClick = [this]()
        {
            juce::SystemClipboard::copyTextToClipboard(hardwareId);
            copyButton.setButtonText("Copied!");
            juce::Timer::callAfterDelay(2000, [this]() {
                if (this != nullptr)
                    copyButton.setButtonText("Copy Hardware ID");
            });
        };
        addAndMakeVisible(copyButton);

        // ==================== License Key Input ====================
        licenseLabel.setText("Masukkan License Key:", juce::dontSendNotification);
        licenseLabel.setFont(juce::Font(12.0f));
        licenseLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF888888));
        addAndMakeVisible(licenseLabel);

        licenseInput.setTextToShowWhenEmpty("XXXX-XXXX-XXXX-XXXX", juce::Colour(0xFF555555));
        licenseInput.setFont(juce::Font(16.0f, juce::Font::bold));
        licenseInput.setJustification(juce::Justification::centred);
        licenseInput.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xFF1A1A2E));
        licenseInput.setColour(juce::TextEditor::textColourId, juce::Colour(0xFFFFFFFF));
        licenseInput.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xFF3D3D6E));
        licenseInput.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xFFE94560));
        addAndMakeVisible(licenseInput);

        // ==================== Activate Button ====================
        activateButton.setButtonText("ACTIVATE");
        activateButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFFE94560));
        activateButton.setColour(juce::TextButton::textColourOnId, juce::Colour(0xFFFFFFFF));
        activateButton.onClick = [this]() { attemptActivation(); };
        addAndMakeVisible(activateButton);

        // ==================== Status Message ====================
        statusLabel.setText("", juce::dontSendNotification);
        statusLabel.setFont(juce::Font(12.0f));
        statusLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(statusLabel);

        // ==================== Trial Info ====================
        updateTrialInfo();
        trialLabel.setFont(juce::Font(12.0f));
        trialLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(trialLabel);

        // ==================== Continue Trial Button ====================
        continueTrialButton.setButtonText("Lanjut Trial");
        continueTrialButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2D2D5E));
        continueTrialButton.setColour(juce::TextButton::textColourOnId, juce::Colour(0xFFAAAAAA));
        continueTrialButton.onClick = [this]()
        {
            if (onTrialContinue)
                onTrialContinue();
        };
        addAndMakeVisible(continueTrialButton);

        // ==================== Close Button ====================
        closeButton.setButtonText("Tutup");
        closeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2D2D5E));
        closeButton.setColour(juce::TextButton::textColourOnId, juce::Colour(0xFFFFFFFF));
        closeButton.onClick = [this]()
        {
            if (onCloseClicked)
                onCloseClicked();
            else if (onTrialContinue) // fallback
                onTrialContinue();
        };
        addAndMakeVisible(closeButton);
    }

    void updateTrialInfo()
    {
        auto status = LicenseManager::getStatus();
        
        // Hide both by default, reveal as needed
        continueTrialButton.setVisible(false);
        closeButton.setVisible(false);

        if (status == LicenseManager::LicenseStatus::Trial)
        {
            int days = LicenseManager::getTrialDaysRemaining();
            trialLabel.setText("Trial aktif - sisa " + juce::String(days) + " hari",
                               juce::dontSendNotification);
            trialLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF00CC66));
            continueTrialButton.setVisible(true);
        }
        else if (status == LicenseManager::LicenseStatus::TrialExpired)
        {
            trialLabel.setText("Trial telah berakhir. Silakan aktivasi dengan license key.",
                               juce::dontSendNotification);
            trialLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFFF4444));
        }
        else if (status == LicenseManager::LicenseStatus::Expired)
        {
            trialLabel.setText("Masa aktif lisensi Anda telah habis. Silakan perpanjang lisensi.",
                               juce::dontSendNotification);
            trialLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFFF8C00)); // Orange warning color
        }
        else if (status == LicenseManager::LicenseStatus::NotActivated)
        {
            trialLabel.setText("Selamat datang! Trial 7 hari dimulai hari ini.",
                               juce::dontSendNotification);
            trialLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF00CC66));
            continueTrialButton.setVisible(true);
        }
        else if (status == LicenseManager::LicenseStatus::Licensed)
        {
            // NEW: Show expiry date or lifetime status
            juce::String key = LicenseManager::loadLicense();
            juce::int64 expiry = LicenseManager::getLicenseExpiry(key);
            
            if (expiry < 0)
            {
                trialLabel.setText("Status: AKTIF (Lifetime)", juce::dontSendNotification);
            }
            else
            {
                juce::Time expiryTime(expiry);
                trialLabel.setText("Status: AKTIF (Habis pada " + expiryTime.formatted("%d-%m-%Y") + ")", 
                                   juce::dontSendNotification);
            }
            trialLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF00CC66));
            closeButton.setVisible(true);
        }
    }

    void paint(juce::Graphics& g) override
    {
        // Semi-transparent dark overlay
        g.fillAll(juce::Colour(0xEE0A0A1A));

        // Central dialog box
        auto dialogBounds = getDialogBounds();

        // Shadow
        g.setColour(juce::Colour(0x40000000));
        g.fillRoundedRectangle(dialogBounds.toFloat().translated(3, 3), 12.0f);

        // Background
        juce::ColourGradient bg(
            juce::Colour(0xFF16213E), dialogBounds.getX(), dialogBounds.getY(),
            juce::Colour(0xFF0F1629), dialogBounds.getX(), dialogBounds.getBottom(), false);
        g.setGradientFill(bg);
        g.fillRoundedRectangle(dialogBounds.toFloat(), 12.0f);

        // Border
        g.setColour(juce::Colour(0xFF2D2D5E));
        g.drawRoundedRectangle(dialogBounds.toFloat(), 12.0f, 2.0f);

        // Accent line at top
        g.setColour(juce::Colour(0xFFE94560));
        auto topLine = dialogBounds.toFloat().removeFromTop(3);
        g.fillRoundedRectangle(topLine.reduced(20, 0), 2.0f);
    }

    void resized() override
    {
        auto dialog = getDialogBounds();
        auto area = dialog.reduced(30, 20);

        titleLabel.setBounds(area.removeFromTop(35));
        subtitleLabel.setBounds(area.removeFromTop(20));
        area.removeFromTop(15);

        // Trial info
        trialLabel.setBounds(area.removeFromTop(20));
        area.removeFromTop(10);

        // Hardware ID
        hwIdTitleLabel.setBounds(area.removeFromTop(18));
        area.removeFromTop(4);
        hwIdLabel.setBounds(area.removeFromTop(30));
        area.removeFromTop(6);
        copyButton.setBounds(area.removeFromTop(28).withSizeKeepingCentre(160, 28));
        area.removeFromTop(15);

        // License input
        licenseLabel.setBounds(area.removeFromTop(18));
        area.removeFromTop(4);
        licenseInput.setBounds(area.removeFromTop(35).reduced(40, 0));
        area.removeFromTop(10);

        // Status
        statusLabel.setBounds(area.removeFromTop(20));
        area.removeFromTop(8);

        // Buttons
        auto buttonArea = area.removeFromTop(38);
        auto buttonWidth = 140;
        
        // Count visible buttons to center them
        int visibleButtons = 1; // Activate is always visible
        if (continueTrialButton.isVisible()) visibleButtons++;
        if (closeButton.isVisible()) visibleButtons++;
        
        auto totalWidth = (buttonWidth * visibleButtons) + (10 * (visibleButtons - 1));
        auto currentX = buttonArea.getCentreX() - totalWidth / 2;

        activateButton.setBounds(currentX, buttonArea.getY(), buttonWidth, 38);
        currentX += buttonWidth + 10;
        
        if (continueTrialButton.isVisible())
        {
            continueTrialButton.setBounds(currentX, buttonArea.getY(), buttonWidth, 38);
            currentX += buttonWidth + 10;
        }
        
        if (closeButton.isVisible())
        {
            closeButton.setBounds(currentX, buttonArea.getY(), buttonWidth, 38);
        }
    }

    // Callbacks
    std::function<void()> onActivationSuccess;
    std::function<void()> onTrialContinue;
    std::function<void()> onCloseClicked;

private:
    juce::Rectangle<int> getDialogBounds() const
    {
        int w = juce::jmin(500, getWidth() - 40);
        int h = juce::jmin(420, getHeight() - 40);
        return juce::Rectangle<int>(0, 0, w, h).withCentre(getLocalBounds().getCentre());
    }

    void attemptActivation()
    {
        auto key = licenseInput.getText().trim();
        if (key.isEmpty())
        {
            showStatus("Masukkan license key terlebih dahulu!", false);
            return;
        }

        if (LicenseManager::validateLicense(key, hardwareId))
        {
            LicenseManager::saveLicense(key);
            showStatus("Aktivasi berhasil!", true);

            // Delay a bit then callback
            juce::Timer::callAfterDelay(1000, [this]()
            {
                if (onActivationSuccess)
                    onActivationSuccess();
            });
        }
        else
        {
            showStatus("License key tidak valid!", false);
        }
    }

    void showStatus(const juce::String& msg, bool success)
    {
        statusLabel.setText(msg, juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId,
                              success ? juce::Colour(0xFF00CC66) : juce::Colour(0xFFFF4444));
    }

    juce::String hardwareId;

    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::Label hwIdTitleLabel;
    juce::Label hwIdLabel;
    juce::TextButton copyButton;
    juce::Label licenseLabel;
    juce::TextEditor licenseInput;
    juce::TextButton activateButton;
    juce::Label statusLabel;
    juce::Label trialLabel;
    juce::TextButton continueTrialButton;
    juce::TextButton closeButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ActivationDialog)
};
