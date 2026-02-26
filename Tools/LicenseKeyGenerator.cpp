// JAGAT MULTI FX - License Key Generator
// Tool untuk developer/admin untuk generate license key dari Hardware ID
//
// Usage: LicenseKeyGenerator.exe <HARDWARE_ID>
// Output: License Key dalam format XXXX-XXXX-XXXX-XXXX
//
// Compile standalone:
//   cl /EHsc /std:c++17 LicenseKeyGenerator.cpp /Fe:LicenseKeyGenerator.exe

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <vector>
#include <cstdint>
#include <cctype>
#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <vector>
#include <cstdint>
#include <cctype>

// ==================== Minimal SHA-256 Implementation ====================
// (standalone, no JUCE dependency)

namespace {

static const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

inline uint32_t rotr(uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }
inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
inline uint32_t sig0(uint32_t x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); }
inline uint32_t sig1(uint32_t x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); }
inline uint32_t gam0(uint32_t x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); }
inline uint32_t gam1(uint32_t x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); }

std::string sha256(const std::string& input)
{
    uint32_t h[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };

    size_t len = input.size();
    size_t bitLen = len * 8;

    // Pad message
    std::vector<uint8_t> msg(input.begin(), input.end());
    msg.push_back(0x80);
    while ((msg.size() % 64) != 56)
        msg.push_back(0x00);
    for (int i = 7; i >= 0; i--)
        msg.push_back((uint8_t)(bitLen >> (i * 8)));

    // Process blocks
    for (size_t offset = 0; offset < msg.size(); offset += 64)
    {
        uint32_t w[64];
        for (int i = 0; i < 16; i++)
            w[i] = ((uint32_t)msg[offset + i * 4] << 24) |
                    ((uint32_t)msg[offset + i * 4 + 1] << 16) |
                    ((uint32_t)msg[offset + i * 4 + 2] << 8) |
                    ((uint32_t)msg[offset + i * 4 + 3]);
        for (int i = 16; i < 64; i++)
            w[i] = gam1(w[i - 2]) + w[i - 7] + gam0(w[i - 15]) + w[i - 16];

        uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
        uint32_t e = h[4], f = h[5], g = h[6], hh = h[7];

        for (int i = 0; i < 64; i++)
        {
            uint32_t t1 = hh + sig1(e) + ch(e, f, g) + K[i] + w[i];
            uint32_t t2 = sig0(a) + maj(a, b, c);
            hh = g; g = f; f = e; e = d + t1;
            d = c; c = b; b = a; a = t1 + t2;
        }

        h[0] += a; h[1] += b; h[2] += c; h[3] += d;
        h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
    }

    std::ostringstream result;
    for (int i = 0; i < 8; i++)
        result << std::hex << std::setfill('0') << std::setw(8) << h[i];
    return result.str();
}

} // anonymous namespace

// ==================== License Generation ====================

static const std::string SECRET_KEY = "J4G4T_MULT1_FX_S3CR3T_K3Y_2026_WW";

std::string generateLicenseKey(const std::string& hardwareId, int64_t expirySecs = -1)
{
    // New Format: [HWID_HASH_4_CHARS]-[EXPIRY_HEX]-[SIGNATURE_4_CHARS]
    
    // 1. HW Hash (4 chars)
    std::string hwHash = sha256(hardwareId).substr(0, 4);
    for (auto& c : hwHash) c = toupper(c);

    // 2. Expiry Hex (8 chars)
    std::string expiryHex;
    if (expirySecs < 0)
    {
        expiryHex = "FFFFFFFF"; // Lifetime
    }
    else
    {
        std::stringstream ss;
        ss << std::hex << std::uppercase << std::setfill('0') << std::setw(8) << expirySecs;
        expiryHex = ss.str();
    }

    // 3. Signature Hash (4 chars)
    std::string rawSig = hardwareId + expiryHex + SECRET_KEY;
    std::string sigHash = sha256(rawSig).substr(0, 4);
    for (auto& c : sigHash) c = toupper(c);

    std::string key = hwHash + expiryHex + sigHash;

    // 4. Format as XXXX-XXXX-XXXX-XXXX
    std::string formatted;
    for (int i = 0; i < (int)key.size(); i++)
    {
        if (i > 0 && i % 4 == 0)
            formatted += '-';
        formatted += key[i];
    }
    return formatted;
}

int main(int argc, char* argv[])
{
    std::cout << "========================================" << std::endl;
    std::cout << "  JAGAT MULTI FX - License Key Generator" << std::endl;
    std::cout << "  By Wisnu Wardana" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    std::string hwId;

    if (argc >= 2)
    {
        // Mode CLI: ambil dari argumen
        hwId = argv[1];
    }
    else
    {
        // Mode interaktif: minta input dari user
        std::cout << "Masukkan Hardware ID (dari Activation Dialog):" << std::endl;
        std::cout << "> ";
        std::getline(std::cin, hwId);
        std::cout << std::endl;
    }

    // Hapus spasi di awal/akhir
    while (!hwId.empty() && hwId.front() == ' ') hwId.erase(hwId.begin());
    while (!hwId.empty() && hwId.back() == ' ') hwId.pop_back();

    if (hwId.empty())
    {
        std::cout << "Error: Hardware ID tidak boleh kosong!" << std::endl;
        std::cout << std::endl;
        std::cout << "Tekan Enter untuk keluar...";
        std::cin.get();
        return 1;
    }

    // Convert to uppercase
    for (auto& c : hwId)
        c = toupper(c);

    // Get Duration Choice
    int choice = 5; // Default Lifetime
    if (argc < 2)
    {
        std::cout << "Pilih durasi lisensi:" << std::endl;
        std::cout << "0. 1 Hari (Untuk Testing)" << std::endl;
        std::cout << "1. 1 Bulan (30 hari)" << std::endl;
        std::cout << "2. 3 Bulan (90 hari)" << std::endl;
        std::cout << "3. 6 Bulan (180 hari)" << std::endl;
        std::cout << "4. 1 Tahun (365 hari)" << std::endl;
        std::cout << "5. Lifetime (Selamanya)" << std::endl;
        std::cout << "> ";
        
        std::string choiceStr;
        std::getline(std::cin, choiceStr);
        try {
            choice = std::stoi(choiceStr);
        } catch (...) {
            choice = 5;
        }
    }

    // Calculate Expiry Timestamp
    int64_t expirySecs = -1;
    auto now = std::chrono::system_clock::now();
    auto nowSecs = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    
    int64_t daySecs = 24 * 60 * 60;
    
    std::string durationStr;

    switch (choice)
    {
        case 0: expirySecs = nowSecs + (1 * daySecs); durationStr = "1 Hari (Testing)"; break;
        case 1: expirySecs = nowSecs + (30 * daySecs); durationStr = "1 Bulan (30 hari)"; break;
        case 2: expirySecs = nowSecs + (90 * daySecs); durationStr = "3 Bulan (90 hari)"; break;
        case 3: expirySecs = nowSecs + (180 * daySecs); durationStr = "6 Bulan (180 hari)"; break;
        case 4: expirySecs = nowSecs + (365 * daySecs); durationStr = "1 Tahun (365 hari)"; break;
        default: expirySecs = -1; durationStr = "Lifetime (Selamanya)"; break;
    }

    std::cout << "\nHardware ID : " << hwId << std::endl;
    std::cout << "Durasi      : " << durationStr << std::endl;
    std::string licenseKey = generateLicenseKey(hwId, expirySecs);
    std::cout << "License Key : " << licenseKey << std::endl;
    std::cout << std::endl;
    std::cout << "Kirim License Key di atas ke user." << std::endl;
    std::cout << std::endl;

    // Jika mode interaktif, tunggu Enter sebelum tutup
    if (argc < 2)
    {
        std::cout << "Tekan Enter untuk keluar...";
        std::cin.get();
    }

    return 0;
}
