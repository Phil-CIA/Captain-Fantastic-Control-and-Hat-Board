#include "ota_commands.h"

namespace captain {
namespace command {

bool handleOtaCommand(const String& command, const String& commandRaw, CommandContext& context) {
    if (command == "ota help") {
        Serial.println("OTA commands:");
        Serial.println("  ota status");
        Serial.println("  ota wifi <ssid> <password>");
        Serial.println("  ota flash <http-url>");
        Serial.println("  ota flash                 (uses CAPTAIN_OTA_DEFAULT_URL)");
        return true;
    }

    if (command == "ota status") {
        Serial.printf("OTA status: %s\n", captain::ota::statusText(context.otaRuntime));
        Serial.printf("OTA wifi: %s\n", context.otaRuntime.wifiSsid[0] ? context.otaRuntime.wifiSsid : "not set");
        const String defaultUrl = captain::ota::configuredDefaultUrl();
        Serial.printf("OTA default URL: %s\n", defaultUrl.length() > 0 ? defaultUrl.c_str() : "not set");
        Serial.printf("OTA in-progress: %s\n", context.otaRuntime.inProgress ? "yes" : "no");

        if (context.oledStatusWriter != nullptr) {
            char line2[26];
            snprintf(line2, sizeof(line2), "WiFi: %s", context.otaRuntime.wifiSsid[0] ? "set" : "not set");
            char line3[26];
            snprintf(line3, sizeof(line3), "State: %s", context.otaRuntime.inProgress ? "running" : "idle");
            context.oledStatusWriter("OTA status", line2, line3);
        }
        return true;
    }

    if (command.startsWith("ota wifi ")) {
        const String args = commandRaw.substring(9);
        const int split = args.indexOf(' ');
        if (split <= 0 || split >= static_cast<int>(args.length() - 1)) {
            Serial.println("Usage: ota wifi <ssid> <password>");
            return true;
        }

        const String ssid = args.substring(0, split);
        const String pass = args.substring(split + 1);
        captain::ota::setWifiCredentials(context.otaRuntime, ssid.c_str(), pass.c_str());
        captain::ota::publishStatus(context.otaRuntime,
                                    context.otaStatusCallback,
                                    "OTA creds updated",
                                    context.otaRuntime.wifiSsid,
                                    "ready to flash");
        captain::ota::connectWifiWithTimeout(context.otaRuntime, 4000U, true, context.otaStatusCallback);
        return true;
    }

    if (command == "ota flash" || command.startsWith("ota flash ")) {
        String url;
        if (command == "ota flash") {
            url = captain::ota::configuredDefaultUrl();
        } else {
            url = commandRaw.substring(10);
            url.trim();
        }

        if (url.length() == 0) {
            Serial.println("OTA URL missing. Set CAPTAIN_OTA_DEFAULT_URL or use: ota flash <url>");
            return true;
        }

        if (!(url.startsWith("http://") || url.startsWith("https://"))) {
            Serial.println("OTA URL must start with http:// or https://");
            return true;
        }

        captain::ota::runStagedOtaFromUrl(context.otaRuntime,
                                          url,
                                          captain::audio::isExternalFlashReady(context.audioRuntime),
                                          context.otaStatusCallback,
                                          context.stopAudioCallback);
        return true;
    }

    return false;
}

}  // namespace command
}  // namespace captain