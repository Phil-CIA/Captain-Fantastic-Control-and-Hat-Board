#ifndef COMMAND_RUNTIME_H
#define COMMAND_RUNTIME_H

#include <Arduino.h>

#include "audio_runtime.h"
#include "input_overlay_runtime.h"
#include "matrix_interface_runtime.h"
#include "ota_runtime.h"
#include "service_menu_runtime.h"

namespace captain {
namespace command {

void pollSerial(captain::ota::Runtime& otaRuntime,
                captain::audio::Runtime& audioRuntime,
                captain::matrix::Runtime& matrixRuntime,
                captain::service::Runtime& serviceRuntime,
                captain::input::overlay::Runtime& inputOverlayRuntime,
                captain::ota::StatusCallback otaStatusCallback,
                captain::ota::StopAudioCallback stopAudioCallback,
                captain::input::overlay::OledStatusWriter oledStatusWriter);

}  // namespace command
}  // namespace captain

#endif