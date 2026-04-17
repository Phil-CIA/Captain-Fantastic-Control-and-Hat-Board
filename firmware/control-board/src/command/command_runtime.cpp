#include "command_runtime.h"

#include "audio_commands.h"
#include "app_mode_config.h"
#include "captain_mapping.h"
#include "command_shared.h"
#include "input_commands.h"
#include "ota_commands.h"
#include "service_menu_runtime.h"
#include <WiFi.h>

namespace captain {
namespace command {
namespace {

constexpr bool CAPTAIN_ENABLE_OUTPUT_TEST = CAPTAIN_ENABLE_OUTPUT_TEST_DEFAULT;

}  // namespace

void pollSerial(captain::ota::Runtime& otaRuntime,
                captain::audio::Runtime& audioRuntime,
                captain::matrix::Runtime& matrixRuntime,
                captain::protocol::Runtime& protocolRuntime,
                captain::service::Runtime& serviceRuntime,
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
        protocolRuntime,
        serviceRuntime,
        inputOverlayRuntime,
        otaStatusCallback,
        stopAudioCallback,
        oledStatusWriter
    };

    if (command == "mp3 help") {
        Serial.println("MP3 commands: mp3 startup | mp3 attract | mp3 start | mp3 bonus | mp3 gameover | mp3 hiscore | mp3 stop | mp3 storage | mp3 volume [0-100]");
        Serial.println("OTA commands: ota help | ota status | ota wifi <ssid> <pass> | ota flash [http-url]");
        Serial.println("Matrix commands: matrix status | matrix proto status | matrix lamp clear | matrix lamp set <row> <col> <0|1>");
        Serial.println("Service commands: service help | service menu | service status | service next | service prev | service set <0-100> | service run [switch|coil|audio] | service save | service cancel | service exit");
        Serial.println("Input commands: input status");
        Serial.println("Mode commands: mode");
        Serial.println("System commands: system status");
        return;
    }

    if (command == "system status") {
        const bool matrixDetected = captain::matrix::isMatrixDetected(matrixRuntime);
        const bool matrixLinkHealthy = captain::matrix::isLinkHealthy(matrixRuntime);
        const bool degradedMode = !matrixLinkHealthy;

        Serial.println("System readiness summary");
        Serial.printf("  Profile: %s\n", CAPTAIN_APP_MODE_NAME);
        Serial.printf("  WiFi: %s\n", WiFi.status() == WL_CONNECTED ? "connected" : "down");
        Serial.printf("  OTA status: %s\n", captain::ota::statusText(otaRuntime));
        Serial.printf("  Audio storage SPIFFS/W25Q128: %s / %s\n",
                      captain::audio::isInternalSpiffsReady(audioRuntime) ? "ready" : "down",
                      captain::audio::isExternalFlashReady(audioRuntime) ? "ready" : "down");
        Serial.printf("  Audio gain: %.2f\n", captain::audio::getMasterGain(audioRuntime));
        Serial.printf("  Matrix detected/link: %s / %s\n",
                      matrixDetected ? "yes" : "no",
                      matrixLinkHealthy ? "healthy" : "down");
        Serial.printf("  Matrix diag flags: 0x%02X\n", captain::matrix::diagnosticFlags(matrixRuntime));
        Serial.printf("  Protocol tx ok/fail: %lu / %lu\n",
                      static_cast<unsigned long>(captain::protocol::txSuccessCount(protocolRuntime)),
                      static_cast<unsigned long>(captain::protocol::txFailureCount(protocolRuntime)));
        Serial.printf("  Protocol model/version: %u / %u.%u.%u\n",
                      static_cast<unsigned>(captain::protocol::protocolModel()),
                      static_cast<unsigned>(captain::protocol::protocolVersionMajor()),
                      static_cast<unsigned>(captain::protocol::protocolVersionMinor()),
                      static_cast<unsigned>(captain::protocol::protocolVersionPatch()));
        Serial.printf("  Service menu active: %s\n", serviceRuntime.active ? "yes" : "no");
        Serial.printf("  Degraded mode: %s\n", degradedMode ? "YES (matrix unavailable)" : "no");
        return;
    }

    if (command == "matrix status") {
        Serial.printf("Matrix detected: %s\n", captain::matrix::isMatrixDetected(matrixRuntime) ? "yes" : "no");
        Serial.printf("Matrix link healthy: %s\n", captain::matrix::isLinkHealthy(matrixRuntime) ? "yes" : "no");
        Serial.printf("Matrix diag flags: 0x%02X\n", captain::matrix::diagnosticFlags(matrixRuntime));
        return;
    }

    if (command == "matrix proto status") {
        Serial.printf("Protocol model: %u\n", static_cast<unsigned>(captain::protocol::protocolModel()));
        Serial.printf("Protocol version: %u.%u.%u\n",
                      static_cast<unsigned>(captain::protocol::protocolVersionMajor()),
                      static_cast<unsigned>(captain::protocol::protocolVersionMinor()),
                      static_cast<unsigned>(captain::protocol::protocolVersionPatch()));
        Serial.printf("Protocol refresh interval: %lu ms\n",
                      static_cast<unsigned long>(captain::protocol::refreshIntervalMs()));
        Serial.printf("Protocol tx frame counter: %lu\n",
                      static_cast<unsigned long>(captain::protocol::txFrameCounter(protocolRuntime)));
        Serial.printf("Protocol tx ok: %lu\n", static_cast<unsigned long>(captain::protocol::txSuccessCount(protocolRuntime)));
        Serial.printf("Protocol tx fail: %lu\n", static_cast<unsigned long>(captain::protocol::txFailureCount(protocolRuntime)));
        Serial.printf("Protocol refreshes dirty/heartbeat: %lu / %lu\n",
                      static_cast<unsigned long>(captain::protocol::dirtyRefreshCount(protocolRuntime)),
                      static_cast<unsigned long>(captain::protocol::heartbeatRefreshCount(protocolRuntime)));
        for (uint8_t row = 0; row < CAPTAIN_LAMP_ROWS; row++) {
            Serial.printf("  Lamp row %u mask=0x%02X\n",
                          static_cast<unsigned>(row),
                          static_cast<unsigned>(captain::protocol::lampRowMask(protocolRuntime, row)));
        }
        return;
    }

    if (command == "matrix lamp clear") {
        captain::protocol::clearLamps(protocolRuntime);
        Serial.println("Matrix lamp intent cleared");
        return;
    }

    if (command.startsWith("matrix lamp set ")) {
        int row = -1;
        int col = -1;
        int state = 0;

        if (sscanf(command.c_str(), "matrix lamp set %d %d %d", &row, &col, &state) != 3) {
            Serial.println("Usage: matrix lamp set <row> <col> <0|1>");
            return;
        }

        if (row < 0 || row >= CAPTAIN_LAMP_ROWS || col < 0 || col >= CAPTAIN_LAMP_COLS) {
            Serial.printf("Lamp bounds: row=0..%u col=0..%u\n",
                          static_cast<unsigned>(CAPTAIN_LAMP_ROWS - 1),
                          static_cast<unsigned>(CAPTAIN_LAMP_COLS - 1));
            return;
        }

        captain::protocol::setLamp(protocolRuntime,
                                   static_cast<uint8_t>(row),
                                   static_cast<uint8_t>(col),
                                   state != 0);

        Serial.printf("Matrix lamp intent R%dC%d (%s) -> %s\n",
                      row,
                      col,
                      captainLampName(static_cast<uint8_t>(row), static_cast<uint8_t>(col)),
                      state != 0 ? "ON" : "off");
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

    if (captain::service::handleCommand(command,
                                        commandRaw,
                                        serviceRuntime,
                                        audioRuntime,
                                        oledStatusWriter)) {
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