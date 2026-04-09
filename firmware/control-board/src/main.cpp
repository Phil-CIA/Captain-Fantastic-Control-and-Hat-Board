#include <Arduino.h>
#include <Wire.h>

#include "debug_oled_config.h"
#include "direct_input_config.h"
#include "displays.h"
#include "i2c_bus_config.h"
#include "solenoid_gpio_config.h"

namespace {
constexpr uint8_t HEARTBEAT_PIN = 2;
constexpr uint32_t HEARTBEAT_INTERVAL_MS = 500;
constexpr uint32_t DISPLAY_INTERVAL_MS = 250;
constexpr uint32_t INPUT_POLL_MS = 20;
constexpr uint32_t OUTPUT_TEST_INTERVAL_MS = 750;

// Keep this OFF by default.
// Only enable for bench proof-of-life when BOTH rails are intentionally fed
// from 5 V, the real 26 V rail is disconnected, and no real solenoid loads are attached.
constexpr bool CAPTAIN_ENABLE_OUTPUT_TEST = false;

bool heartbeatState = false;
uint32_t lastHeartbeatMs = 0;
uint32_t lastDisplayMs = 0;
uint32_t lastInputPollMs = 0;
uint32_t lastOutputTestMs = 0;
uint32_t displayCounter = 0;
int8_t activeOutput = -1;
bool previousInputState[DIRECT_INPUT_COUNT] = {};

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
        Serial.println("Debug OLED not detected at 0x3C/0x3D; confirm address and controller");
    }
}

void configureDirectInputs() {
    for (uint8_t index = 0; index < DIRECT_INPUT_COUNT; index++) {
        pinMode(CAPTAIN_DIRECT_INPUT_PINS[index], INPUT);
        previousInputState[index] = digitalRead(CAPTAIN_DIRECT_INPUT_PINS[index]);
    }
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
}

void updateHeartbeat(uint32_t now) {
    if (now - lastHeartbeatMs < HEARTBEAT_INTERVAL_MS) {
        return;
    }

    lastHeartbeatMs = now;
    heartbeatState = !heartbeatState;
    digitalWrite(HEARTBEAT_PIN, heartbeatState ? HIGH : LOW);
}

void updateDisplayCounter(uint32_t now) {
    if (now - lastDisplayMs < DISPLAY_INTERVAL_MS) {
        return;
    }

    lastDisplayMs = now;
    updateLEDScore(displayCounter);
    displayCounter = (displayCounter + 111U) % 1000000U;
}

void pollDirectInputs(uint32_t now) {
    if (now - lastInputPollMs < INPUT_POLL_MS) {
        return;
    }

    lastInputPollMs = now;
    for (uint8_t index = 0; index < DIRECT_INPUT_COUNT; index++) {
        const bool currentState = digitalRead(CAPTAIN_DIRECT_INPUT_PINS[index]);
        if (currentState == previousInputState[index]) {
            continue;
        }

        previousInputState[index] = currentState;
        const bool asserted = CAPTAIN_DIRECT_INPUT_ACTIVE_LOW ? !currentState : currentState;
        Serial.printf("Input %s -> %s\n",
                      CAPTAIN_DIRECT_INPUT_NAMES[index],
                      asserted ? "ACTIVE" : "inactive");
    }
}

void runOptionalOutputTest(uint32_t now) {
    if (!CAPTAIN_ENABLE_OUTPUT_TEST || now - lastOutputTestMs < OUTPUT_TEST_INTERVAL_MS) {
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
}
}  // namespace

void setup() {
    Serial.begin(115200);
    delay(250);

    pinMode(HEARTBEAT_PIN, OUTPUT);
    digitalWrite(HEARTBEAT_PIN, LOW);

    configureOutputsSafe();
    configureDirectInputs();

    Wire.begin(CAPTAIN_I2C_SDA_PIN, CAPTAIN_I2C_SCL_PIN, CAPTAIN_I2C_FREQUENCY_HZ);
    scanI2CBus();
    reportDebugOledStatus();
    initDisplay();
    displayStartupTest();
    displayGoodMessage(300);
    clearDisplay();

    printBringupBanner();
}

void loop() {
    const uint32_t now = millis();
    updateHeartbeat(now);
    updateDisplayCounter(now);
    pollDirectInputs(now);
    runOptionalOutputTest(now);
}
