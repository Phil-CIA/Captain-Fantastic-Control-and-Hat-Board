#ifndef DIRECT_INPUT_RUNTIME_H
#define DIRECT_INPUT_RUNTIME_H

#include <Arduino.h>

#include "direct_input_config.h"

namespace captain {
namespace input {

using DirectInputChangeCallback = void (*)(uint8_t index, bool asserted, uint32_t nowMs);

void initialize();
void refreshAssertedStates();
void poll(uint32_t nowMs, uint32_t pollIntervalMs, DirectInputChangeCallback callback);
bool isAsserted(uint8_t index);

}  // namespace input
}  // namespace captain

#endif
