#ifndef INPUT_OVERLAY_RUNTIME_H
#define INPUT_OVERLAY_RUNTIME_H

#include <Arduino.h>

namespace captain {
namespace input {
namespace overlay {

using OledStatusWriter = void (*)(const char* line1, const char* line2, const char* line3);

struct Runtime {
    uint32_t overlayUntilMs;
    uint32_t lastIdleMs;
};

void initialize(Runtime& runtime, uint32_t nowMs);
void renderStatusOverlay(const Runtime& runtime, OledStatusWriter writer, const char* header);
void showOverlay(Runtime& runtime,
                 uint32_t nowMs,
                 uint32_t durationMs,
                 OledStatusWriter writer,
                 const char* header);
void onInputChanged(Runtime& runtime,
                    uint8_t index,
                    uint32_t nowMs,
                    bool deepOverlayEnabled,
                    OledStatusWriter writer);
void onInputStatusCommand(Runtime& runtime, uint32_t nowMs, OledStatusWriter writer);
void updateIdle(Runtime& runtime,
                uint32_t nowMs,
                bool otaInProgress,
                OledStatusWriter writer);

}  // namespace overlay
}  // namespace input
}  // namespace captain

#endif