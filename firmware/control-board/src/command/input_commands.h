#ifndef CAPTAIN_INPUT_COMMANDS_H
#define CAPTAIN_INPUT_COMMANDS_H

#include <Arduino.h>

#include "command_shared.h"

namespace captain {
namespace command {

bool handleInputCommand(const String& command, CommandContext& context);

}  // namespace command
}  // namespace captain

#endif