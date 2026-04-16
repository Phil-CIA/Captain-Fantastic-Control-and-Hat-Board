#include "headbox_runtime.h"
#include "headbox_595_config.h"
#include "headbox_attract_config.h"
#include "displays.h"

#include <Arduino.h>

namespace captain {
namespace headbox {

namespace {

constexpr uint32_t HEADBOX_UPDATE_INTERVAL_MS = 25;

void setLamp(uint16_t& pattern, CaptainHeadboxLampId lampId, bool on) {
    if (lampId >= HEADBOX_LAMP_COUNT) {
        return;
    }
    const uint8_t srBit = CAPTAIN_HEADBOX_LAMP_TO_SR_BIT[lampId];
    const uint16_t mask = static_cast<uint16_t>(1u << srBit);
    if (on) {
        pattern |= mask;
    } else {
        pattern &= static_cast<uint16_t>(~mask);
    }
}

void writeLamps(uint16_t pattern) {
    if (CAPTAIN_HEADBOX_595_DATA_PIN < 0 ||
        CAPTAIN_HEADBOX_595_CLOCK_PIN < 0 ||
        CAPTAIN_HEADBOX_595_LATCH_PIN < 0) {
        return;
    }
    const uint8_t bitOrder = CAPTAIN_HEADBOX_595_MSB_FIRST ? MSBFIRST : LSBFIRST;
    digitalWrite(CAPTAIN_HEADBOX_595_LATCH_PIN, LOW);
    shiftOut(static_cast<uint8_t>(CAPTAIN_HEADBOX_595_DATA_PIN),
             static_cast<uint8_t>(CAPTAIN_HEADBOX_595_CLOCK_PIN),
             bitOrder,
             static_cast<uint8_t>((pattern >> 8) & 0xFF));
    shiftOut(static_cast<uint8_t>(CAPTAIN_HEADBOX_595_DATA_PIN),
             static_cast<uint8_t>(CAPTAIN_HEADBOX_595_CLOCK_PIN),
             bitOrder,
             static_cast<uint8_t>(pattern & 0xFF));
    digitalWrite(CAPTAIN_HEADBOX_595_LATCH_PIN, HIGH);
}

uint16_t patternFromScore(uint32_t score, bool blink) {
    uint16_t pattern = 0;
    const uint8_t ballIndex = static_cast<uint8_t>((score / 1000U) % 5U);
    setLamp(pattern, static_cast<CaptainHeadboxLampId>(HEADBOX_BALL_1 + ballIndex), true);
    const uint8_t playerIndex = static_cast<uint8_t>((score / 5000U) % 4U);
    setLamp(pattern, static_cast<CaptainHeadboxLampId>(HEADBOX_PLAYER_1 + playerIndex), true);
    setLamp(pattern, HEADBOX_TILT, blink);
    setLamp(pattern, HEADBOX_GAME_OVER, score == 0U);
    return pattern;
}

uint16_t attractPattern(uint32_t now) {
    const CaptainHeadboxLampId chaseOrder[] = {
        HEADBOX_PLAYER_1,
        HEADBOX_PLAYER_2,
        HEADBOX_PLAYER_3,
        HEADBOX_PLAYER_4,
        HEADBOX_TILT,
        HEADBOX_BALL_5,
        HEADBOX_BALL_4,
        HEADBOX_BALL_3,
        HEADBOX_BALL_2,
        HEADBOX_BALL_1
    };
    const uint8_t chaseSteps = static_cast<uint8_t>(sizeof(chaseOrder) / sizeof(chaseOrder[0]));
    const uint8_t laps = max<uint8_t>(1, CAPTAIN_HEADBOX_ATTRACT_CHASE_LAPS);
    const uint32_t totalSlots = static_cast<uint32_t>(chaseSteps) * laps;
    const uint32_t slotMs = max<uint32_t>(CAPTAIN_HEADBOX_ATTRACT_MIN_STEP_MS,
                                          CAPTAIN_HEADBOX_ATTRACT_LOOP_PERIOD_MS / totalSlots);
    const uint32_t loopMs = slotMs * totalSlots;
    const uint32_t phaseMs = now % loopMs;
    const uint32_t slotIndex = phaseMs / slotMs;
    const uint32_t inSlotMs = phaseMs % slotMs;

    if (inSlotMs >= (slotMs / 2u)) {
        return 0;
    }

    const uint8_t chaseIndex = static_cast<uint8_t>(slotIndex % chaseSteps);
    uint16_t pattern = 0;
    setLamp(pattern, chaseOrder[chaseIndex], true);
    return pattern;
}

}  // namespace

void initialize(Runtime& rt) {
    rt.lastUpdateMs = 0;
    rt.lastPattern = 0;

    if (CAPTAIN_HEADBOX_595_DATA_PIN < 0 ||
        CAPTAIN_HEADBOX_595_CLOCK_PIN < 0 ||
        CAPTAIN_HEADBOX_595_LATCH_PIN < 0) {
        Serial.println("Headbox 595 disabled: pin mapping not set");
        return;
    }

    pinMode(CAPTAIN_HEADBOX_595_DATA_PIN, OUTPUT);
    pinMode(CAPTAIN_HEADBOX_595_CLOCK_PIN, OUTPUT);
    pinMode(CAPTAIN_HEADBOX_595_LATCH_PIN, OUTPUT);
    digitalWrite(CAPTAIN_HEADBOX_595_DATA_PIN, LOW);
    digitalWrite(CAPTAIN_HEADBOX_595_CLOCK_PIN, LOW);
    digitalWrite(CAPTAIN_HEADBOX_595_LATCH_PIN, HIGH);

    writeLamps(0);

    Serial.printf("Headbox 595: DATA=%d CLOCK=%d LATCH=%d order=%s\n",
                  static_cast<int>(CAPTAIN_HEADBOX_595_DATA_PIN),
                  static_cast<int>(CAPTAIN_HEADBOX_595_CLOCK_PIN),
                  static_cast<int>(CAPTAIN_HEADBOX_595_LATCH_PIN),
                  CAPTAIN_HEADBOX_595_MSB_FIRST ? "MSB" : "LSB");
}

void runStartupSequence(Runtime& rt) {
    displayStatusMessage("BOOT");

    for (uint8_t flash = 0; flash < 2; flash++) {
        writeLamps(0xFFFFu);
        delay(120);
        writeLamps(0x0000u);
        delay(80);
    }

    for (uint8_t lamp = 0; lamp < HEADBOX_LAMP_COUNT; lamp++) {
        uint16_t pattern = 0;
        setLamp(pattern, static_cast<CaptainHeadboxLampId>(lamp), true);
        writeLamps(pattern);
        delay(75);
    }

    writeLamps(0x0000u);
    rt.lastPattern = 0;
}

void update(Runtime& rt, uint32_t now, bool otaInProgress, uint32_t score) {
    if (now - rt.lastUpdateMs < HEADBOX_UPDATE_INTERVAL_MS) {
        return;
    }
    rt.lastUpdateMs = now;

    uint16_t pattern = 0;
    if (otaInProgress) {
        const bool phase = ((now / CAPTAIN_OTA_VISUAL_INTERVAL_MS) % 2U) != 0U;
        pattern = phase ? 0xFFFFu : 0x0000u;
    } else if (CAPTAIN_HEADBOX_ATTRACT_LOOP) {
        pattern = attractPattern(now);
    } else {
        const bool blink = ((now / 350U) % 2U) != 0U;
        pattern = patternFromScore(score, blink);
    }

    if (pattern == rt.lastPattern) {
        return;
    }

    rt.lastPattern = pattern;
    writeLamps(pattern);
}

}  // namespace headbox
}  // namespace captain
