#include "direct_input_runtime.h"

namespace captain {
namespace input {
namespace {

bool previousInputState[DIRECT_INPUT_COUNT] = {};
bool directInputAsserted[DIRECT_INPUT_COUNT] = {};
uint32_t lastPollMs = 0;

bool isAssertedFromRaw(bool rawState) {
    return CAPTAIN_DIRECT_INPUT_ACTIVE_LOW ? !rawState : rawState;
}

}  // namespace

void initialize() {
    for (uint8_t index = 0; index < DIRECT_INPUT_COUNT; index++) {
        pinMode(CAPTAIN_DIRECT_INPUT_PINS[index], INPUT);
        previousInputState[index] = digitalRead(CAPTAIN_DIRECT_INPUT_PINS[index]);
        directInputAsserted[index] = isAssertedFromRaw(previousInputState[index]);
    }
    lastPollMs = millis();
}

void refreshAssertedStates() {
    for (uint8_t index = 0; index < DIRECT_INPUT_COUNT; index++) {
        const bool rawState = digitalRead(CAPTAIN_DIRECT_INPUT_PINS[index]);
        previousInputState[index] = rawState;
        directInputAsserted[index] = isAssertedFromRaw(rawState);
    }
}

void poll(uint32_t nowMs, uint32_t pollIntervalMs, DirectInputChangeCallback callback) {
    if (nowMs - lastPollMs < pollIntervalMs) {
        return;
    }

    lastPollMs = nowMs;
    for (uint8_t index = 0; index < DIRECT_INPUT_COUNT; index++) {
        const bool currentState = digitalRead(CAPTAIN_DIRECT_INPUT_PINS[index]);
        if (currentState == previousInputState[index]) {
            continue;
        }

        previousInputState[index] = currentState;
        const bool asserted = isAssertedFromRaw(currentState);
        directInputAsserted[index] = asserted;

        if (callback != nullptr) {
            callback(index, asserted, nowMs);
        }
    }
}

bool isAsserted(uint8_t index) {
    if (index >= DIRECT_INPUT_COUNT) {
        return false;
    }
    return directInputAsserted[index];
}

}  // namespace input
}  // namespace captain
