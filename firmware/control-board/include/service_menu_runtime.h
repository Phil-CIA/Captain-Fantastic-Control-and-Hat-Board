#ifndef SERVICE_MENU_RUNTIME_H
#define SERVICE_MENU_RUNTIME_H

#include <Arduino.h>

#include "audio_runtime.h"
#include "input_overlay_runtime.h"

namespace captain {
namespace service {

struct Runtime {
    bool active;
    bool dirty;
    uint8_t selectedIndex;
    uint8_t committedVolumePercent;
    uint8_t pendingVolumePercent;
};

void initialize(Runtime& runtime, float startingGain);
bool handleCommand(const String& command,
                   const String& commandRaw,
                   Runtime& runtime,
                   captain::audio::Runtime& audioRuntime,
                   captain::input::overlay::OledStatusWriter oledStatusWriter);

}  // namespace service
}  // namespace captain

#endif