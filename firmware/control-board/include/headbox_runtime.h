#ifndef HEADBOX_RUNTIME_H
#define HEADBOX_RUNTIME_H

#include <Arduino.h>

namespace captain {
namespace headbox {

struct Runtime {
    uint32_t lastUpdateMs;
    uint16_t lastPattern;
};

// Configure 595 GPIO pins and zero all lamps.
void initialize(Runtime& rt);

// Boot flash + walk sequence shown during setup().
void runStartupSequence(Runtime& rt);

// Called every loop() tick. otaInProgress causes all-lamps blink.
// score is used for score-based lamp pattern when attract loop is disabled.
void update(Runtime& rt, uint32_t now, bool otaInProgress, uint32_t score = 0U);

}  // namespace headbox
}  // namespace captain

#endif  // HEADBOX_RUNTIME_H
