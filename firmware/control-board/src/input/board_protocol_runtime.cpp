#include "board_protocol_runtime.h"

#include <string.h>

#include <Arduino.h>
#include <Wire.h>

namespace captain {
namespace protocol {
namespace {

constexpr uint32_t CAPTAIN_PROTOCOL_REFRESH_MS = 250;

bool writeLampRegisterBlock(const uint8_t* rowMasks, size_t count) {
    if (rowMasks == nullptr || count == 0) {
        return false;
    }

    Wire.beginTransmission(CAPTAIN_MATRIX_I2C_ADDRESS);
    Wire.write(CAPTAIN_MATRIX_REG_LAMP_BASE);

    for (size_t row = 0; row < count; row++) {
        Wire.write(rowMasks[row]);
    }

    return Wire.endTransmission() == 0;
}

}  // namespace

void initialize(Runtime& runtime) {
    runtime.lastRefreshMs = 0;
    runtime.lastTxMs = 0;
    runtime.txSuccessCount = 0;
    runtime.txFailureCount = 0;
    runtime.dirtyRefreshCount = 0;
    runtime.heartbeatRefreshCount = 0;
    runtime.txFrameCounter = 0;
    runtime.dirty = true;
    memset(runtime.lampRowMasks, 0, sizeof(runtime.lampRowMasks));
}

void begin(Runtime& runtime, uint32_t nowMs) {
    runtime.lastRefreshMs = nowMs;
    runtime.lastTxMs = nowMs;
    runtime.dirty = true;
}

void update(Runtime& runtime, uint32_t nowMs, bool matrixLinkHealthy) {
    if (!matrixLinkHealthy) {
        return;
    }

    const bool dueToDirty = runtime.dirty;
    const bool dueToHeartbeat = !runtime.dirty && (nowMs - runtime.lastRefreshMs >= CAPTAIN_PROTOCOL_REFRESH_MS);

    if (!dueToDirty && !dueToHeartbeat) {
        return;
    }

    const bool ok = writeLampRegisterBlock(runtime.lampRowMasks, CAPTAIN_LAMP_ROWS);
    runtime.lastRefreshMs = nowMs;

    if (ok) {
        runtime.txSuccessCount++;
        runtime.txFrameCounter++;
        runtime.lastTxMs = nowMs;

        if (dueToDirty) {
            runtime.dirtyRefreshCount++;
        } else {
            runtime.heartbeatRefreshCount++;
        }

        runtime.dirty = false;
    } else {
        runtime.txFailureCount++;
    }
}

bool setLamp(Runtime& runtime, uint8_t row, uint8_t col, bool enabled) {
    if (row >= CAPTAIN_LAMP_ROWS || col >= CAPTAIN_LAMP_COLS) {
        return false;
    }

    const uint8_t mask = captainMatrixLampRowMask(col);
    if (enabled) {
        runtime.lampRowMasks[row] |= mask;
    } else {
        runtime.lampRowMasks[row] &= static_cast<uint8_t>(~mask);
    }

    runtime.dirty = true;
    return true;
}

void clearLamps(Runtime& runtime) {
    memset(runtime.lampRowMasks, 0, sizeof(runtime.lampRowMasks));
    runtime.dirty = true;
}

uint32_t txSuccessCount(const Runtime& runtime) {
    return runtime.txSuccessCount;
}

uint32_t txFailureCount(const Runtime& runtime) {
    return runtime.txFailureCount;
}

uint32_t dirtyRefreshCount(const Runtime& runtime) {
    return runtime.dirtyRefreshCount;
}

uint32_t heartbeatRefreshCount(const Runtime& runtime) {
    return runtime.heartbeatRefreshCount;
}

uint32_t txFrameCounter(const Runtime& runtime) {
    return runtime.txFrameCounter;
}

uint32_t refreshIntervalMs() {
    return CAPTAIN_PROTOCOL_REFRESH_MS;
}

uint8_t protocolModel() {
    return CAPTAIN_PROTOCOL_MODEL_I2C_REGMAP;
}

uint8_t protocolVersionMajor() {
    return CAPTAIN_PROTOCOL_VERSION_MAJOR;
}

uint8_t protocolVersionMinor() {
    return CAPTAIN_PROTOCOL_VERSION_MINOR;
}

uint8_t protocolVersionPatch() {
    return CAPTAIN_PROTOCOL_VERSION_PATCH;
}

uint8_t lampRowMask(const Runtime& runtime, uint8_t row) {
    if (row >= CAPTAIN_LAMP_ROWS) {
        return 0;
    }
    return runtime.lampRowMasks[row];
}

}  // namespace protocol
}  // namespace captain