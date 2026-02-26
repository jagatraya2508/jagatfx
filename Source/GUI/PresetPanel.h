#pragma once
#include <JuceHeader.h>

class PresetPanel : public juce::Component
{
public:
    PresetPanel(juce::AudioProcessorValueTreeState& apvts)
        : valueTreeState(apvts)
    {
        // Preset selector - Original + Famous Guitarist Tones
        presetSelector.addItem("Default Clean", 1);
        presetSelector.addItem("Warm Crunch", 2);
        presetSelector.addItem("Classic Rock", 3);
        presetSelector.addItem("Heavy Metal", 4);
        presetSelector.addItem("Djent Machine", 5);
        presetSelector.addItem("Ambient Clean", 6);
        presetSelector.addItem("Blues Lead", 7);
        presetSelector.addItem("Prog Rock", 8);
        // --- Famous Guitarist Tones ---
        presetSelector.addItem("Jimi Hendrix - Purple Haze", 9);
        presetSelector.addItem("Slash - Sweet Child", 10);
        presetSelector.addItem("John Mayer - Clean Blues", 11);
        presetSelector.addItem("Metallica - Master of Puppets", 12);
        presetSelector.addItem("Eddie Van Halen - Brown Sound", 13);
        presetSelector.addItem("David Gilmour - Comfortably Numb", 14);
        presetSelector.addItem("Kurt Cobain - Smells Like Teen", 15);
        presetSelector.addItem("Stevie Ray Vaughan - Texas Flood", 16);
        presetSelector.addItem("Angus Young - Highway to Hell", 17);
        presetSelector.addItem("Mark Knopfler - Sultans Clean", 18);
        presetSelector.addItem("Santana - Smooth Lead", 19);
        presetSelector.addItem("Joe Satriani - Surfing", 20);
        presetSelector.addItem("BB King - Lucille Blues", 21);
        presetSelector.addItem("Meshuggah - Djent", 22);
        presetSelector.addItem("Soldano - Massive Lead", 23);
        // --- New Guitarist Tones ---
        presetSelector.addItem("Paul Gilbert - Shred Machine", 24);
        presetSelector.addItem("Yngwie Malmsteen - Neoclassical", 25);
        presetSelector.addItem("Synyster Gates - A7X Lead", 26);
        presetSelector.addItem("Marty Friedman - Exotic Lead", 27);
        presetSelector.addItem("Vito Bratta - Melodic Rock", 28);
        presetSelector.addItem("Steve Vai - Liquid Lead", 29);
        presetSelector.addItem("Eddie Van Halen - Hot Rod", 30);
        presetSelector.addItem("Brian May - Queen Tone", 31);
        presetSelector.addItem("Grand Organ Synth", 32);
        presetSelector.addItem("Funky Auto Wah", 33);
        presetSelector.addItem("Blues Harmonica", 34);
        presetSelector.addItem("Reed Organ / Accordion", 35);
        presetSelector.setSelectedId(1, juce::dontSendNotification);
        presetSelector.onChange = [this]() { applyPreset(presetSelector.getSelectedId()); };
        addAndMakeVisible(presetSelector);

        // Save/Load buttons
        saveBtn.setButtonText("SAVE");
        loadBtn.setButtonText("LOAD");
        prevBtn.setButtonText("<");
        nextBtn.setButtonText(">");
        licenseBtn.setButtonText("LICENSE");
        licenseBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2D2D5E));
        licenseBtn.setColour(juce::TextButton::textColourOnId, juce::Colour(0xFF00D4FF));
        
        addAndMakeVisible(saveBtn);
        addAndMakeVisible(loadBtn);
        addAndMakeVisible(prevBtn);
        addAndMakeVisible(nextBtn);
        addAndMakeVisible(licenseBtn);

        // Button actions
        saveBtn.onClick = [this]() { savePreset(); };
        loadBtn.onClick = [this]() { loadPreset(); };
        prevBtn.onClick = [this]()
        {
            int current = presetSelector.getSelectedId();
            if (current > 1) presetSelector.setSelectedId(current - 1);
        };
        nextBtn.onClick = [this]()
        {
            int current = presetSelector.getSelectedId();
            if (current < presetSelector.getNumItems()) presetSelector.setSelectedId(current + 1);
        };
        licenseBtn.onClick = [this]()
        {
            if (onLicenseClicked)
                onLicenseClicked();
        };

        // Title
        titleLabel.setText("JAGAT MULTI FX", juce::dontSendNotification);
        titleLabel.setFont(juce::Font(18.0f, juce::Font::bold));
        titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFE94560));
        titleLabel.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(titleLabel);

        // Developer Credit
        developerLabel.setText("Created by Wisnu Wardana", juce::dontSendNotification);
        developerLabel.setFont(juce::Font(10.0f));
        developerLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFAAAAAA));
        developerLabel.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(developerLabel);

        presetLabel.setText("PRESET:", juce::dontSendNotification);
        presetLabel.setFont(juce::Font(12.0f));
        presetLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF8899AA));
        presetLabel.setJustificationType(juce::Justification::centredRight);
        addAndMakeVisible(presetLabel);
    }

    void paint(juce::Graphics& g) override
    {
        // Top bar background (semi-transparent)
        g.setColour(juce::Colour(0xB01A1A2E));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 8.0f);
        g.setColour(juce::Colour(0xFF333355));
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1), 6.0f, 1.0f);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced(8);

        auto titleArea = bounds.removeFromLeft(330);
        titleLabel.setBounds(titleArea.removeFromTop(titleArea.getHeight() * 0.7f));
        developerLabel.setBounds(titleArea);

        licenseBtn.setBounds(bounds.removeFromRight(70).reduced(2));
        saveBtn.setBounds(bounds.removeFromRight(50).reduced(2));
        loadBtn.setBounds(bounds.removeFromRight(50).reduced(2));
        nextBtn.setBounds(bounds.removeFromRight(30).reduced(2));
        presetSelector.setBounds(bounds.removeFromRight(200).reduced(2));
        prevBtn.setBounds(bounds.removeFromRight(30).reduced(2));
        presetLabel.setBounds(bounds.removeFromRight(60));
    }

private:
    // Helper to set a parameter value
    void setParam(const juce::String& paramId, float value)
    {
        if (auto* param = valueTreeState.getParameter(paramId))
            param->setValueNotifyingHost(param->convertTo0to1(value));
    }

    void applyPreset(int presetId)
    {
        // Reset all effects off first
        setParam("compEnabled", 0.0f);
        setParam("odEnabled", 0.0f);
        setParam("distEnabled", 0.0f);
        setParam("hgEnabled", 0.0f);
        setParam("chorusEnabled", 0.0f);
        setParam("flangerEnabled", 0.0f);
        setParam("phaserEnabled", 0.0f);
        setParam("harmEnabled", 0.0f);
        setParam("delayEnabled", 0.0f);
        setParam("reverbEnabled", 0.0f);
        setParam("peqEnabled", 0.0f);
        setParam("geqEnabled", 0.0f);
        setParam("stringEnabled", 0.0f);
        setParam("autoWahEnabled", 0.0f);
        setParam("talkEnabled", 0.0f);

        // Keep gate and amp always on
        setParam("gateEnabled", 1.0f);
        setParam("ampEnabled", 1.0f);
        setParam("cabEnabled", 1.0f);

        switch (presetId)
        {
        case 1: // Default Clean
            setParam("ampModel", 0.0f);  // Clean
            setParam("ampGain", 3.0f);
            setParam("ampChannel", 5.0f);
            setParam("tsBass", 5.0f);
            setParam("tsMid", 5.0f);
            setParam("tsTreble", 5.0f);
            setParam("paPresence", 5.0f);
            setParam("paResonance", 5.0f);
            setParam("paMaster", 5.0f);
            setParam("cabModel", 0.0f);  // 1x12 Open Back
            setParam("cabMic", 0.0f);
            setParam("gateThreshold", -50.0f);
            break;

        case 2: // Warm Crunch
            setParam("ampModel", 1.0f);  // Crunch
            setParam("ampGain", 5.5f);
            setParam("ampChannel", 5.5f);
            setParam("tsBass", 5.5f);
            setParam("tsMid", 6.0f);
            setParam("tsTreble", 5.5f);
            setParam("paPresence", 5.5f);
            setParam("paResonance", 5.0f);
            setParam("paMaster", 5.0f);
            setParam("cabModel", 0.0f);
            setParam("cabMic", 1.0f);    // Off-Axis
            setParam("gateThreshold", -48.0f);
            setParam("reverbEnabled", 1.0f);
            setParam("reverbModel", 1.0f);  // Room
            setParam("reverbSize", 0.3f);
            setParam("reverbMix", 0.15f);
            break;

        case 3: // Classic Rock
            setParam("ampModel", 5.0f);  // Marshall JCM
            setParam("ampGain", 6.5f);
            setParam("ampChannel", 6.0f);
            setParam("tsBass", 6.0f);
            setParam("tsMid", 6.0f);
            setParam("tsTreble", 6.5f);
            setParam("paPresence", 6.0f);
            setParam("paResonance", 5.0f);
            setParam("paMaster", 5.5f);
            setParam("cabModel", 3.0f);  // 4x12 Greenback
            setParam("cabMic", 0.0f);
            setParam("gateThreshold", -44.0f);
            setParam("reverbEnabled", 1.0f);
            setParam("reverbModel", 1.0f);  // Room
            setParam("reverbSize", 0.3f);
            setParam("reverbMix", 0.15f);
            break;

        case 4: // Heavy Metal
            setParam("ampModel", 6.0f);  // Mesa Rectifier
            setParam("ampGain", 8.0f);
            setParam("ampChannel", 7.0f);
            setParam("tsBass", 7.0f);
            setParam("tsMid", 4.5f);
            setParam("tsTreble", 7.0f);
            setParam("paPresence", 7.0f);
            setParam("paResonance", 5.0f);
            setParam("paMaster", 5.5f);
            setParam("cabModel", 2.0f);  // 4x12 V30
            setParam("cabMic", 0.0f);
            setParam("gateThreshold", -38.0f);
            setParam("hgEnabled", 1.0f);
            setParam("hgModel", 0.0f);   // Rectifier
            setParam("hgGain", 7.5f);
            setParam("hgTone", 5.5f);
            setParam("hgLevel", 5.0f);
            break;

        case 5: // Djent Machine
            setParam("ampModel", 6.0f);  // Mesa Rectifier
            setParam("ampGain", 9.0f);
            setParam("ampChannel", 7.0f);
            setParam("tsBass", 7.0f);
            setParam("tsMid", 3.0f);
            setParam("tsTreble", 7.5f);
            setParam("paPresence", 8.0f);
            setParam("paResonance", 4.0f);
            setParam("paMaster", 5.5f);
            setParam("cabModel", 2.0f);  // 4x12 V30
            setParam("cabMic", 0.0f);
            setParam("gateThreshold", -34.0f);
            setParam("hgEnabled", 1.0f);
            setParam("hgModel", 3.0f);   // Djent
            setParam("hgGain", 9.0f);
            setParam("hgTone", 6.0f);
            setParam("hgLevel", 5.0f);
            setParam("compEnabled", 1.0f);
            setParam("compModel", 2.0f);  // FET
            setParam("compThreshold", -22.0f);
            setParam("compRatio", 6.0f);
            break;

        case 6: // Ambient Clean
            setParam("ampModel", 0.0f);  // Clean
            setParam("ampGain", 2.5f);
            setParam("ampChannel", 5.5f);
            setParam("tsBass", 4.5f);
            setParam("tsMid", 5.0f);
            setParam("tsTreble", 6.5f);
            setParam("paPresence", 6.0f);
            setParam("paResonance", 4.5f);
            setParam("paMaster", 5.0f);
            setParam("cabModel", 0.0f);
            setParam("cabMic", 1.0f);    // Off-Axis
            setParam("gateThreshold", -55.0f);
            setParam("chorusEnabled", 1.0f);
            setParam("chorusRate", 0.6f);
            setParam("chorusDepth", 0.4f);
            setParam("chorusMix", 0.3f);
            setParam("delayEnabled", 1.0f);
            setParam("delayModel", 0.0f);  // Digital
            setParam("delayTime", 600.0f);
            setParam("delayFeedback", 0.45f);
            setParam("delayMix", 0.3f);
            setParam("reverbEnabled", 1.0f);
            setParam("reverbModel", 4.0f);  // Cathedral
            setParam("reverbSize", 0.8f);
            setParam("reverbDamping", 0.3f);
            setParam("reverbMix", 0.4f);
            break;

        case 7: // Blues Lead
            setParam("ampModel", 4.0f);  // Fender Twin
            setParam("ampGain", 5.5f);
            setParam("ampChannel", 6.5f);
            setParam("tsBass", 5.5f);
            setParam("tsMid", 7.0f);
            setParam("tsTreble", 6.0f);
            setParam("paPresence", 5.5f);
            setParam("paResonance", 5.0f);
            setParam("paMaster", 5.5f);
            setParam("cabModel", 0.0f);
            setParam("cabMic", 0.0f);
            setParam("gateThreshold", -48.0f);
            setParam("odEnabled", 1.0f);
            setParam("odModel", 0.0f);   // Tube Screamer
            setParam("odDrive", 5.5f);
            setParam("odTone", 5.5f);
            setParam("odLevel", 5.5f);
            setParam("reverbEnabled", 1.0f);
            setParam("reverbModel", 3.0f);  // Spring
            setParam("reverbSize", 0.4f);
            setParam("reverbMix", 0.2f);
            break;

        case 8: // Prog Rock
            setParam("ampModel", 5.0f);  // Marshall JCM
            setParam("ampGain", 6.0f);
            setParam("ampChannel", 6.0f);
            setParam("tsBass", 5.0f);
            setParam("tsMid", 6.5f);
            setParam("tsTreble", 6.5f);
            setParam("paPresence", 6.0f);
            setParam("paResonance", 5.0f);
            setParam("paMaster", 5.5f);
            setParam("cabModel", 1.0f);  // 2x12 Closed
            setParam("cabMic", 0.0f);
            setParam("gateThreshold", -45.0f);
            setParam("distEnabled", 1.0f);
            setParam("distModel", 1.0f);  // RAT
            setParam("distGain", 5.0f);
            setParam("distTone", 6.0f);
            setParam("distLevel", 5.0f);
            setParam("chorusEnabled", 1.0f);
            setParam("chorusRate", 0.8f);
            setParam("chorusDepth", 0.3f);
            setParam("chorusMix", 0.2f);
            setParam("delayEnabled", 1.0f);
            setParam("delayModel", 1.0f);  // Analog
            setParam("delayTime", 450.0f);
            setParam("delayFeedback", 0.35f);
            setParam("delayMix", 0.25f);
            setParam("reverbEnabled", 1.0f);
            setParam("reverbModel", 0.0f);  // Hall
            setParam("reverbSize", 0.5f);
            setParam("reverbMix", 0.2f);
            break;

        // ===== FAMOUS GUITARIST TONES =====

        case 9: // Jimi Hendrix - Purple Haze (Fuzz + Marshall-style)
            setParam("ampModel", 1.0f);  // Crunch
            setParam("ampGain", 7.0f);
            setParam("ampChannel", 6.0f);
            setParam("tsBass", 6.0f);
            setParam("tsMid", 4.0f);
            setParam("tsTreble", 7.0f);
            setParam("paPresence", 6.0f);
            setParam("paResonance", 5.0f);
            setParam("paMaster", 5.5f);
            setParam("cabModel", 3.0f);  // 4x12 Greenback
            setParam("cabMic", 0.0f);
            setParam("gateThreshold", -45.0f);
            // Overdrive as fuzz
            setParam("odEnabled", 1.0f);
            setParam("odModel", 0.0f);   // Tube Screamer style
            setParam("odDrive", 8.0f);
            setParam("odTone", 6.5f);
            setParam("odLevel", 5.5f);
            // Reverb - spring
            setParam("reverbEnabled", 1.0f);
            setParam("reverbModel", 3.0f);  // Spring
            setParam("reverbSize", 0.35f);
            setParam("reverbMix", 0.2f);
            break;

        case 10: // Slash - Sweet Child O' Mine (Marshall JCM800)
            setParam("ampModel", 5.0f);  // Marshall JCM
            setParam("ampGain", 7.5f);
            setParam("ampChannel", 6.5f);
            setParam("tsBass", 6.0f);
            setParam("tsMid", 7.0f);
            setParam("tsTreble", 6.5f);
            setParam("paPresence", 6.0f);
            setParam("paResonance", 5.5f);
            setParam("paMaster", 5.5f);
            setParam("cabModel", 2.0f);  // 4x12 V30
            setParam("cabMic", 0.0f);
            setParam("gateThreshold", -42.0f);
            // Slight delay
            setParam("delayEnabled", 1.0f);
            setParam("delayModel", 1.0f);  // Analog
            setParam("delayTime", 350.0f);
            setParam("delayFeedback", 0.25f);
            setParam("delayMix", 0.15f);
            // Room reverb
            setParam("reverbEnabled", 1.0f);
            setParam("reverbModel", 1.0f);  // Room
            setParam("reverbSize", 0.35f);
            setParam("reverbMix", 0.18f);
            break;

        case 11: // John Mayer - Clean Blues (Fender Twin Clean)
            setParam("ampModel", 4.0f);  // Fender Twin
            setParam("ampGain", 3.5f);
            setParam("ampChannel", 6.0f);
            setParam("tsBass", 5.5f);
            setParam("tsMid", 6.0f);
            setParam("tsTreble", 6.5f);
            setParam("paPresence", 6.0f);
            setParam("paResonance", 4.5f);
            setParam("paMaster", 5.0f);
            setParam("cabModel", 0.0f);  // 1x12 Open Back
            setParam("cabMic", 1.0f);    // Off-Axis
            setParam("gateThreshold", -55.0f);
            // Compressor
            setParam("compEnabled", 1.0f);
            setParam("compModel", 1.0f);  // Optical
            setParam("compThreshold", -18.0f);
            setParam("compRatio", 3.0f);
            setParam("compMakeup", 2.0f);
            // Light overdrive (Klon)
            setParam("odEnabled", 1.0f);
            setParam("odModel", 2.0f);   // Klon
            setParam("odDrive", 3.0f);
            setParam("odTone", 6.0f);
            setParam("odLevel", 5.5f);
            // Spring reverb
            setParam("reverbEnabled", 1.0f);
            setParam("reverbModel", 3.0f);  // Spring
            setParam("reverbSize", 0.4f);
            setParam("reverbMix", 0.25f);
            break;

        case 12: // Metallica - Master of Puppets (Mesa Boogie High Gain)
            setParam("ampModel", 6.0f);  // Mesa Rectifier
            setParam("ampGain", 8.5f);
            setParam("ampChannel", 7.0f);
            setParam("tsBass", 6.5f);
            setParam("tsMid", 4.0f);     // Scooped mids
            setParam("tsTreble", 7.0f);
            setParam("paPresence", 7.0f);
            setParam("paResonance", 5.0f);
            setParam("paMaster", 5.5f);
            setParam("cabModel", 2.0f);  // 4x12 V30
            setParam("cabMic", 0.0f);    // On-Axis
            setParam("gateThreshold", -38.0f);
            // High Gain pedal
            setParam("hgEnabled", 1.0f);
            setParam("hgModel", 0.0f);   // Rectifier
            setParam("hgGain", 8.0f);
            setParam("hgTone", 5.5f);
            setParam("hgLevel", 5.0f);
            break;

        case 13: // Eddie Van Halen - Brown Sound (Marshall + Hot Rod)
            setParam("ampModel", 5.0f);  // Marshall JCM
            setParam("ampGain", 8.5f);
            setParam("ampChannel", 7.0f);
            setParam("tsBass", 5.5f);
            setParam("tsMid", 7.0f);
            setParam("tsTreble", 6.0f);
            setParam("paPresence", 7.0f);
            setParam("paResonance", 5.5f);
            setParam("paMaster", 6.0f);
            setParam("cabModel", 3.0f);  // 4x12 Greenback
            setParam("cabMic", 0.0f);
            setParam("gateThreshold", -40.0f);
            // Phaser
            setParam("phaserEnabled", 1.0f);
            setParam("phaserRate", 0.8f);
            setParam("phaserDepth", 0.4f);
            setParam("phaserFeedback", 0.5f);
            setParam("phaserMix", 0.25f);
            // Flanger for eruption tone
            setParam("flangerEnabled", 1.0f);
            setParam("flangerRate", 0.3f);
            setParam("flangerDepth", 0.35f);
            setParam("flangerFeedback", 0.4f);
            setParam("flangerMix", 0.2f);
            break;

        case 14: // David Gilmour - Comfortably Numb (Big Muff + Delay)
            setParam("ampModel", 4.0f);  // Fender Twin
            setParam("ampGain", 4.5f);
            setParam("ampChannel", 6.0f);
            setParam("tsBass", 5.0f);
            setParam("tsMid", 6.5f);
            setParam("tsTreble", 6.0f);
            setParam("paPresence", 5.5f);
            setParam("paResonance", 5.0f);
            setParam("paMaster", 5.5f);
            setParam("cabModel", 1.0f);  // 2x12 Closed
            setParam("cabMic", 1.0f);    // Off-Axis
            setParam("gateThreshold", -48.0f);
            // Distortion (Big Muff style)
            setParam("distEnabled", 1.0f);
            setParam("distModel", 1.0f);  // RAT (closest to Big Muff)
            setParam("distGain", 6.0f);
            setParam("distTone", 5.5f);
            setParam("distLevel", 5.0f);
            // Long delay
            setParam("delayEnabled", 1.0f);
            setParam("delayModel", 1.0f);  // Analog
            setParam("delayTime", 500.0f);
            setParam("delayFeedback", 0.4f);
            setParam("delayMix", 0.3f);
            setParam("delayMod", 0.15f);
            // Hall reverb
            setParam("reverbEnabled", 1.0f);
            setParam("reverbModel", 0.0f);  // Hall
            setParam("reverbSize", 0.7f);
            setParam("reverbDamping", 0.4f);
            setParam("reverbMix", 0.35f);
            // Chorus
            setParam("chorusEnabled", 1.0f);
            setParam("chorusRate", 0.7f);
            setParam("chorusDepth", 0.35f);
            setParam("chorusMix", 0.2f);
            break;

        case 15: // Kurt Cobain - Smells Like Teen Spirit (DS-1 + Mesa)
            setParam("ampModel", 6.0f);  // Mesa Rectifier
            setParam("ampGain", 7.5f);
            setParam("ampChannel", 6.5f);
            setParam("tsBass", 7.0f);
            setParam("tsMid", 5.0f);
            setParam("tsTreble", 6.0f);
            setParam("paPresence", 5.5f);
            setParam("paResonance", 6.0f);
            setParam("paMaster", 5.5f);
            setParam("cabModel", 2.0f);  // 4x12 V30
            setParam("cabMic", 0.0f);
            setParam("gateThreshold", -40.0f);
            // DS-1 Distortion
            setParam("distEnabled", 1.0f);
            setParam("distModel", 0.0f);  // DS-1
            setParam("distGain", 7.5f);
            setParam("distTone", 5.0f);
            setParam("distLevel", 6.0f);
            // Small chorus
            setParam("chorusEnabled", 1.0f);
            setParam("chorusRate", 1.2f);
            setParam("chorusDepth", 0.3f);
            setParam("chorusMix", 0.15f);
            break;

        case 16: // Stevie Ray Vaughan - Texas Flood (Fender + Tube Screamer)
            setParam("ampModel", 4.0f);  // Fender Twin
            setParam("ampGain", 6.5f);
            setParam("ampChannel", 7.0f);
            setParam("tsBass", 5.5f);
            setParam("tsMid", 7.0f);
            setParam("tsTreble", 6.0f);
            setParam("paPresence", 6.0f);
            setParam("paResonance", 5.0f);
            setParam("paMaster", 6.0f);
            setParam("cabModel", 0.0f);  // 1x12 Open Back
            setParam("cabMic", 0.0f);
            setParam("gateThreshold", -46.0f);
            // Tube Screamer (classic SRV)
            setParam("odEnabled", 1.0f);
            setParam("odModel", 0.0f);   // Tube Screamer
            setParam("odDrive", 6.5f);
            setParam("odTone", 6.0f);
            setParam("odLevel", 6.0f);
            // Spring reverb
            setParam("reverbEnabled", 1.0f);
            setParam("reverbModel", 3.0f);  // Spring
            setParam("reverbSize", 0.4f);
            setParam("reverbMix", 0.2f);
            break;

        case 17: // Angus Young - Highway to Hell (Marshall Crunch)
            setParam("ampModel", 5.0f);  // Marshall JCM
            setParam("ampGain", 7.0f);
            setParam("ampChannel", 7.0f);
            setParam("tsBass", 5.5f);
            setParam("tsMid", 6.5f);
            setParam("tsTreble", 7.0f);
            setParam("paPresence", 7.0f);
            setParam("paResonance", 5.0f);
            setParam("paMaster", 6.0f);
            setParam("cabModel", 3.0f);  // 4x12 Greenback
            setParam("cabMic", 0.0f);
            setParam("gateThreshold", -42.0f);
            // Room reverb (live sound)
            setParam("reverbEnabled", 1.0f);
            setParam("reverbModel", 1.0f);  // Room
            setParam("reverbSize", 0.3f);
            setParam("reverbMix", 0.15f);
            break;

        case 18: // Mark Knopfler - Sultans of Swing (Clean Stratocaster)
            setParam("ampModel", 0.0f);  // Clean
            setParam("ampGain", 3.0f);
            setParam("ampChannel", 6.5f);
            setParam("tsBass", 5.0f);
            setParam("tsMid", 6.0f);
            setParam("tsTreble", 7.0f);
            setParam("paPresence", 6.5f);
            setParam("paResonance", 4.0f);
            setParam("paMaster", 5.5f);
            setParam("cabModel", 0.0f);  // 1x12 Open Back
            setParam("cabMic", 1.0f);    // Off-Axis
            setParam("gateThreshold", -55.0f);
            // Compressor
            setParam("compEnabled", 1.0f);
            setParam("compModel", 1.0f);  // Optical
            setParam("compThreshold", -15.0f);
            setParam("compRatio", 3.5f);
            setParam("compMakeup", 3.0f);
            // Light reverb
            setParam("reverbEnabled", 1.0f);
            setParam("reverbModel", 1.0f);  // Room
            setParam("reverbSize", 0.25f);
            setParam("reverbMix", 0.15f);
            break;

        case 19: // Santana - Smooth Lead (Mesa + Sustain)
            setParam("ampModel", 6.0f);  // Mesa Rectifier
            setParam("ampGain", 6.5f);
            setParam("ampChannel", 6.0f);
            setParam("tsBass", 5.0f);
            setParam("tsMid", 8.0f);     // Lots of mids (Santana signature)
            setParam("tsTreble", 5.5f);
            setParam("paPresence", 5.0f);
            setParam("paResonance", 5.5f);
            setParam("paMaster", 5.5f);
            setParam("cabModel", 1.0f);  // 2x12 Closed
            setParam("cabMic", 0.0f);
            setParam("gateThreshold", -46.0f);
            // Overdrive (smooth)
            setParam("odEnabled", 1.0f);
            setParam("odModel", 2.0f);   // Klon
            setParam("odDrive", 5.0f);
            setParam("odTone", 5.0f);
            setParam("odLevel", 5.5f);
            // Compressor for sustain
            setParam("compEnabled", 1.0f);
            setParam("compModel", 0.0f);  // VCA
            setParam("compThreshold", -20.0f);
            setParam("compRatio", 4.0f);
            setParam("compMakeup", 3.0f);
            // Hall reverb
            setParam("reverbEnabled", 1.0f);
            setParam("reverbModel", 0.0f);  // Hall
            setParam("reverbSize", 0.5f);
            setParam("reverbMix", 0.25f);
            break;

        case 20: // Joe Satriani - Surfing With The Alien (Liquid Lead)
            setParam("ampModel", 5.0f);  // Marshall JCM
            setParam("ampGain", 8.0f);
            setParam("ampChannel", 6.5f);
            setParam("tsBass", 5.0f);
            setParam("tsMid", 7.5f);
            setParam("tsTreble", 6.0f);
            setParam("paPresence", 6.5f);
            setParam("paResonance", 5.0f);
            setParam("paMaster", 5.5f);
            setParam("cabModel", 2.0f);  // 4x12 V30
            setParam("cabMic", 0.0f);
            setParam("gateThreshold", -42.0f);
            // Distortion
            setParam("distEnabled", 1.0f);
            setParam("distModel", 0.0f);  // DS-1
            setParam("distGain", 7.0f);
            setParam("distTone", 6.0f);
            setParam("distLevel", 5.5f);
            // Wah-like phaser
            setParam("phaserEnabled", 1.0f);
            setParam("phaserRate", 0.5f);
            setParam("phaserDepth", 0.6f);
            setParam("phaserFeedback", 0.6f);
            setParam("phaserMix", 0.3f);
            // Delay
            setParam("delayEnabled", 1.0f);
            setParam("delayModel", 0.0f);  // Digital
            setParam("delayTime", 400.0f);
            setParam("delayFeedback", 0.3f);
            setParam("delayMix", 0.2f);
            // Reverb
            setParam("reverbEnabled", 1.0f);
            setParam("reverbModel", 0.0f);  // Hall
            setParam("reverbSize", 0.5f);
            setParam("reverbMix", 0.2f);
            break;

        case 21: // BB King - Lucille Blues (Warm, Gentle Overdrive)
            setParam("ampModel", 4.0f);  // Fender Twin
            setParam("ampGain", 4.0f);
            setParam("ampChannel", 6.0f);
            setParam("tsBass", 6.0f);
            setParam("tsMid", 7.0f);
            setParam("tsTreble", 5.0f);
            setParam("paPresence", 4.5f);
            setParam("paResonance", 5.0f);
            setParam("paMaster", 5.0f);
            setParam("cabModel", 0.0f);  // 1x12 Open Back
            setParam("cabMic", 1.0f);    // Off-Axis (warmer)
            setParam("gateThreshold", -52.0f);
            // Light overdrive
            setParam("odEnabled", 1.0f);
            setParam("odModel", 1.0f);   // Blues Driver
            setParam("odDrive", 3.5f);
            setParam("odTone", 4.5f);
            setParam("odLevel", 5.5f);
            // Compressor
            setParam("compEnabled", 1.0f);
            setParam("compModel", 1.0f);  // Optical
            setParam("compThreshold", -16.0f);
            setParam("compRatio", 3.0f);
            setParam("compMakeup", 2.5f);
            // Spring reverb
            setParam("reverbEnabled", 1.0f);
            setParam("reverbModel", 3.0f);  // Spring
            setParam("reverbSize", 0.35f);
            setParam("reverbMix", 0.2f);
            break;

        case 22: // Meshuggah - Djent (Extreme Tight High Gain)
            setParam("ampModel", 6.0f);  // Mesa Rectifier
            setParam("ampGain", 9.0f);
            setParam("ampChannel", 7.0f);
            setParam("tsBass", 7.0f);
            setParam("tsMid", 3.0f);     // Scooped
            setParam("tsTreble", 7.5f);
            setParam("paPresence", 8.0f);
            setParam("paResonance", 4.0f);
            setParam("paMaster", 5.5f);
            setParam("cabModel", 2.0f);  // 4x12 V30
            setParam("cabMic", 0.0f);
            setParam("gateThreshold", -34.0f);
            // High Gain - Djent
            setParam("hgEnabled", 1.0f);
            setParam("hgModel", 3.0f);   // Djent
            setParam("hgGain", 9.5f);
            setParam("hgTone", 6.0f);
            setParam("hgLevel", 5.0f);
            // Tight compressor
            setParam("compEnabled", 1.0f);
            setParam("compModel", 2.0f);  // FET
            setParam("compThreshold", -25.0f);
            setParam("compRatio", 8.0f);
            setParam("compAttack", 1.0f);
            setParam("compMakeup", 2.0f);
            // EQ boost
            setParam("geqEnabled", 1.0f);
            setParam("geqBand2", 3.0f);   // 125Hz boost
            setParam("geqBand6", -2.0f);  // 2kHz cut
            setParam("geqBand8", 4.0f);   // 8kHz boost
            break;

        case 23: // Soldano - Massive Lead
            setParam("ampModel", 7.0f);  // Soldano Lead
            setParam("ampGain", 8.5f);
            setParam("ampChannel", 6.5f);
            setParam("tsBass", 6.0f);
            setParam("tsMid", 6.0f);
            setParam("tsTreble", 6.5f);
            setParam("paPresence", 6.0f);
            setParam("paResonance", 5.5f);
            setParam("paMaster", 5.5f);
            setParam("cabModel", 2.0f);  // 4x12 V30
            setParam("cabMic", 0.0f);
            setParam("gateThreshold", -40.0f);
            // Optional OD boost
            setParam("odEnabled", 1.0f);
            setParam("odModel", 0.0f);   // Tube Screamer
            setParam("odDrive", 2.0f);
            setParam("odTone", 6.0f);
            setParam("odLevel", 7.0f);
            // Delay for lead
            setParam("delayEnabled", 1.0f);
            setParam("delayModel", 0.0f);  // Digital
            setParam("delayTime", 420.0f);
            setParam("delayFeedback", 0.35f);
            setParam("delayMix", 0.25f);
            // Reverb
            setParam("reverbEnabled", 1.0f);
            setParam("reverbModel", 0.0f);  // Hall
            setParam("reverbSize", 0.6f);
            setParam("reverbMix", 0.25f);
            break;

        // ===== NEW GUITARIST TONES =====

        case 24: // Paul Gilbert - Shred Machine (Marshall JCM + Ibanez)
            setParam("ampModel", 5.0f);  // Marshall JCM
            setParam("ampGain", 7.5f);
            setParam("ampChannel", 6.5f);
            setParam("tsBass", 5.0f);
            setParam("tsMid", 7.0f);     // Strong mids for cutting lead
            setParam("tsTreble", 6.5f);
            setParam("paPresence", 6.5f);
            setParam("paResonance", 5.0f);
            setParam("paMaster", 5.5f);
            setParam("cabModel", 2.0f);  // 4x12 V30
            setParam("cabMic", 0.0f);
            setParam("gateThreshold", -42.0f);
            // Distortion for high gain shred
            setParam("distEnabled", 1.0f);
            setParam("distModel", 2.0f);  // Metal Zone
            setParam("distGain", 7.0f);
            setParam("distTone", 6.0f);
            setParam("distLevel", 5.5f);
            // Compressor for sustain
            setParam("compEnabled", 1.0f);
            setParam("compModel", 0.0f);  // VCA
            setParam("compThreshold", -18.0f);
            setParam("compRatio", 4.0f);
            setParam("compMakeup", 2.0f);
            // Short delay for lead
            setParam("delayEnabled", 1.0f);
            setParam("delayModel", 0.0f);  // Digital
            setParam("delayTime", 350.0f);
            setParam("delayFeedback", 0.25f);
            setParam("delayMix", 0.15f);
            setParam("reverbEnabled", 1.0f);
            setParam("reverbModel", 1.0f);  // Room
            setParam("reverbSize", 0.3f);
            setParam("reverbMix", 0.15f);
            break;

        case 25: // Yngwie Malmsteen - Neoclassical (Marshall + OD boost)
            setParam("ampModel", 5.0f);  // Marshall JCM
            setParam("ampGain", 8.5f);
            setParam("ampChannel", 7.0f);
            setParam("tsBass", 5.0f);
            setParam("tsMid", 7.5f);     // Lots of mids for singing lead
            setParam("tsTreble", 6.0f);
            setParam("paPresence", 6.5f);
            setParam("paResonance", 5.5f);
            setParam("paMaster", 6.0f);
            setParam("cabModel", 3.0f);  // 4x12 Greenback
            setParam("cabMic", 0.0f);
            setParam("gateThreshold", -40.0f);
            // OD boost (Yngwie uses DOD 250 / Boss OD)
            setParam("odEnabled", 1.0f);
            setParam("odModel", 0.0f);   // Tube Screamer
            setParam("odDrive", 3.0f);   // Low drive, more volume
            setParam("odTone", 6.5f);
            setParam("odLevel", 7.5f);   // High level for boosting amp
            // Compressor for sustain on leads
            setParam("compEnabled", 1.0f);
            setParam("compModel", 0.0f);  // VCA
            setParam("compThreshold", -20.0f);
            setParam("compRatio", 3.5f);
            setParam("compMakeup", 2.0f);
            // Delay for lead lines
            setParam("delayEnabled", 1.0f);
            setParam("delayModel", 1.0f);  // Analog
            setParam("delayTime", 380.0f);
            setParam("delayFeedback", 0.3f);
            setParam("delayMix", 0.2f);
            setParam("reverbEnabled", 1.0f);
            setParam("reverbModel", 0.0f);  // Hall
            setParam("reverbSize", 0.5f);
            setParam("reverbMix", 0.2f);
            break;

        case 26: // Synyster Gates - A7X Lead (Schecter + High Gain)
            setParam("ampModel", 6.0f);  // Mesa Rectifier
            setParam("ampGain", 8.0f);
            setParam("ampChannel", 7.0f);
            setParam("tsBass", 6.5f);
            setParam("tsMid", 5.5f);
            setParam("tsTreble", 7.0f);
            setParam("paPresence", 7.0f);
            setParam("paResonance", 5.0f);
            setParam("paMaster", 5.5f);
            setParam("cabModel", 2.0f);  // 4x12 V30
            setParam("cabMic", 0.0f);
            setParam("gateThreshold", -38.0f);
            // High Gain - Rectifier style
            setParam("hgEnabled", 1.0f);
            setParam("hgModel", 0.0f);   // Rectifier
            setParam("hgGain", 7.5f);
            setParam("hgTone", 6.0f);
            setParam("hgLevel", 5.5f);
            // OD boost
            setParam("odEnabled", 1.0f);
            setParam("odModel", 0.0f);   // Tube Screamer
            setParam("odDrive", 2.0f);
            setParam("odTone", 6.0f);
            setParam("odLevel", 6.5f);
            // Delay for solos
            setParam("delayEnabled", 1.0f);
            setParam("delayModel", 0.0f);  // Digital
            setParam("delayTime", 400.0f);
            setParam("delayFeedback", 0.3f);
            setParam("delayMix", 0.2f);
            setParam("reverbEnabled", 1.0f);
            setParam("reverbModel", 0.0f);  // Hall
            setParam("reverbSize", 0.4f);
            setParam("reverbMix", 0.18f);
            break;

        case 27: // Marty Friedman - Exotic Lead (Unique tone)
            setParam("ampModel", 5.0f);  // Marshall JCM
            setParam("ampGain", 7.5f);
            setParam("ampChannel", 6.5f);
            setParam("tsBass", 5.5f);
            setParam("tsMid", 7.5f);     // High mids for singing lead
            setParam("tsTreble", 5.5f);  // Controlled treble
            setParam("paPresence", 5.5f);
            setParam("paResonance", 5.0f);
            setParam("paMaster", 5.5f);
            setParam("cabModel", 2.0f);  // 4x12 V30
            setParam("cabMic", 1.0f);    // Off-Axis for smoother
            setParam("gateThreshold", -42.0f);
            // Distortion - warm RAT
            setParam("distEnabled", 1.0f);
            setParam("distModel", 1.0f);  // RAT
            setParam("distGain", 6.5f);
            setParam("distTone", 5.0f);  // Warmer tone
            setParam("distLevel", 5.5f);
            // Chorus for exotic flavor
            setParam("chorusEnabled", 1.0f);
            setParam("chorusRate", 0.6f);
            setParam("chorusDepth", 0.25f);
            setParam("chorusMix", 0.15f);
            // Delay
            setParam("delayEnabled", 1.0f);
            setParam("delayModel", 1.0f);  // Analog
            setParam("delayTime", 420.0f);
            setParam("delayFeedback", 0.35f);
            setParam("delayMix", 0.22f);
            setParam("reverbEnabled", 1.0f);
            setParam("reverbModel", 0.0f);  // Hall
            setParam("reverbSize", 0.45f);
            setParam("reverbMix", 0.2f);
            break;

        case 28: // Vito Bratta - Melodic Rock (White Lion)
            setParam("ampModel", 5.0f);  // Marshall JCM
            setParam("ampGain", 7.0f);
            setParam("ampChannel", 6.5f);
            setParam("tsBass", 5.5f);
            setParam("tsMid", 6.5f);
            setParam("tsTreble", 7.0f);
            setParam("paPresence", 7.0f);
            setParam("paResonance", 5.0f);
            setParam("paMaster", 5.5f);
            setParam("cabModel", 3.0f);  // 4x12 Greenback
            setParam("cabMic", 0.0f);
            setParam("gateThreshold", -42.0f);
            // OD boost for lead
            setParam("odEnabled", 1.0f);
            setParam("odModel", 0.0f);   // Tube Screamer
            setParam("odDrive", 4.5f);
            setParam("odTone", 6.5f);
            setParam("odLevel", 6.0f);
            // Chorus for shimmer
            setParam("chorusEnabled", 1.0f);
            setParam("chorusRate", 0.8f);
            setParam("chorusDepth", 0.3f);
            setParam("chorusMix", 0.2f);
            // Delay for melodic runs
            setParam("delayEnabled", 1.0f);
            setParam("delayModel", 1.0f);  // Analog
            setParam("delayTime", 450.0f);
            setParam("delayFeedback", 0.35f);
            setParam("delayMix", 0.25f);
            setParam("reverbEnabled", 1.0f);
            setParam("reverbModel", 0.0f);  // Hall
            setParam("reverbSize", 0.55f);
            setParam("reverbMix", 0.22f);
            break;

        case 29: // Steve Vai - Liquid Lead (Carvin Legacy + FX)
            setParam("ampModel", 2.0f);  // High Gain
            setParam("ampGain", 7.5f);
            setParam("ampChannel", 6.5f);
            setParam("tsBass", 5.0f);
            setParam("tsMid", 7.0f);
            setParam("tsTreble", 6.5f);
            setParam("paPresence", 6.5f);
            setParam("paResonance", 5.0f);
            setParam("paMaster", 5.5f);
            setParam("cabModel", 2.0f);  // 4x12 V30
            setParam("cabMic", 0.0f);
            setParam("gateThreshold", -42.0f);
            // Distortion for sustain
            setParam("distEnabled", 1.0f);
            setParam("distModel", 0.0f);  // DS-1 (Vai's classic)
            setParam("distGain", 6.5f);
            setParam("distTone", 6.0f);
            setParam("distLevel", 5.5f);
            // Compressor for smooth sustain
            setParam("compEnabled", 1.0f);
            setParam("compModel", 0.0f);  // VCA
            setParam("compThreshold", -18.0f);
            setParam("compRatio", 3.5f);
            setParam("compMakeup", 2.5f);
            // Chorus for width
            setParam("chorusEnabled", 1.0f);
            setParam("chorusRate", 0.5f);
            setParam("chorusDepth", 0.3f);
            setParam("chorusMix", 0.2f);
            // Delay
            setParam("delayEnabled", 1.0f);
            setParam("delayModel", 0.0f);  // Digital
            setParam("delayTime", 440.0f);
            setParam("delayFeedback", 0.3f);
            setParam("delayMix", 0.2f);
            // Reverb
            setParam("reverbEnabled", 1.0f);
            setParam("reverbModel", 0.0f);  // Hall
            setParam("reverbSize", 0.5f);
            setParam("reverbMix", 0.22f);
            break;

        case 30: // Eddie Van Halen - Hot Rod Marshall (Variac Brown Sound)
            setParam("ampModel", 5.0f);  // Marshall JCM
            setParam("ampGain", 9.0f);
            setParam("ampChannel", 7.0f);
            setParam("tsBass", 5.5f);
            setParam("tsMid", 7.0f);
            setParam("tsTreble", 6.0f);
            setParam("paPresence", 7.0f);
            setParam("paResonance", 5.5f);
            setParam("paMaster", 6.5f);
            setParam("cabModel", 3.0f);  // 4x12 Greenback
            setParam("cabMic", 0.0f);
            setParam("gateThreshold", -40.0f);
            // OD boost for sizzle
            setParam("odEnabled", 1.0f);
            setParam("odModel", 0.0f);   // Tube Screamer
            setParam("odDrive", 3.0f);
            setParam("odTone", 6.0f);
            setParam("odLevel", 6.5f);
            // Phaser (MXR Phase 90 - EVH signature)
            setParam("phaserEnabled", 1.0f);
            setParam("phaserRate", 0.7f);
            setParam("phaserDepth", 0.4f);
            setParam("phaserFeedback", 0.5f);
            setParam("phaserMix", 0.25f);
            // Short delay
            setParam("delayEnabled", 1.0f);
            setParam("delayModel", 1.0f);  // Analog
            setParam("delayTime", 320.0f);
            setParam("delayFeedback", 0.2f);
            setParam("delayMix", 0.15f);
            setParam("reverbEnabled", 1.0f);
            setParam("reverbModel", 1.0f);  // Room
            setParam("reverbSize", 0.35f);
            setParam("reverbMix", 0.18f);
            break;

        case 31: // Brian May - Queen Tone (AC30 + Treble Booster)
            setParam("ampModel", 1.0f);  // Crunch (Vox AC30-like)
            setParam("ampGain", 6.5f);
            setParam("ampChannel", 6.5f);
            setParam("tsBass", 5.0f);
            setParam("tsMid", 6.0f);
            setParam("tsTreble", 7.5f);  // Bright treble
            setParam("paPresence", 7.0f);
            setParam("paResonance", 4.5f);
            setParam("paMaster", 5.5f);
            setParam("cabModel", 0.0f);  // 1x12 Open Back (Vox-style)
            setParam("cabMic", 0.0f);
            setParam("gateThreshold", -44.0f);
            // Treble booster OD (Brian May signature)
            setParam("odEnabled", 1.0f);
            setParam("odModel", 2.0f);   // Klon (transparent boost)
            setParam("odDrive", 5.0f);
            setParam("odTone", 7.5f);    // High tone = treble boost
            setParam("odLevel", 6.0f);
            // Delay (multi-track layering effect)
            setParam("delayEnabled", 1.0f);
            setParam("delayModel", 1.0f);  // Analog
            setParam("delayTime", 300.0f);
            setParam("delayFeedback", 0.3f);
            setParam("delayMix", 0.2f);
            // Chorus for multi-guitar layer effect
            setParam("chorusEnabled", 1.0f);
            setParam("chorusRate", 0.4f);
            setParam("chorusDepth", 0.35f);
            setParam("chorusMix", 0.25f);
            // Room reverb
            setParam("reverbEnabled", 1.0f);
            setParam("reverbModel", 1.0f);  // Room
            setParam("reverbSize", 0.4f);
            setParam("reverbMix", 0.2f);
            break;

        case 32: // Grand Organ Synth
            setParam("stringEnabled", 1.0f);
            setParam("stringAttack", 120.0f);
            setParam("stringOctave", 0.85f); // High ensemble depth
            setParam("stringBrightness", 4500.0f);
            setParam("stringResonance", 0.4f);
            setParam("stringMix", 0.9f);
            setParam("ampModel", 0.0f);  // Clean
            setParam("reverbEnabled", 1.0f);
            setParam("reverbModel", 4.0f);  // Cathedral
            setParam("reverbSize", 0.8f);
            setParam("reverbMix", 0.4f);
            break;

        case 33: // Funky Auto Wah (Kental)
            setParam("autoWahEnabled", 1.0f);
            setParam("autoWahSens", 0.7f);
            setParam("autoWahAttack", 15.0f);
            setParam("autoWahRelease", 100.0f);
            setParam("autoWahRange", 500.0f);
            setParam("ampModel", 1.0f);  // Crunch
            setParam("ampGain", 4.0f);
            setParam("reverbEnabled", 1.0f);
            setParam("reverbModel", 1.0f);  // Room
            setParam("reverbSize", 0.3f);
            setParam("reverbMix", 0.15f);
            break;

        case 34: // Blues Harmonica
            setParam("stringEnabled", 1.0f);
            setParam("stringAttack", 40.0f); // Fast attack for harmonica response
            setParam("stringOctave", 0.65f); // Enable vibrato & pitch scoop
            setParam("stringBrightness", 5000.0f);
            setParam("stringResonance", 0.35f);
            setParam("stringMix", 0.85f);
            // Drive for that bluesy harmonica grit
            setParam("odEnabled", 1.0f);
            setParam("odModel", 0.0f); // Tube Screamer
            setParam("odDrive", 6.0f);
            setParam("odTone", 4.0f);
            setParam("ampModel", 1.0f); // Crunch
            setParam("reverbEnabled", 1.0f);
            setParam("reverbModel", 1.0f); // Room
            setParam("reverbMix", 0.2f);
            break;

        case 35: // Reed Organ / Accordion
            setParam("stringEnabled", 1.0f);
            setParam("stringAttack", 180.0f); // Slower swell
            setParam("stringOctave", 0.45f); // Moderate ensemble
            setParam("stringBrightness", 3500.0f);
            setParam("stringResonance", 0.5f);
            setParam("stringMix", 0.75f);
            setParam("ampModel", 0.0f); // Clean
            setParam("chorusEnabled", 1.0f); // Extra thickness
            setParam("chorusMix", 0.3f);
            setParam("reverbEnabled", 1.0f);
            setParam("reverbModel", 4.0f); // Cathedral
            setParam("reverbSize", 0.7f);
            setParam("reverbMix", 0.3f);
            break;
        }
    }

    void savePreset()
    {
        fileChooser = std::make_unique<juce::FileChooser>("Save Preset",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.gfxpreset");

        fileChooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file != juce::File{})
                {
                    auto state = valueTreeState.copyState();
                    std::unique_ptr<juce::XmlElement> xml(state.createXml());
                    xml->writeTo(file);
                }
            });
    }

    void loadPreset()
    {
        fileChooser = std::make_unique<juce::FileChooser>("Load Preset",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.gfxpreset");

        fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file != juce::File{})
                {
                    auto xml = juce::parseXML(file);
                    if (xml != nullptr)
                    {
                        auto state = juce::ValueTree::fromXml(*xml);
                        if (state.isValid())
                            valueTreeState.replaceState(state);
                    }
                }
            });
    }

public:
    std::function<void()> onLicenseClicked;

private:

    juce::AudioProcessorValueTreeState& valueTreeState;
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::ComboBox presetSelector;
    juce::TextButton saveBtn, loadBtn, prevBtn, nextBtn, licenseBtn;
    juce::Label titleLabel, developerLabel, presetLabel;
};
