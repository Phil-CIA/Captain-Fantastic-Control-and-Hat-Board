#include "input_overlay_runtime.h"

#include <stdio.h>

#include "app_mode_config.h"
#include "direct_input_config.h"
#include "direct_input_runtime.h"

namespace captain {
namespace input {
namespace overlay {
namespace {

constexpr uint32_t EVENT_OVERLAY_MS = 3000U;
constexpr uint32_t STATUS_OVERLAY_MS = 5000U;
constexpr uint32_t IDLE_REFRESH_MS = 2500U;

bool shouldShowEventOverlay(uint8_t index, bool deepOverlayEnabled) {
    return deepOverlayEnabled || index == DIRECT_INPUT_TILT || index == DIRECT_INPUT_START;
}

}  // namespace

void initialize(Runtime& runtime, uint32_t nowMs) {
    runtime.overlayUntilMs = nowMs;
    runtime.lastIdleMs = nowMs;
}

void renderStatusOverlay(const Runtime&, OledStatusWriter writer, const char* header) {
    if (writer == nullptr) {
        return;
    }

    char line2[28];
    char line3[28];

    snprintf(line2,
             sizeof(line2),
             "Tilt:%s Start:%s",
             captain::input::isAsserted(DIRECT_INPUT_TILT) ? "ON" : "off",
             captain::input::isAsserted(DIRECT_INPUT_START) ? "ON" : "off");
    snprintf(line3,
             sizeof(line3),
             "SW1:%s SW2:%s",
             captain::input::isAsserted(DIRECT_INPUT_SW1) ? "ON" : "off",
             captain::input::isAsserted(DIRECT_INPUT_SW2) ? "ON" : "off");

    writer(header != nullptr ? header : "Input check", line2, line3);
}

void showOverlay(Runtime& runtime,
                 uint32_t nowMs,
                 uint32_t durationMs,
                 OledStatusWriter writer,
                 const char* header) {
    runtime.overlayUntilMs = nowMs + durationMs;
    renderStatusOverlay(runtime, writer, header);
}

void onInputChanged(Runtime& runtime,
                    uint8_t index,
                    uint32_t nowMs,
                    bool deepOverlayEnabled,
                    OledStatusWriter writer) {
    if (!shouldShowEventOverlay(index, deepOverlayEnabled)) {
        return;
    }

    showOverlay(runtime, nowMs, EVENT_OVERLAY_MS, writer, "Input event");
}

void onInputStatusCommand(Runtime& runtime, uint32_t nowMs, OledStatusWriter writer) {
    showOverlay(runtime, nowMs, STATUS_OVERLAY_MS, writer, "Input status");
}

void updateIdle(Runtime& runtime,
                uint32_t nowMs,
                bool otaInProgress,
                OledStatusWriter writer) {
    if (otaInProgress || writer == nullptr) {
        return;
    }

    if (nowMs < runtime.overlayUntilMs) {
        return;
    }

    if (nowMs - runtime.lastIdleMs < IDLE_REFRESH_MS) {
        return;
    }

    runtime.lastIdleMs = nowMs;
    renderStatusOverlay(runtime, writer, "Mode: ATTRACT");
}

}  // namespace overlay
}  // namespace input
}  // namespace captain