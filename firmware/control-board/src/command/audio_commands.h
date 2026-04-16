#ifndef CAPTAIN_AUDIO_COMMANDS_H
#define CAPTAIN_AUDIO_COMMANDS_H

#include <Arduino.h>

#include "command_shared.h"

namespace captain {
namespace command {

bool handleAudioCommand(const String& command, CommandContext& context);

}  // namespace command
}  // namespace captain

#endif