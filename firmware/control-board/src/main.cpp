#include <Arduino.h>
#include <ctype.h>
#include <FS.h>
#include <SPI.h>
#include <SPIFFS.h>
#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <AudioFileSource.h>
#include <AudioFileSourceSPIFFS.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2S.h>
#include <SerialFlash.h>

#include "debug_oled_config.h"
#include "direct_input_config.h"
#include "direct_input_runtime.h"
#include "input_overlay_runtime.h"
#include "app_mode_config.h"
#include "audio_i2s_config.h"
#include "audio_runtime.h"
#include "captain_mapping.h"
#include "command_runtime.h"
#include "displays.h"
#include "external_flash_config.h"
#include "headbox_runtime.h"
#include "i2c_bus_config.h"
#include "matrix_interface_runtime.h"
#include "ota_config.h"
#include "ota_runtime.h"
#include "solenoid_gpio_config.h"

void stopAudioPlaybackForOtaBridge();

namespace {
#ifndef CAPTAIN_SERIALFLASH_SPI_HZ
#define CAPTAIN_SERIALFLASH_SPI_HZ 8000000
#endif

constexpr uint8_t HEARTBEAT_RGB_PIN = 15;
constexpr uint8_t HEARTBEAT_RGB_PIXELS = 1;
constexpr uint32_t HEARTBEAT_INTERVAL_MS = 500;
constexpr uint32_t DISPLAY_INTERVAL_MS = 250;
constexpr uint32_t INPUT_POLL_MS = 20;
constexpr uint32_t OUTPUT_TEST_INTERVAL_MS = 750;
constexpr float CAPTAIN_AUDIO_MP3_GAIN = 0.12f;

// System vs test behavior is selected by CAPTAIN_APP_MODE_TEST build flag.
constexpr bool CAPTAIN_ENABLE_OUTPUT_TEST = CAPTAIN_ENABLE_OUTPUT_TEST_DEFAULT;

bool heartbeatState = false;
uint32_t lastHeartbeatMs = 0;
uint32_t lastDisplayMs = 0;
uint32_t lastOutputTestMs = 0;
uint32_t displayCounter = 0;
int8_t activeOutput = -1;
captain::headbox::Runtime headboxRuntime = {};
captain::ota::Runtime otaRuntime = {};
captain::audio::Runtime audioRuntime = {};
captain::matrix::Runtime matrixRuntime = {};
captain::input::overlay::Runtime inputOverlayRuntime = {};
Adafruit_SSD1306 debugOled(CAPTAIN_DEBUG_OLED_WIDTH, CAPTAIN_DEBUG_OLED_HEIGHT, &Wire, CAPTAIN_DEBUG_OLED_RESET_PIN);
Adafruit_NeoPixel* heartbeatRgb = nullptr;
bool debugOledReady = false;
uint8_t debugOledAddress = CAPTAIN_DEBUG_OLED_I2C_ADDRESS_PRIMARY;

void writeDebugOledStatus(const char* line1, const char* line2 = nullptr, const char* line3 = nullptr);
void updateOtaStatus(const char* line1, const char* line2, const char* line3);
void updateOnboardDisplayForOtaStatus(const char* line1, const char* line2, const char* line3);
void onMatrixSwitchEdge(uint8_t row, uint8_t col, bool closed, uint32_t nowMs);
int8_t parsePercentFromStatusText(const char* text) {
    if (text == nullptr) {
        return -1;
    }

    const char* percentMark = strchr(text, '%');
    if (percentMark == nullptr) {
        return -1;
    }

    const char* begin = percentMark;
    while (begin > text && isdigit(static_cast<unsigned char>(*(begin - 1)))) {
        begin--;
    }

    if (begin == percentMark) {
        return -1;
    }

    int value = 0;
    for (const char* cursor = begin; cursor < percentMark; cursor++) {
        value = (value * 10) + (*cursor - '0');
    }

    if (value < 0) {
        value = 0;
    }
    if (value > 100) {
        value = 100;
    }
    return static_cast<int8_t>(value);
}

void drawWifiIconOnDebugOled(bool connected) {
    const int x = 116;
    const int y = 2;
    const uint16_t color = connected ? SSD1306_WHITE : SSD1306_BLACK;

    debugOled.drawCircle(x + 5, y + 8, 6, color);
    debugOled.drawCircle(x + 5, y + 8, 4, color);
    debugOled.fillCircle(x + 5, y + 8, connected ? 1 : 0, SSD1306_WHITE);

    if (!connected) {
        debugOled.drawLine(x, y + 13, x + 10, y + 3, SSD1306_WHITE);
    }
}

void drawProgressBarOnDebugOled(int8_t percent) {
    if (percent < 0) {
        return;
    }

    constexpr int BAR_X = 4;
    constexpr int BAR_Y = 56;
    constexpr int BAR_W = 120;
    constexpr int BAR_H = 7;

    const int fillWidth = (BAR_W - 2) * percent / 100;
    debugOled.drawRect(BAR_X, BAR_Y, BAR_W, BAR_H, SSD1306_WHITE);
    debugOled.fillRect(BAR_X + 1, BAR_Y + 1, fillWidth, BAR_H - 2, SSD1306_WHITE);
}

void updateIdleOledStatus(uint32_t now) {
    if (!debugOledReady) {
        return;
    }

    captain::input::overlay::updateIdle(inputOverlayRuntime, now, otaRuntime.inProgress, writeDebugOledStatus);
}

void updateOnboardDisplayForOtaStatus(const char* line1, const char* line2, const char* line3) {
    const bool wifiConnected = WiFi.status() == WL_CONNECTED;
    char text[6] = {};

    if (line1 == nullptr) {
        displayStatusMessage("OTA", wifiConnected);
        return;
    }

    if (strcmp(line1, "OTA WiFi connect") == 0) {
        displayStatusMessage("WAIT", false);
        return;
    }

    if (strcmp(line1, "OTA WiFi ready") == 0) {
        displayStatusMessage("READY", true);
        return;
    }

    if (strcmp(line1, "OTA download") == 0) {
        unsigned long percent = 0;
        if (line2 != nullptr && sscanf(line2, "DL %lu%%", &percent) == 1) {
            snprintf(text, sizeof(text), "D%03lu", percent > 999UL ? 999UL : percent);
            displayStatusMessage(text, wifiConnected);
        } else {
            displayStatusMessage("DOWN", wifiConnected);
        }
        return;
    }

    if (strcmp(line1, "OTA flashing") == 0) {
        unsigned long percent = 0;
        if (line2 != nullptr && sscanf(line2, "FL %lu%%", &percent) == 1) {
            snprintf(text, sizeof(text), "F%03lu", percent > 999UL ? 999UL : percent);
            displayStatusMessage(text, wifiConnected);
        } else {
            displayStatusMessage("FLASH", wifiConnected);
        }
        return;
    }

    if (strcmp(line1, "OTA stage ready") == 0) {
        displayStatusMessage("STAGE", wifiConnected);
        return;
    }

    if (strcmp(line1, "OTA success") == 0) {
        displayStatusMessage("DONE", wifiConnected);
        return;
    }

    if (strcmp(line1, "OTA busy") == 0) {
        displayStatusMessage("BUSY", wifiConnected);
        return;
    }

    if (strstr(line1, "fail") != nullptr ||
        strstr(line1, "missing") != nullptr ||
        strstr(line1, "down") != nullptr) {
        displayStatusMessage("FAIL", wifiConnected);
        return;
    }

    if (strcmp(line1, "OTA autostart") == 0) {
        displayStatusMessage("AUTO", wifiConnected);
        return;
    }

    if (strcmp(line1, "OTA creds updated") == 0) {
        displayStatusMessage("WIFI", wifiConnected);
        return;
    }

    if (line3 != nullptr && strstr(line3, "waiting") != nullptr) {
        displayStatusMessage("WAIT", wifiConnected);
        return;
    }

    displayStatusMessage("OTA", wifiConnected);
}

void updateOtaStatus(const char* line1, const char* line2, const char* line3) {
    writeDebugOledStatus(line1 != nullptr ? line1 : "OTA", line2, line3);
    updateOnboardDisplayForOtaStatus(line1, line2, line3);
}

void setHeartbeatColor(uint8_t red, uint8_t green, uint8_t blue) {
    if (heartbeatRgb == nullptr) {
        return;
    }

    heartbeatRgb->setPixelColor(0, heartbeatRgb->Color(red, green, blue));
    heartbeatRgb->show();
}

void runHeartbeatStartupCycle() {
    setHeartbeatColor(32, 0, 0);
    delay(180);
    setHeartbeatColor(0, 32, 0);
    delay(180);
    setHeartbeatColor(0, 0, 32);
    delay(180);
    setHeartbeatColor(0, 0, 0);
}


bool probeI2CAddress(uint8_t address) {
    Wire.beginTransmission(address);
    return Wire.endTransmission() == 0;
}

void scanI2CBus() {
    Serial.println("I2C scan start");
    uint8_t foundCount = 0;
    for (uint8_t address = 1; address < 127; address++) {
        if (!probeI2CAddress(address)) {
            continue;
        }

        foundCount++;
        Serial.printf("  found I2C device at 0x%02X\n", address);
    }

    if (foundCount == 0) {
        Serial.println("  no I2C devices detected");
    }
}

void reportDebugOledStatus() {
    if (!CAPTAIN_DEBUG_OLED_ENABLED) {
        Serial.println("Debug OLED support disabled");
        return;
    }

    const bool primaryFound = probeI2CAddress(CAPTAIN_DEBUG_OLED_I2C_ADDRESS_PRIMARY);
    const bool secondaryFound = probeI2CAddress(CAPTAIN_DEBUG_OLED_I2C_ADDRESS_SECONDARY);

    Serial.printf("Debug OLED expected size: %ux%u\n",
                  static_cast<unsigned>(CAPTAIN_DEBUG_OLED_WIDTH),
                  static_cast<unsigned>(CAPTAIN_DEBUG_OLED_HEIGHT));

    if (primaryFound || secondaryFound) {
        Serial.printf("Debug OLED detected at %s%s%s\n",
                      primaryFound ? "0x3C" : "",
                      (primaryFound && secondaryFound) ? " and " : "",
                      secondaryFound ? "0x3D" : "");
    } else {
        Serial.println("Debug OLED not detected at 0x3C/0x3D; confirm wiring and address");
    }
}

void writeDebugOledStatus(const char* line1, const char* line2, const char* line3) {
    if (!debugOledReady) {
        return;
    }

    debugOled.clearDisplay();
    debugOled.setTextSize(1);
    debugOled.setTextColor(SSD1306_WHITE);
    debugOled.drawLine(0, 14, 127, 14, SSD1306_WHITE);
    debugOled.setCursor(0, 2);
    debugOled.print(F("Captain Ctrl"));

    if (line1 != nullptr) {
        debugOled.setCursor(0, 20);
        debugOled.print(line1);
    }
    if (line2 != nullptr) {
        debugOled.setCursor(0, 33);
        debugOled.print(line2);
    }
    if (line3 != nullptr) {
        debugOled.setCursor(0, 46);
        debugOled.print(line3);
    }

    drawWifiIconOnDebugOled(WiFi.status() == WL_CONNECTED);

    int8_t percent = parsePercentFromStatusText(line2);
    if (percent < 0) {
        percent = parsePercentFromStatusText(line3);
    }
    drawProgressBarOnDebugOled(percent);

    debugOled.display();
}

void initDebugOled() {
    if (!CAPTAIN_DEBUG_OLED_ENABLED) {
        return;
    }

    const uint8_t candidateAddresses[2] = {
        CAPTAIN_DEBUG_OLED_I2C_ADDRESS_PRIMARY,
        CAPTAIN_DEBUG_OLED_I2C_ADDRESS_SECONDARY
    };

    for (uint8_t address : candidateAddresses) {
        if (!probeI2CAddress(address)) {
            continue;
        }

        if (debugOled.begin(SSD1306_SWITCHCAPVCC, address)) {
            debugOledReady = true;
            debugOledAddress = address;
            Serial.printf("Debug OLED initialized at 0x%02X\n", address);
            writeDebugOledStatus("Boot OK", "5V bring-up", "26V disconnected");
            return;
        }
    }

    Serial.println("Debug OLED detected but failed to initialize as SSD1306");
}

void configureOutputsSafe() {
    for (uint8_t index = 0; index < SOLENOID_COUNT; index++) {
        pinMode(CAPTAIN_SOLENOID_PINS[index], OUTPUT);
        digitalWrite(CAPTAIN_SOLENOID_PINS[index], LOW);
    }
}

void printBringupBanner() {
    Serial.println();
    Serial.println("Captain Fantastic control-board bring-up");
    Serial.printf("Profile: %s\n", CAPTAIN_APP_MODE_NAME);
    Serial.println("Mode: 5 V-only firmware development");
    Serial.println("Note: keep the real 26 V solenoid rail disconnected");
    Serial.printf("I2C: SDA=%u SCL=%u display=0x%02X matrix=0x%02X\n",
                  CAPTAIN_I2C_SDA_PIN,
                  CAPTAIN_I2C_SCL_PIN,
                  CAPTAIN_DISPLAY_I2C_ADDRESS,
                  CAPTAIN_MATRIX_I2C_ADDRESS);
    Serial.printf("Debug OLED candidates: 0x%02X / 0x%02X\n",
                  CAPTAIN_DEBUG_OLED_I2C_ADDRESS_PRIMARY,
                  CAPTAIN_DEBUG_OLED_I2C_ADDRESS_SECONDARY);
    if (debugOledReady) {
        Serial.printf("Debug OLED active at 0x%02X\n", debugOledAddress);
    }
}

void updateHeartbeat(uint32_t now) {
    if (now - lastHeartbeatMs < HEARTBEAT_INTERVAL_MS) {
        return;
    }

    lastHeartbeatMs = now;
    heartbeatState = !heartbeatState;
    setHeartbeatColor(heartbeatState ? 0 : 0, heartbeatState ? 24 : 0, heartbeatState ? 0 : 0);
}

void updateDisplayCounter(uint32_t now) {
    if (now - lastDisplayMs < DISPLAY_INTERVAL_MS) {
        return;
    }

    lastDisplayMs = now;
    updateLEDScore(displayCounter, WiFi.status() == WL_CONNECTED);
    displayCounter = (displayCounter + 111U) % 1000000U;
}

void onDirectInputChanged(uint8_t index, bool asserted, uint32_t now) {
    Serial.printf("Input %s -> %s\n",
                  CAPTAIN_DIRECT_INPUT_NAMES[index],
                  asserted ? "ACTIVE" : "inactive");

    captain::input::overlay::onInputChanged(inputOverlayRuntime,
                                            index,
                                            now,
                                            CAPTAIN_ENABLE_DEEP_INPUT_OVERLAY,
                                            writeDebugOledStatus);
}

void pollDirectInputs(uint32_t now) {
    captain::input::poll(now, INPUT_POLL_MS, onDirectInputChanged);
}

void runOptionalOutputTest(uint32_t now, bool matrixLinkHealthy) {
    if (!CAPTAIN_ENABLE_OUTPUT_TEST) {
        return;
    }

    if (!matrixLinkHealthy) {
        if (activeOutput >= 0) {
            digitalWrite(CAPTAIN_SOLENOID_PINS[activeOutput], LOW);
            activeOutput = -1;
        }
        return;
    }

    if (now - lastOutputTestMs < OUTPUT_TEST_INTERVAL_MS) {
        return;
    }

    lastOutputTestMs = now;

    if (activeOutput >= 0) {
        digitalWrite(CAPTAIN_SOLENOID_PINS[activeOutput], LOW);
    }

    activeOutput = static_cast<int8_t>((activeOutput + 1) % SOLENOID_COUNT);
    digitalWrite(CAPTAIN_SOLENOID_PINS[activeOutput], HIGH);

    Serial.printf("Output test -> %s (5 V bench proof-of-life only)\n",
                  CAPTAIN_SOLENOID_NAMES[activeOutput]);

    writeDebugOledStatus("Output test", CAPTAIN_SOLENOID_NAMES[activeOutput], "5V proof-of-life");
}

void onMatrixSwitchEdge(uint8_t row, uint8_t col, bool closed, uint32_t nowMs) {
    (void)nowMs;

    if (!closed) {
        return;
    }

    Serial.printf("Matrix switch closed R%uC%u -> %s\n",
                  static_cast<unsigned>(row),
                  static_cast<unsigned>(col),
                  captainSwitchName(row, col));
}

void pollSerialAudioCommands() {
    captain::command::pollSerial(otaRuntime,
                                 audioRuntime,
                                 matrixRuntime,
                                 inputOverlayRuntime,
                                 updateOtaStatus,
                                 ::stopAudioPlaybackForOtaBridge,
                                 writeDebugOledStatus);
}
}  // namespace

void stopAudioPlaybackForOtaBridge() {
    captain::audio::stop(audioRuntime);
}

void setup() {
    Serial.begin(115200);
    Serial.setTimeout(25);
    delay(250);

    static Adafruit_NeoPixel heartbeatRgbInstance(HEARTBEAT_RGB_PIXELS, HEARTBEAT_RGB_PIN, NEO_GRB + NEO_KHZ800);
    heartbeatRgb = &heartbeatRgbInstance;
    heartbeatRgb->begin();
    heartbeatRgb->setBrightness(64);
    setHeartbeatColor(0, 0, 0);
    runHeartbeatStartupCycle();

    configureOutputsSafe();
    captain::headbox::initialize(headboxRuntime);
    captain::input::initialize();
    captain::input::overlay::initialize(inputOverlayRuntime, millis());
    captain::ota::initialize(otaRuntime);
    captain::ota::seedCredentialsFromConfig(otaRuntime);
    captain::ota::connectWifiWithTimeout(otaRuntime, 5000U, true, updateOtaStatus);
    captain::audio::initialize(audioRuntime);
    captain::audio::initPath(audioRuntime, CAPTAIN_SERIALFLASH_SPI_HZ);

    Wire.begin(CAPTAIN_I2C_SDA_PIN, CAPTAIN_I2C_SCL_PIN, CAPTAIN_I2C_FREQUENCY_HZ);
    scanI2CBus();
    captain::matrix::initialize(matrixRuntime);
    captain::matrix::begin(matrixRuntime, millis());
    reportDebugOledStatus();
    initDebugOled();
    writeDebugOledStatus("Mode: STARTUP", CAPTAIN_APP_MODE_NAME, "Init checks...");
    initDisplay();
    captain::headbox::runStartupSequence(headboxRuntime);
    displayStartupTest();
    displayGoodMessage(300);
    displayStatusMessage("ATTR");
    delay(450);
    clearDisplay();

    printBringupBanner();
    Serial.println("Type 'mp3 help' in monitor to trigger event clips.");
    Serial.println("Type 'ota help' to run staged OTA via W25Q128.");
    Serial.println("Type 'input status' to verify Start/Tilt switches.");
    Serial.println("Type 'mode' to print active firmware profile.");

    captain::input::overlay::showOverlay(inputOverlayRuntime,
                                         millis(),
                                         3000U,
                                         writeDebugOledStatus,
                                         "Mode: ATTRACT");

    if (otaRuntime.autostartRequested) {
        const String autoUrl = captain::ota::configuredDefaultUrl();
        if (autoUrl.length() > 0) {
            captain::ota::publishStatus(otaRuntime, updateOtaStatus, "OTA autostart", "boot requested", nullptr);
            captain::ota::runStagedOtaFromUrl(otaRuntime,
                                              autoUrl,
                                              captain::audio::isExternalFlashReady(audioRuntime),
                                              updateOtaStatus,
                                              stopAudioPlaybackForOtaBridge);
        } else {
            captain::ota::publishStatus(otaRuntime, updateOtaStatus, "OTA autostart off", "default URL missing", nullptr);
        }
        otaRuntime.autostartRequested = false;
    }
}

void loop() {
    const uint32_t now = millis();
    captain::ota::maintainWifiConnection(otaRuntime, now, updateOtaStatus);
    updateHeartbeat(now);
    updateDisplayCounter(now);
    captain::matrix::update(matrixRuntime, now, onMatrixSwitchEdge);
    pollDirectInputs(now);
    runOptionalOutputTest(now, captain::matrix::isLinkHealthy(matrixRuntime));
    captain::headbox::update(headboxRuntime, now, otaRuntime.inProgress, displayCounter);
    captain::audio::updatePlayback(audioRuntime);
    captain::audio::updateDiagnostic(audioRuntime, now);
    updateIdleOledStatus(now);
    pollSerialAudioCommands();
}
