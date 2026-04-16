#ifndef CAPTAIN_OTA_COMMANDS_H
#define CAPTAIN_OTA_COMMANDS_H

#include <Arduino.h>

#include "command_shared.h"

namespace captain {
namespace command {

bool handleOtaCommand(const String& command, const String& commandRaw, CommandContext& context);

}  // namespace command
}  // namespace captain

#endif