#include "input_commands.h"

#include "direct_input_config.h"
#include "direct_input_runtime.h"

namespace captain {
namespace command {

bool handleInputCommand(const String& command, CommandContext& context) {
    if (command != "input status") {
        return false;
    }

    captain::input::refreshAssertedStates();
    for (uint8_t index = 0; index < DIRECT_INPUT_COUNT; index++) {
        const bool raw = digitalRead(CAPTAIN_DIRECT_INPUT_PINS[index]);
        const bool asserted = captain::input::isAsserted(index);
        Serial.printf("Input %s: raw=%u asserted=%s\n",
                      CAPTAIN_DIRECT_INPUT_NAMES[index],
                      raw ? 1u : 0u,
                      asserted ? "ACTIVE" : "inactive");
    }

    captain::input::overlay::onInputStatusCommand(context.inputOverlayRuntime,
                                                  millis(),
                                                  context.oledStatusWriter);
    return true;
}

}  // namespace command
}  // namespace captain