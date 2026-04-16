#include "displays.h"

#include <ctype.h>
#include <string.h>

Adafruit_7segment ledDisplay = Adafruit_7segment();

namespace {
const uint16_t segmentPatterns[16] = {
    0b0111111000,
    0b0011000000,
    0b0110110100,
    0b0111100100,
    0b0011001100,
    0b0101101100,
    0b0101111100,
    0b0111000000,
    0b0111111100,
    0b0111101100,
    0b0111011100,
    0b0000111100,
    0b0100110000,
    0b0001110100,
    0b0100111100,
    0b0100011100
};

constexpr uint16_t SEG_A = 0b0100000000;
constexpr uint16_t SEG_B = 0b0010000000;
constexpr uint16_t SEG_C = 0b0001000000;
constexpr uint16_t SEG_D = 0b0000100000;
constexpr uint16_t SEG_E = 0b0000010000;
constexpr uint16_t SEG_F = 0b0000001000;
constexpr uint16_t SEG_G = 0b0000000100;
constexpr uint16_t SEG_DP = 0b0000000001;

constexpr uint16_t LETTER_G = SEG_A | SEG_C | SEG_D | SEG_E | SEG_F;
constexpr uint16_t LETTER_O = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F;
constexpr uint16_t LETTER_D = SEG_B | SEG_C | SEG_D | SEG_E | SEG_G;
constexpr uint16_t WIFI_ICON = SEG_A | SEG_B | SEG_F | SEG_G | SEG_DP;

bool displayReady = false;

uint8_t probeI2CAddress(uint8_t address) {
    Wire.beginTransmission(address);
    return Wire.endTransmission();
}

void writeRawDigits(const uint16_t* patterns) {
    if (!displayReady || patterns == nullptr) {
        return;
    }

    Wire.beginTransmission(HT16K33_ADDRESS);
    Wire.write(0x00);
    for (uint8_t index = 0; index < 6; index++) {
        Wire.write(patterns[index] & 0xFF);
        Wire.write((patterns[index] >> 8) & 0xFF);
    }
    Wire.endTransmission();
}

uint16_t encodeChar(char value) {
    switch (toupper(static_cast<unsigned char>(value))) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return segmentPatterns[value - '0'];
        case 'A':
            return SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G;
        case 'B':
            return SEG_C | SEG_D | SEG_E | SEG_F | SEG_G;
        case 'C':
            return SEG_A | SEG_D | SEG_E | SEG_F;
        case 'D':
            return SEG_B | SEG_C | SEG_D | SEG_E | SEG_G;
        case 'E':
            return SEG_A | SEG_D | SEG_E | SEG_F | SEG_G;
        case 'F':
            return SEG_A | SEG_E | SEG_F | SEG_G;
        case 'G':
            return LETTER_G;
        case 'H':
            return SEG_B | SEG_C | SEG_E | SEG_F | SEG_G;
        case 'I':
            return SEG_B | SEG_C;
        case 'J':
            return SEG_B | SEG_C | SEG_D | SEG_E;
        case 'L':
            return SEG_D | SEG_E | SEG_F;
        case 'N':
            return SEG_A | SEG_B | SEG_C | SEG_E | SEG_F;
        case 'O':
            return LETTER_O;
        case 'P':
            return SEG_A | SEG_B | SEG_E | SEG_F | SEG_G;
        case 'R':
            return SEG_E | SEG_G;
        case 'S':
            return segmentPatterns[5];
        case 'T':
            return SEG_D | SEG_E | SEG_F | SEG_G;
        case 'U':
            return SEG_B | SEG_C | SEG_D | SEG_E | SEG_F;
        case 'W':
            return SEG_B | SEG_C | SEG_D | SEG_E | SEG_F;
        case 'Y':
            return SEG_B | SEG_C | SEG_D | SEG_F | SEG_G;
        case '-':
            return SEG_G;
        case '_':
            return SEG_D;
        default:
            return 0;
    }
}
}

void initDisplay() {
    const uint8_t displayProbe = probeI2CAddress(HT16K33_ADDRESS);
    const uint8_t matrixProbe = probeI2CAddress(CAPTAIN_MATRIX_I2C_ADDRESS);

    Serial.printf("I2C probe display 0x%02X: %s (err=%u)\n",
                  HT16K33_ADDRESS,
                  displayProbe == 0 ? "OK" : "MISSING",
                  displayProbe);
    Serial.printf("I2C probe matrix 0x%02X: %s (err=%u)\n",
                  CAPTAIN_MATRIX_I2C_ADDRESS,
                  matrixProbe == 0 ? "OK" : "MISSING",
                  matrixProbe);

    if (!ledDisplay.begin(HT16K33_ADDRESS)) {
        Serial.println("HT16K33 not found at 0x70");
        displayReady = false;
        return;
    }

    displayReady = true;
    ledDisplay.setBrightness(10);
    ledDisplay.clear();
    ledDisplay.writeDisplay();
}

void displayStartupTest() {
    if (!displayReady) {
        return;
    }

    for (uint8_t flash = 0; flash < 3; flash++) {
        Wire.beginTransmission(HT16K33_ADDRESS);
        Wire.write(0x00);
        for (uint8_t i = 0; i < 6; i++) {
            Wire.write(0b11111100);
            Wire.write(0b00000001);
        }
        Wire.endTransmission();
        delay(150);

        Wire.beginTransmission(HT16K33_ADDRESS);
        Wire.write(0x00);
        for (uint8_t i = 0; i < 12; i++) {
            Wire.write(0x00);
        }
        Wire.endTransmission();
        delay(150);
    }

    for (int8_t num = 9; num >= 0; num--) {
        const uint16_t pattern = segmentPatterns[num];
        uint16_t raw[6] = {pattern, pattern, pattern, pattern, pattern, pattern};
        writeRawDigits(raw);
        delay(80);
    }

    for (uint8_t pos = 0; pos < 6; pos++) {
        uint16_t raw[6] = {};
        raw[pos] = 0b1111111101;
        writeRawDigits(raw);
        delay(100);
    }

    clearDisplay();
}

void displayGoodMessage(uint16_t durationMs) {
    if (!displayReady) {
        return;
    }

    uint16_t raw[6] = {};
    raw[1] = LETTER_G;
    raw[2] = LETTER_O;
    raw[3] = LETTER_O;
    raw[4] = LETTER_D;
    writeRawDigits(raw);
    delay(durationMs);
}

void displayStatusMessage(const char* text, bool wifiConnected) {
    if (!displayReady) {
        return;
    }

    uint16_t raw[6] = {};
    uint8_t startIndex = 0;
    uint8_t availableDigits = 6;

    if (wifiConnected) {
        raw[0] = WIFI_ICON;
        startIndex = 1;
        availableDigits = 5;
    }

    char compact[7] = {};
    size_t compactLength = 0;
    if (text != nullptr) {
        for (size_t index = 0; text[index] != '\0' && compactLength < availableDigits; index++) {
            if (text[index] == ' ') {
                continue;
            }
            compact[compactLength++] = text[index];
        }
    }

    const uint8_t offset = startIndex + ((availableDigits - static_cast<uint8_t>(compactLength)) / 2u);
    for (uint8_t index = 0; index < compactLength; index++) {
        raw[offset + index] = encodeChar(compact[index]);
    }

    writeRawDigits(raw);
}

void updateLEDScore(uint32_t score, bool wifiConnected) {
    if (!displayReady) {
        return;
    }

    if (score > MAX_SCORE) {
        score = MAX_SCORE;
    }

    uint8_t digits[6] = {
        static_cast<uint8_t>((score / 100000) % 10),
        static_cast<uint8_t>((score / 10000) % 10),
        static_cast<uint8_t>((score / 1000) % 10),
        static_cast<uint8_t>((score / 100) % 10),
        static_cast<uint8_t>((score / 10) % 10),
        static_cast<uint8_t>(score % 10)
    };

    uint16_t raw[6] = {};
    for (uint8_t index = 0; index < 6; index++) {
        raw[index] = segmentPatterns[digits[index]];
    }
    if (wifiConnected) {
        raw[0] |= SEG_DP;
    }

    writeRawDigits(raw);
}

void clearDisplay() {
    if (!displayReady) {
        return;
    }

    ledDisplay.clear();
    ledDisplay.writeDisplay();
}

void setDisplayBrightness(uint8_t level) {
    if (!displayReady) {
        return;
    }

    if (level > 15) {
        level = 15;
    }
    ledDisplay.setBrightness(level);
}
