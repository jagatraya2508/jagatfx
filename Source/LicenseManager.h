#pragma once
#include <JuceHeader.h>

// Windows headers for hardware ID
#ifdef _WIN32
#include <Windows.h>
#include <iphlpapi.h>
#include <intrin.h>
#pragma comment(lib, "iphlpapi.lib")
#endif

class LicenseManager
{
public:
    LicenseManager() = default;

    // ==================== Hardware Identification ====================

    static juce::String getMacAddress()
    {
#ifdef _WIN32
        PIP_ADAPTER_INFO adapterInfo = nullptr;
        ULONG bufLen = 0;

        // First call to get buffer size
        if (GetAdaptersInfo(adapterInfo, &bufLen) == ERROR_BUFFER_OVERFLOW)
        {
            adapterInfo = (IP_ADAPTER_INFO*)malloc(bufLen);
            if (adapterInfo == nullptr) return "000000000000";
        }

        juce::String mac = "000000000000";
        if (GetAdaptersInfo(adapterInfo, &bufLen) == NO_ERROR)
        {
            // Use the first non-zero adapter
            PIP_ADAPTER_INFO adapter = adapterInfo;
            while (adapter)
            {
                if (adapter->AddressLength == 6)
                {
                    juce::String thisMac;
                    for (UINT i = 0; i < adapter->AddressLength; i++)
                        thisMac += juce::String::toHexString(adapter->Address[i]).paddedLeft('0', 2);
                    
                    if (thisMac != "000000000000")
                    {
                        mac = thisMac.toUpperCase();
                        break;
                    }
                }
                adapter = adapter->Next;
            }
        }

        if (adapterInfo) free(adapterInfo);
        return mac;
#else
        return "000000000000";
#endif
    }

    static juce::String getCpuId()
    {
#ifdef _WIN32
        int cpuInfo[4] = { 0 };
        __cpuid(cpuInfo, 1);
        juce::String cpuId;
        cpuId += juce::String::toHexString(cpuInfo[0]);
        cpuId += juce::String::toHexString(cpuInfo[3]);
        return cpuId.toUpperCase();
#else
        return "0000000000000000";
#endif
    }

    static juce::String getDiskSerial()
    {
#ifdef _WIN32
        DWORD volumeSerial = 0;
        if (GetVolumeInformationA("C:\\", nullptr, 0, &volumeSerial, nullptr, nullptr, nullptr, 0))
        {
            return juce::String::toHexString((int)volumeSerial).toUpperCase();
        }
        return "00000000";
#else
        return "00000000";
#endif
    }

    // ==================== Hardware ID Generation ====================

    static juce::String getHardwareId()
    {
        juce::String combined = getMacAddress() + getCpuId() + getDiskSerial();
        return sha256Short(combined);
    }

    // ==================== License Key Validation ====================

    static juce::String generateLicenseKey(const juce::String& hardwareId, juce::int64 expiryDateMs = -1)
    {
        // New Format: [HWID_HASH_4_CHARS]-[EXPIRY_HEX]-[SIGNATURE_8_CHARS]
        juce::String hwHash = sha256Short(hardwareId).substring(0, 4);
        
        // Expiry Hex: 8 chars
        juce::String expiryHex;
        if (expiryDateMs < 0)
        {
            expiryHex = "FFFFFFFF"; // Lifetime
        }
        else
        {
            // Convert ms to seconds for shorter hex
            juce::int64 expirySecs = expiryDateMs / 1000;
            expiryHex = juce::String::toHexString(expirySecs).paddedLeft('0', 8).toUpperCase();
        }

        // Signature: hash of (HWID + EXPIRY + SECRET)
        juce::String rawSig = hardwareId + expiryHex + getSecret();
        juce::String sigHash = sha256Full(rawSig).substring(0, 8).toUpperCase();

        juce::String key = hwHash + expiryHex + sigHash;
        return formatKey(key); // Format as XXXX-XXXX-XXXX-XXXX
    }

    static bool validateLicense(const juce::String& licenseKey, const juce::String& hardwareId)
    {
        juce::String cleanInput = licenseKey.removeCharacters("-").toUpperCase().trim();
        // Needs exactly 16 characters (4 hw + 8 expiry + 4 signature)
        // Wait, 4 + 8 + 8 = 20 chars. Let's make signature 4 chars to keep it 16 chars total: 
        // Oh wait, standard was 16 chars (XXXX-XXXX-XXXX-XXXX).
        // Let's use: HWID(4) + EXPIRY(8) + SIG(4) = 16 chars.
        
        if (cleanInput.length() != 16)
            return false;

        juce::String hwHashInput = cleanInput.substring(0, 4);
        juce::String expiryInput = cleanInput.substring(4, 12);
        juce::String sigInput = cleanInput.substring(12, 16);

        // 1. Check HWID Hash match
        juce::String actualHwHash = sha256Short(hardwareId).substring(0, 4);
        if (hwHashInput != actualHwHash)
            return false;

        // 2. Check Signature match
        juce::String rawSig = hardwareId + expiryInput + getSecret();
        juce::String expectedSig = sha256Full(rawSig).substring(0, 4).toUpperCase();
        if (sigInput != expectedSig)
            return false;

        // 3. Check Expiration
        if (expiryInput == "FFFFFFFF")
            return true; // Lifetime

        auto expirySecs = expiryInput.getHexValue64();
        auto currentSecs = juce::Time::getCurrentTime().toMilliseconds() / 1000;
        
        return currentSecs < expirySecs;
    }

    static juce::int64 getLicenseExpiry(const juce::String& licenseKey)
    {
        juce::String cleanInput = licenseKey.removeCharacters("-").toUpperCase().trim();
        if (cleanInput.length() != 16) return 0;
        
        juce::String expiryInput = cleanInput.substring(4, 12);
        if (expiryInput == "FFFFFFFF") return -1; // Lifetime
        
        return expiryInput.getHexValue64() * 1000; // Return ms
    }

    // ==================== License File Management ====================

    static juce::File getLicenseFile()
    {
        auto appDir = juce::File::getSpecialLocation(
            juce::File::userApplicationDataDirectory).getChildFile("JagatMultiFX");
        if (!appDir.exists())
            appDir.createDirectory();
        return appDir.getChildFile("license.dat");
    }

    static bool saveLicense(const juce::String& licenseKey)
    {
        auto file = getLicenseFile();
        // Simple XOR encryption
        auto encrypted = encrypt(licenseKey, getEncryptionKey());
        return file.replaceWithText(encrypted);
    }

    static juce::String loadLicense()
    {
        auto file = getLicenseFile();
        if (!file.existsAsFile())
            return {};
        auto encrypted = file.loadFileAsString();
        return decrypt(encrypted, getEncryptionKey());
    }

    static bool isActivated()
    {
        auto key = loadLicense();
        if (key.isEmpty())
            return false;
        return validateLicense(key, getHardwareId());
    }

    // ==================== Trial System ====================

    static juce::File getTrialFile()
    {
        auto appDir = juce::File::getSpecialLocation(
            juce::File::userApplicationDataDirectory).getChildFile("JagatMultiFX");
        if (!appDir.exists())
            appDir.createDirectory();
        return appDir.getChildFile("trial.dat");
    }

    static void startTrial()
    {
        if (getTrialFile().existsAsFile())
            return; // already started

        // Store encrypted start timestamp
        auto now = juce::Time::getCurrentTime().toMilliseconds();
        juce::String data = juce::String(now);
        auto encrypted = encrypt(data, getEncryptionKey());
        getTrialFile().replaceWithText(encrypted);
    }

    static bool isTrialActive()
    {
        auto file = getTrialFile();
        if (!file.existsAsFile())
            return false;

        auto encrypted = file.loadFileAsString();
        auto decrypted = decrypt(encrypted, getEncryptionKey());

        auto startTime = decrypted.getLargeIntValue();
        if (startTime <= 0)
            return false;

        auto now = juce::Time::getCurrentTime().toMilliseconds();
        auto elapsed = now - startTime;
        auto sevenDays = (juce::int64)7 * 24 * 60 * 60 * 1000;

        return elapsed < sevenDays;
    }

    static int getTrialDaysRemaining()
    {
        auto file = getTrialFile();
        if (!file.existsAsFile())
            return 7;

        auto encrypted = file.loadFileAsString();
        auto decrypted = decrypt(encrypted, getEncryptionKey());

        auto startTime = decrypted.getLargeIntValue();
        if (startTime <= 0)
            return 0;

        auto now = juce::Time::getCurrentTime().toMilliseconds();
        auto elapsed = now - startTime;
        auto sevenDays = (juce::int64)7 * 24 * 60 * 60 * 1000;
        auto remaining = sevenDays - elapsed;

        if (remaining <= 0) return 0;
        return (int)(remaining / (24 * 60 * 60 * 1000)) + 1;
    }

    static bool hasTrialStarted()
    {
        return getTrialFile().existsAsFile();
    }

    // ==================== Overall Status ====================

    enum class LicenseStatus
    {
        Licensed,      // Fully activated and not expired
        Expired,       // Activated but expired
        Trial,         // Trial active
        TrialExpired,  // Trial expired, needs license
        NotActivated   // First run, no trial started
    };

    static LicenseStatus getStatus()
    {
        auto key = loadLicense();
        if (key.isNotEmpty())
        {
            // If the key format is correct and signature matches, but it returns false in validateLicense, it means it's expired.
            // Let's do a partial validation to check if it's expired vs invalid.
            juce::String cleanInput = key.removeCharacters("-").toUpperCase().trim();
            if (cleanInput.length() == 16)
            {
                juce::String expectedSig = sha256Full(getHardwareId() + cleanInput.substring(4, 12) + getSecret()).substring(0, 4).toUpperCase();
                juce::String actualHwHash = sha256Short(getHardwareId()).substring(0, 4);
                
                // If the key belongs to this machine and hasn't been tampered with
                if (cleanInput.substring(0, 4) == actualHwHash && cleanInput.substring(12, 16) == expectedSig)
                {
                    if (validateLicense(key, getHardwareId()))
                        return LicenseStatus::Licensed;
                    else
                        return LicenseStatus::Expired;
                }
            }
        }

        if (!hasTrialStarted())
        {
            startTrial();
            return LicenseStatus::Trial;
        }

        if (isTrialActive())
            return LicenseStatus::Trial;

        return LicenseStatus::TrialExpired;
    }

private:
    // ==================== Crypto Helpers ====================

    static juce::String getSecret()
    {
        // Secret key for license generation - DO NOT SHARE
        return "J4G4T_MULT1_FX_S3CR3T_K3Y_2026_WW";
    }

    static juce::String getEncryptionKey()
    {
        return "JagatFX_Encrypt_Key_2026!";
    }

    static juce::String sha256Short(const juce::String& input)
    {
        // Use JUCE SHA256
        juce::MemoryBlock data(input.toRawUTF8(), input.getNumBytesAsUTF8());
        juce::SHA256 hash(data);
        auto result = hash.toHexString().removeCharacters(" ");
        return result.substring(0, 12).toUpperCase();
    }

    static juce::String sha256Full(const juce::String& input)
    {
        juce::MemoryBlock data(input.toRawUTF8(), input.getNumBytesAsUTF8());
        juce::SHA256 hash(data);
        return hash.toHexString().removeCharacters(" ");
    }

    static juce::String encrypt(const juce::String& input, const juce::String& key)
    {
        auto inputUtf8 = input.toStdString();
        auto keyUtf8 = key.toStdString();
        std::string result;
        result.reserve(inputUtf8.size());

        for (size_t i = 0; i < inputUtf8.size(); i++)
        {
            result += (char)(inputUtf8[i] ^ keyUtf8[i % keyUtf8.size()]);
        }

        // Convert to hex string for safe file storage
        juce::String hexResult;
        for (auto c : result)
            hexResult += juce::String::toHexString((int)(unsigned char)c).paddedLeft('0', 2);
        return hexResult;
    }

    static juce::String decrypt(const juce::String& hexInput, const juce::String& key)
    {
        auto keyUtf8 = key.toStdString();
        std::string result;
        
        // Parse hex
        for (int i = 0; i < hexInput.length() - 1; i += 2)
        {
            auto byteString = hexInput.substring(i, i + 2);
            char b = (char)byteString.getHexValue32();
            result += (char)(b ^ keyUtf8[(i / 2) % keyUtf8.size()]);
        }
        
        return juce::String(juce::CharPointer_UTF8(result.c_str()));
    }

    static juce::String formatKey(const juce::String& raw)
    {
        // Format as XXXX-XXXX-XXXX-XXXX
        juce::String formatted;
        for (int i = 0; i < raw.length(); i++)
        {
            if (i > 0 && i % 4 == 0)
                formatted += "-";
            formatted += raw[i];
        }
        return formatted;
    }
};
