#include "ota_runtime.h"

#include <HTTPClient.h>
#include <Update.h>
#include <WiFi.h>
#include <SerialFlash.h>

#include "ota_config.h"

namespace captain {
namespace ota {
namespace {

constexpr uint32_t CAPTAIN_OTA_PROGRESS_PRINT_MS = 700;
constexpr uint32_t CAPTAIN_OTA_WIFI_TIMEOUT_MS = 20000;
constexpr uint32_t CAPTAIN_WIFI_RECONNECT_INTERVAL_MS = 10000;
constexpr size_t CAPTAIN_OTA_COPY_BUFFER_BYTES = 1024;
constexpr const char* CAPTAIN_OTA_STAGE_FILE = "ota_stage.bin";

void printStatus(const char* line1, const char* line2, const char* line3) {
    Serial.print("OTA: ");
    Serial.print(line1 != nullptr ? line1 : "");
    if (line2 != nullptr) {
        Serial.print(" | ");
        Serial.print(line2);
    }
    if (line3 != nullptr) {
        Serial.print(" | ");
        Serial.print(line3);
    }
    Serial.println();
}

bool stageFirmwareToExternalFlash(Runtime& runtime,
                                  const String& url,
                                  bool externalFlashReady,
                                  StatusCallback statusCallback,
                                  uint32_t* stagedBytesOut) {
    if (!externalFlashReady) {
        publishStatus(runtime, statusCallback, "OTA extflash down", "W25Q128 not ready", nullptr);
        return false;
    }

    HTTPClient http;
    publishStatus(runtime, statusCallback, "OTA download", "HTTP begin", nullptr);
    if (!http.begin(url)) {
        publishStatus(runtime, statusCallback, "OTA HTTP fail", "begin()", nullptr);
        return false;
    }

    const int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        char codeText[24];
        snprintf(codeText, sizeof(codeText), "HTTP %d", httpCode);
        publishStatus(runtime, statusCallback, "OTA HTTP fail", codeText, nullptr);
        http.end();
        return false;
    }

    const int contentLength = http.getSize();
    if (contentLength <= 0) {
        publishStatus(runtime, statusCallback, "OTA HTTP fail", "bad length", nullptr);
        http.end();
        return false;
    }

    SerialFlash.remove(CAPTAIN_OTA_STAGE_FILE);
    if (!SerialFlash.createErasable(CAPTAIN_OTA_STAGE_FILE, static_cast<uint32_t>(contentLength))) {
        publishStatus(runtime, statusCallback, "OTA stage fail", "create file", nullptr);
        http.end();
        return false;
    }

    SerialFlashFile destination = SerialFlash.open(CAPTAIN_OTA_STAGE_FILE);
    if (!destination) {
        publishStatus(runtime, statusCallback, "OTA stage fail", "open file", nullptr);
        http.end();
        return false;
    }

    WiFiClient* stream = http.getStreamPtr();
    uint8_t buffer[CAPTAIN_OTA_COPY_BUFFER_BYTES];
    uint32_t received = 0;
    uint32_t lastPrintMs = 0;

    while (http.connected() && received < static_cast<uint32_t>(contentLength)) {
        const size_t available = stream->available();
        if (available == 0) {
            delay(2);
            continue;
        }

        const size_t toRead = (available > sizeof(buffer)) ? sizeof(buffer) : available;
        const int bytesRead = stream->readBytes(buffer, toRead);
        if (bytesRead <= 0) {
            continue;
        }

        const uint32_t bytesWritten = destination.write(buffer, static_cast<uint32_t>(bytesRead));
        if (bytesWritten != static_cast<uint32_t>(bytesRead)) {
            publishStatus(runtime, statusCallback, "OTA stage fail", "write mismatch", nullptr);
            destination.close();
            http.end();
            return false;
        }

        received += bytesWritten;

        const uint32_t now = millis();
        if (now - lastPrintMs >= CAPTAIN_OTA_PROGRESS_PRINT_MS) {
            lastPrintMs = now;
            const uint32_t percent = (received * 100U) / static_cast<uint32_t>(contentLength);
            char pctText[20];
            snprintf(pctText, sizeof(pctText), "DL %lu%%", static_cast<unsigned long>(percent));
            char bytesText[28];
            snprintf(bytesText,
                     sizeof(bytesText),
                     "%lu/%d",
                     static_cast<unsigned long>(received),
                     contentLength);
            publishStatus(runtime, statusCallback, "OTA download", pctText, bytesText);
        }
    }

    destination.close();
    http.end();

    if (received != static_cast<uint32_t>(contentLength)) {
        publishStatus(runtime, statusCallback, "OTA stage fail", "short download", nullptr);
        return false;
    }

    if (stagedBytesOut != nullptr) {
        *stagedBytesOut = received;
    }

    char okText[24];
    snprintf(okText, sizeof(okText), "%lu bytes", static_cast<unsigned long>(received));
    publishStatus(runtime, statusCallback, "OTA stage ready", okText, CAPTAIN_OTA_STAGE_FILE);
    return true;
}

bool applyStagedFirmwareFromExternalFlash(Runtime& runtime,
                                          uint32_t stagedBytes,
                                          StatusCallback statusCallback) {
    SerialFlashFile stagedFile = SerialFlash.open(CAPTAIN_OTA_STAGE_FILE);
    if (!stagedFile) {
        publishStatus(runtime, statusCallback, "OTA flash fail", "stage open", nullptr);
        return false;
    }

    if (!Update.begin(stagedBytes)) {
        publishStatus(runtime, statusCallback, "OTA flash fail", "Update.begin", nullptr);
        stagedFile.close();
        return false;
    }

    uint8_t buffer[CAPTAIN_OTA_COPY_BUFFER_BYTES];
    uint32_t flashed = 0;
    uint32_t lastPrintMs = 0;

    while (flashed < stagedBytes) {
        const uint32_t remaining = stagedBytes - flashed;
        const uint32_t chunk = (remaining > sizeof(buffer)) ? sizeof(buffer) : remaining;
        const uint32_t bytesRead = stagedFile.read(buffer, chunk);
        if (bytesRead == 0) {
            Update.abort();
            stagedFile.close();
            publishStatus(runtime, statusCallback, "OTA flash fail", "stage read", nullptr);
            return false;
        }

        const size_t bytesWritten = Update.write(buffer, bytesRead);
        if (bytesWritten != bytesRead) {
            Update.abort();
            stagedFile.close();
            publishStatus(runtime, statusCallback, "OTA flash fail", "write mismatch", nullptr);
            return false;
        }

        flashed += bytesRead;

        const uint32_t now = millis();
        if (now - lastPrintMs >= CAPTAIN_OTA_PROGRESS_PRINT_MS) {
            lastPrintMs = now;
            const uint32_t percent = (flashed * 100U) / stagedBytes;
            char pctText[20];
            snprintf(pctText, sizeof(pctText), "FL %lu%%", static_cast<unsigned long>(percent));
            char bytesText[28];
            snprintf(bytesText,
                     sizeof(bytesText),
                     "%lu/%lu",
                     static_cast<unsigned long>(flashed),
                     static_cast<unsigned long>(stagedBytes));
            publishStatus(runtime, statusCallback, "OTA flashing", pctText, bytesText);
        }
    }

    stagedFile.close();

    if (!Update.end()) {
        publishStatus(runtime, statusCallback, "OTA flash fail", "Update.end", nullptr);
        return false;
    }

    if (!Update.isFinished()) {
        publishStatus(runtime, statusCallback, "OTA flash fail", "not finished", nullptr);
        return false;
    }

    publishStatus(runtime, statusCallback, "OTA success", "rebooting", nullptr);
    return true;
}

}  // namespace

void initialize(Runtime& runtime) {
    runtime.wifiSsid[0] = '\0';
    runtime.wifiPassword[0] = '\0';
    snprintf(runtime.lastStatus, sizeof(runtime.lastStatus), "idle");
    runtime.autostartRequested = CAPTAIN_OTA_AUTOSTART_ON_BOOT;
    runtime.inProgress = false;
    runtime.lastWifiAttemptMs = 0;
    runtime.wifiReadyAnnounced = false;
}

void seedCredentialsFromConfig(Runtime& runtime) {
    if (strcmp(CAPTAIN_WIFI_SSID, "YOUR_WIFI_SSID") == 0) {
        return;
    }
    snprintf(runtime.wifiSsid, sizeof(runtime.wifiSsid), "%s", CAPTAIN_WIFI_SSID);
    snprintf(runtime.wifiPassword, sizeof(runtime.wifiPassword), "%s", CAPTAIN_WIFI_PASSWORD);
}

String configuredDefaultUrl() {
    if (CAPTAIN_OTA_DEFAULT_URL == nullptr) {
        return String();
    }

    String url(CAPTAIN_OTA_DEFAULT_URL);
    url.trim();
    return url;
}

void publishStatus(Runtime& runtime,
                   StatusCallback statusCallback,
                   const char* line1,
                   const char* line2,
                   const char* line3) {
    if (line1 != nullptr) {
        snprintf(runtime.lastStatus, sizeof(runtime.lastStatus), "%s", line1);
    }

    printStatus(line1, line2, line3);
    if (statusCallback != nullptr) {
        statusCallback(line1, line2, line3);
    }
}

void setWifiCredentials(Runtime& runtime, const char* ssid, const char* password) {
    snprintf(runtime.wifiSsid, sizeof(runtime.wifiSsid), "%s", ssid != nullptr ? ssid : "");
    snprintf(runtime.wifiPassword, sizeof(runtime.wifiPassword), "%s", password != nullptr ? password : "");
    runtime.wifiReadyAnnounced = false;
    runtime.lastWifiAttemptMs = 0;
}

bool connectWifiWithTimeout(Runtime& runtime,
                            uint32_t timeoutMs,
                            bool showStatus,
                            StatusCallback statusCallback) {
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }

    if (runtime.wifiSsid[0] == '\0') {
        if (showStatus) {
            publishStatus(runtime, statusCallback, "OTA WiFi missing", "Use: ota wifi", "<ssid> <pass>");
        }
        return false;
    }

    if (showStatus) {
        publishStatus(runtime, statusCallback, "OTA WiFi connect", runtime.wifiSsid, "waiting...");
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(runtime.wifiSsid, runtime.wifiPassword);

    const uint32_t started = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - started < timeoutMs) {
        delay(250);
    }

    if (WiFi.status() != WL_CONNECTED) {
        if (showStatus) {
            publishStatus(runtime, statusCallback, "OTA WiFi failed", runtime.wifiSsid, "timeout");
        }
        return false;
    }

    const IPAddress ip = WiFi.localIP();
    char ipText[24];
    snprintf(ipText, sizeof(ipText), "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
    if (showStatus) {
        publishStatus(runtime, statusCallback, "OTA WiFi ready", runtime.wifiSsid, ipText);
    }
    return true;
}

void maintainWifiConnection(Runtime& runtime, uint32_t nowMs, StatusCallback statusCallback) {
    if (runtime.wifiSsid[0] == '\0' || runtime.inProgress) {
        return;
    }

    if (WiFi.status() == WL_CONNECTED) {
        if (!runtime.wifiReadyAnnounced) {
            runtime.wifiReadyAnnounced = true;
            const IPAddress ip = WiFi.localIP();
            char ipText[24];
            snprintf(ipText, sizeof(ipText), "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
            publishStatus(runtime, statusCallback, "OTA WiFi ready", runtime.wifiSsid, ipText);
        }
        return;
    }

    if (nowMs - runtime.lastWifiAttemptMs < CAPTAIN_WIFI_RECONNECT_INTERVAL_MS) {
        return;
    }

    runtime.lastWifiAttemptMs = nowMs;
    runtime.wifiReadyAnnounced = false;
    connectWifiWithTimeout(runtime, 3500U, true, statusCallback);
}

const char* statusText(const Runtime& runtime) {
    return runtime.lastStatus;
}

bool runStagedOtaFromUrl(Runtime& runtime,
                         const String& url,
                         bool externalFlashReady,
                         StatusCallback statusCallback,
                         StopAudioCallback stopAudioCallback) {
    if (runtime.inProgress) {
        publishStatus(runtime, statusCallback, "OTA busy", nullptr, nullptr);
        return false;
    }

    runtime.inProgress = true;
    if (stopAudioCallback != nullptr) {
        stopAudioCallback();
    }

    if (!connectWifiWithTimeout(runtime, CAPTAIN_OTA_WIFI_TIMEOUT_MS, true, statusCallback)) {
        runtime.inProgress = false;
        return false;
    }

    uint32_t stagedBytes = 0;
    if (!stageFirmwareToExternalFlash(runtime, url, externalFlashReady, statusCallback, &stagedBytes)) {
        runtime.inProgress = false;
        return false;
    }

    const bool flashOk = applyStagedFirmwareFromExternalFlash(runtime, stagedBytes, statusCallback);
    runtime.inProgress = false;
    if (!flashOk) {
        return false;
    }

    delay(1200);
    ESP.restart();
    return true;
}

}  // namespace ota
}  // namespace captain
