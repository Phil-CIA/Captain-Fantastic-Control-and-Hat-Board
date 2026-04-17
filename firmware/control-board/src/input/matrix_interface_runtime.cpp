#include "matrix_interface_runtime.h"

#include <string.h>

#include <Arduino.h>
#include <Wire.h>

#include "captain_mapping.h"

namespace captain {
namespace matrix {
namespace {

constexpr uint32_t CAPTAIN_MATRIX_SWITCH_POLL_MS = 20;
constexpr uint32_t CAPTAIN_MATRIX_LINK_TIMEOUT_MS = 1000;

bool readRegisterBlock(uint8_t startRegister, uint8_t* destination, size_t length) {
    if (destination == nullptr || length == 0) {
        return false;
    }

    Wire.beginTransmission(CAPTAIN_MATRIX_I2C_ADDRESS);
    Wire.write(startRegister);
    if (Wire.endTransmission(false) != 0) {
        return false;
    }

    const size_t bytesRead = Wire.requestFrom(static_cast<int>(CAPTAIN_MATRIX_I2C_ADDRESS), static_cast<int>(length));
    if (bytesRead != length) {
        while (Wire.available()) {
            Wire.read();
        }
        return false;
    }

    for (size_t index = 0; index < length; index++) {
        if (!Wire.available()) {
            return false;
        }
        destination[index] = static_cast<uint8_t>(Wire.read());
    }

    return true;
}

void emitSwitchEdges(Runtime& runtime, uint32_t nowMs, SwitchEdgeCallback callback) {
    if (callback == nullptr) {
        return;
    }

    for (uint8_t row = 0; row < CAPTAIN_SWITCH_ROWS; row++) {
        for (uint8_t col = 0; col < CAPTAIN_SWITCH_COLS; col++) {
            const size_t bitIndex = captainSwitchBitIndex(row, col);
            const bool previous = captainGetBit(runtime.previousSwitchBytes, bitIndex);
            const bool current = captainGetBit(runtime.currentSwitchBytes, bitIndex);
            if (current == previous) {
                continue;
            }

            callback(row, col, current, nowMs);
        }
    }
}

void refreshLinkHealth(Runtime& runtime, uint32_t nowMs) {
    runtime.linkHealthy = runtime.matrixDetected && (nowMs - runtime.lastHeartbeatMs <= CAPTAIN_MATRIX_LINK_TIMEOUT_MS);
}

}  // namespace

void initialize(Runtime& runtime) {
    runtime.lastPollMs = 0;
    runtime.lastHeartbeatMs = 0;
    runtime.matrixDetected = false;
    runtime.linkHealthy = false;
    runtime.firstSnapshotReceived = false;
    runtime.diagFlags = 0;
    memset(runtime.currentSwitchBytes, 0, sizeof(runtime.currentSwitchBytes));
    memset(runtime.previousSwitchBytes, 0, sizeof(runtime.previousSwitchBytes));
}

void begin(Runtime& runtime, uint32_t nowMs) {
    uint8_t diagFlags = 0;
    if (readRegisterBlock(CAPTAIN_MATRIX_REG_DIAG_BASE, &diagFlags, 1)) {
        runtime.diagFlags = diagFlags;
        runtime.matrixDetected = true;
        runtime.lastHeartbeatMs = nowMs;
    }

    uint8_t switchBytes[CAPTAIN_SWITCH_BYTES] = {};
    if (readRegisterBlock(CAPTAIN_MATRIX_REG_SWITCH_BASE, switchBytes, CAPTAIN_SWITCH_BYTES)) {
        memcpy(runtime.currentSwitchBytes, switchBytes, sizeof(runtime.currentSwitchBytes));
        memcpy(runtime.previousSwitchBytes, switchBytes, sizeof(runtime.previousSwitchBytes));
        runtime.firstSnapshotReceived = true;
        runtime.matrixDetected = true;
        runtime.lastHeartbeatMs = nowMs;
    }

    refreshLinkHealth(runtime, nowMs);
}

void update(Runtime& runtime, uint32_t nowMs, SwitchEdgeCallback callback) {
    if (nowMs - runtime.lastPollMs < CAPTAIN_MATRIX_SWITCH_POLL_MS) {
        refreshLinkHealth(runtime, nowMs);
        return;
    }

    runtime.lastPollMs = nowMs;

    uint8_t diagFlags = 0;
    const bool diagReadOk = readRegisterBlock(CAPTAIN_MATRIX_REG_DIAG_BASE, &diagFlags, 1);
    if (diagReadOk) {
        runtime.diagFlags = diagFlags;
        runtime.matrixDetected = true;
        runtime.lastHeartbeatMs = nowMs;
    }

    uint8_t switchBytes[CAPTAIN_SWITCH_BYTES] = {};
    const bool switchReadOk = readRegisterBlock(CAPTAIN_MATRIX_REG_SWITCH_BASE, switchBytes, CAPTAIN_SWITCH_BYTES);
    if (switchReadOk) {
        runtime.matrixDetected = true;
        runtime.lastHeartbeatMs = nowMs;

        memcpy(runtime.currentSwitchBytes, switchBytes, sizeof(runtime.currentSwitchBytes));

        if (runtime.firstSnapshotReceived) {
            emitSwitchEdges(runtime, nowMs, callback);
        } else {
            runtime.firstSnapshotReceived = true;
        }

        memcpy(runtime.previousSwitchBytes, runtime.currentSwitchBytes, sizeof(runtime.previousSwitchBytes));
    }

    refreshLinkHealth(runtime, nowMs);
}

bool isLinkHealthy(const Runtime& runtime) {
    return runtime.linkHealthy;
}

bool isMatrixDetected(const Runtime& runtime) {
    return runtime.matrixDetected;
}

uint8_t diagnosticFlags(const Runtime& runtime) {
    return runtime.diagFlags;
}

}  // namespace matrix
}  // namespace captain