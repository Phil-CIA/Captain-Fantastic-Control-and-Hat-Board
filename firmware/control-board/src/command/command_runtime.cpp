#include "command_runtime.h"

#include "audio_commands.h"
#include "app_mode_config.h"
#include "command_shared.h"
#include "input_commands.h"
#include "ota_commands.h"

namespace captain {
namespace command {
namespace {

constexpr bool CAPTAIN_ENABLE_OUTPUT_TEST = CAPTAIN_ENABLE_OUTPUT_TEST_DEFAULT;

}  // namespace

void pollSerial(captain::ota::Runtime& otaRuntime,
                captain::audio::Runtime& audioRuntime,
                captain::matrix::Runtime& matrixRuntime,
                captain::input::overlay::Runtime& inputOverlayRuntime,
                captain::ota::StatusCallback otaStatusCallback,
                captain::ota::StopAudioCallback stopAudioCallback,
                captain::input::overlay::OledStatusWriter oledStatusWriter) {
    if (!Serial.available()) {
        return;
    }

    String commandRaw = Serial.readStringUntil('\n');
    commandRaw.trim();
    if (commandRaw.length() == 0) {
        return;
    }

    String command = commandRaw;
    command.toLowerCase();

    CommandContext context = {
        otaRuntime,
        audioRuntime,
        matrixRuntime,
        inputOverlayRuntime,
        otaStatusCallback,
        stopAudioCallback,
        oledStatusWriter
    };

    if (command == "mp3 help") {
        Serial.println("MP3 commands: mp3 startup | mp3 attract | mp3 start | mp3 bonus | mp3 gameover | mp3 hiscore | mp3 stop | mp3 storage");
        Serial.println("OTA commands: ota help | ota status | ota wifi <ssid> <pass> | ota flash [http-url]");
        Serial.println("Matrix commands: matrix status");
        Serial.println("Input commands: input status");
        Serial.println("Mode commands: mode");
        return;
    }

    if (command == "matrix status") {
        Serial.printf("Matrix detected: %s\n", captain::matrix::isMatrixDetected(matrixRuntime) ? "yes" : "no");
        Serial.printf("Matrix link healthy: %s\n", captain::matrix::isLinkHealthy(matrixRuntime) ? "yes" : "no");
        Serial.printf("Matrix diag flags: 0x%02X\n", captain::matrix::diagnosticFlags(matrixRuntime));
        return;
    }

    if (command == "mode") {
        Serial.printf("Active profile: %s\n", CAPTAIN_APP_MODE_NAME);
        Serial.printf("Output test default: %s\n", CAPTAIN_ENABLE_OUTPUT_TEST ? "enabled" : "disabled");
        Serial.printf("Deep input overlay: %s\n", CAPTAIN_ENABLE_DEEP_INPUT_OVERLAY ? "enabled" : "disabled");
        if (oledStatusWriter != nullptr) {
            oledStatusWriter("Mode profile",
                             CAPTAIN_APP_MODE_NAME,
                             CAPTAIN_ENABLE_OUTPUT_TEST ? "Test profile" : "System profile");
        }
        return;
    }

    if (handleInputCommand(command, context)) {
        return;
    }

    if (handleOtaCommand(command, commandRaw, context)) {
        return;
    }

    if (handleAudioCommand(command, context)) {
        return;
    }

    Serial.printf("Unknown command: %s\n", command.c_str());
    Serial.println("Type 'mp3 help' for options");
}

}  // namespace command
}  // namespace captain