#ifndef MATRIX_INTERFACE_RUNTIME_H
#define MATRIX_INTERFACE_RUNTIME_H

#include <stdint.h>

#include "captain_protocol.h"

namespace captain {
namespace matrix {

using SwitchEdgeCallback = void (*)(uint8_t row, uint8_t col, bool closed, uint32_t nowMs);

struct Runtime {
    uint32_t lastPollMs;
    uint32_t lastHeartbeatMs;
    bool matrixDetected;
    bool linkHealthy;
    bool firstSnapshotReceived;
    uint8_t diagFlags;
    uint8_t currentSwitchBytes[CAPTAIN_SWITCH_BYTES];
    uint8_t previousSwitchBytes[CAPTAIN_SWITCH_BYTES];
};

void initialize(Runtime& runtime);
void begin(Runtime& runtime, uint32_t nowMs);
void update(Runtime& runtime, uint32_t nowMs, SwitchEdgeCallback callback);

bool isLinkHealthy(const Runtime& runtime);
bool isMatrixDetected(const Runtime& runtime);
uint8_t diagnosticFlags(const Runtime& runtime);

}  // namespace matrix
}  // namespace captain

#endif