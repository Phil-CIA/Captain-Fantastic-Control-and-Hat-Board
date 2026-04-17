#ifndef CAPTAIN_COMMAND_SHARED_H
#define CAPTAIN_COMMAND_SHARED_H

#include "command_runtime.h"

namespace captain {
namespace command {

struct CommandContext {
    captain::ota::Runtime& otaRuntime;
    captain::audio::Runtime& audioRuntime;
    captain::matrix::Runtime& matrixRuntime;
    captain::input::overlay::Runtime& inputOverlayRuntime;
    captain::ota::StatusCallback otaStatusCallback;
    captain::ota::StopAudioCallback stopAudioCallback;
    captain::input::overlay::OledStatusWriter oledStatusWriter;
};

}  // namespace command
}  // namespace captain

#endif