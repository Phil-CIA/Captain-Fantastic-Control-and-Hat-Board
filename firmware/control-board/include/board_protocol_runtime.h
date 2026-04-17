#ifndef BOARD_PROTOCOL_RUNTIME_H
#define BOARD_PROTOCOL_RUNTIME_H

#include <stdint.h>

#include "captain_protocol.h"

namespace captain {
namespace protocol {

struct Runtime {
    uint32_t lastRefreshMs;
    uint32_t txSuccessCount;
    uint32_t txFailureCount;
    bool dirty;
    uint8_t lampRowMasks[CAPTAIN_LAMP_ROWS];
};

void initialize(Runtime& runtime);
void begin(Runtime& runtime, uint32_t nowMs);
void update(Runtime& runtime, uint32_t nowMs, bool matrixLinkHealthy);

bool setLamp(Runtime& runtime, uint8_t row, uint8_t col, bool enabled);
void clearLamps(Runtime& runtime);

uint32_t txSuccessCount(const Runtime& runtime);
uint32_t txFailureCount(const Runtime& runtime);
uint8_t lampRowMask(const Runtime& runtime, uint8_t row);

}  // namespace protocol
}  // namespace captain

#endif