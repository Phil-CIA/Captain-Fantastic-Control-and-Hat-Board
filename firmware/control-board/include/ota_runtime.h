#ifndef OTA_RUNTIME_H
#define OTA_RUNTIME_H

#include <Arduino.h>

namespace captain {
namespace ota {

using StatusCallback = void (*)(const char* line1, const char* line2, const char* line3);
using StopAudioCallback = void (*)();

struct Runtime {
    char wifiSsid[33];
    char wifiPassword[65];
    char lastStatus[80];
    bool autostartRequested;
    bool inProgress;
    uint32_t lastWifiAttemptMs;
    bool wifiReadyAnnounced;
};

void initialize(Runtime& runtime);
void seedCredentialsFromConfig(Runtime& runtime);
String configuredDefaultUrl();

void publishStatus(Runtime& runtime,
                   StatusCallback statusCallback,
                   const char* line1,
                   const char* line2 = nullptr,
                   const char* line3 = nullptr);

void setWifiCredentials(Runtime& runtime, const char* ssid, const char* password);
bool connectWifiWithTimeout(Runtime& runtime,
                            uint32_t timeoutMs,
                            bool showStatus,
                            StatusCallback statusCallback);
void maintainWifiConnection(Runtime& runtime, uint32_t nowMs, StatusCallback statusCallback);

const char* statusText(const Runtime& runtime);
bool runStagedOtaFromUrl(Runtime& runtime,
                         const String& url,
                         bool externalFlashReady,
                         StatusCallback statusCallback,
                         StopAudioCallback stopAudioCallback);

}  // namespace ota
}  // namespace captain

#endif
