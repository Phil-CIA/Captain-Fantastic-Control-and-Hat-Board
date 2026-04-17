#include "service_menu_runtime.h"

namespace captain {
namespace service {
namespace {

enum MenuItem : uint8_t {
    Volume = 0,
    SwitchTest,
    CoilTest,
    AudioTest,
    Count
};

const char* menuItemName(uint8_t index) {
    switch (index) {
        case Volume:
            return "Volume";
        case SwitchTest:
            return "Switch test";
        case CoilTest:
            return "Coil test";
        case AudioTest:
            return "Audio test";
        default:
            return "Unknown";
    }
}

uint8_t gainToPercent(float gain) {
    if (gain <= 0.0f) {
        return 0;
    }
    if (gain >= 1.0f) {
        return 100;
    }
    return static_cast<uint8_t>(gain * 100.0f + 0.5f);
}

void printHelp() {
    Serial.println("Service commands: service menu | service status | service next | service prev | service set <0-100> | service run [switch|coil|audio] | service save | service cancel | service exit");
}

void printMenu(const Runtime& runtime) {
    Serial.println("Service menu (flat list)");
    for (uint8_t index = 0; index < Count; index++) {
        Serial.printf("  %c %u) %s", runtime.selectedIndex == index ? '>' : ' ', index + 1, menuItemName(index));
        if (index == Volume) {
            Serial.printf(" [%u%% pending, %u%% saved]", runtime.pendingVolumePercent, runtime.committedVolumePercent);
        }
        Serial.println();
    }

    if (runtime.dirty) {
        Serial.println("  pending changes: YES (run 'service save' to commit)");
    }
}

void publishStatusToOled(captain::input::overlay::OledStatusWriter oledStatusWriter,
                         const Runtime& runtime,
                         const char* line3) {
    if (oledStatusWriter == nullptr) {
        return;
    }

    char line2[22] = {};
    snprintf(line2, sizeof(line2), "%s %u%%",
             runtime.selectedIndex == Volume ? "Volume" : menuItemName(runtime.selectedIndex),
             runtime.pendingVolumePercent);
    oledStatusWriter("Service menu", line2, line3);
}

void runAction(Runtime& runtime,
               captain::audio::Runtime& audioRuntime,
               captain::input::overlay::OledStatusWriter oledStatusWriter,
               const String& actionArg) {
    if (actionArg == "switch" || runtime.selectedIndex == SwitchTest) {
        Serial.println("Service action: switch test placeholder (wire to matrix switch diagnostics)");
        publishStatusToOled(oledStatusWriter, runtime, "Switch test");
        return;
    }

    if (actionArg == "coil" || runtime.selectedIndex == CoilTest) {
        Serial.println("Service action: coil test placeholder (wire to driver safety module)");
        publishStatusToOled(oledStatusWriter, runtime, "Coil test");
        return;
    }

    if (actionArg == "audio" || runtime.selectedIndex == AudioTest) {
        Serial.println("Service action: audio test startup clip");
        captain::audio::playEvent(audioRuntime, captain::audio::Event::Startup, false);
        publishStatusToOled(oledStatusWriter, runtime, "Audio test");
        return;
    }

    if (runtime.selectedIndex == Volume) {
        Serial.printf("Service volume preview: %u%% (run 'service save' to commit)\n", runtime.pendingVolumePercent);
        publishStatusToOled(oledStatusWriter, runtime, "Volume preview");
        return;
    }

    Serial.println("Service run target not recognized");
}

}  // namespace

void initialize(Runtime& runtime, float startingGain) {
    runtime.active = false;
    runtime.dirty = false;
    runtime.selectedIndex = Volume;
    runtime.committedVolumePercent = gainToPercent(startingGain);
    runtime.pendingVolumePercent = runtime.committedVolumePercent;
}

bool handleCommand(const String& command,
                   const String& commandRaw,
                   Runtime& runtime,
                   captain::audio::Runtime& audioRuntime,
                   captain::input::overlay::OledStatusWriter oledStatusWriter) {
    (void)commandRaw;

    if (command == "service help") {
        printHelp();
        return true;
    }

    if (command == "service menu") {
        runtime.active = true;
        printMenu(runtime);
        publishStatusToOled(oledStatusWriter, runtime, "Menu open");
        return true;
    }

    if (command == "service status") {
        Serial.printf("Service menu active: %s\n", runtime.active ? "yes" : "no");
        Serial.printf("Service selected item: %s\n", menuItemName(runtime.selectedIndex));
        Serial.printf("Service volume pending/saved: %u%% / %u%%\n",
                      runtime.pendingVolumePercent,
                      runtime.committedVolumePercent);
        Serial.printf("Service dirty: %s\n", runtime.dirty ? "yes" : "no");
        return true;
    }

    if (command == "service exit") {
        runtime.active = false;
        Serial.println("Service menu closed");
        publishStatusToOled(oledStatusWriter, runtime, "Menu closed");
        return true;
    }

    if (command == "service next") {
        runtime.selectedIndex = static_cast<uint8_t>((runtime.selectedIndex + 1) % Count);
        printMenu(runtime);
        publishStatusToOled(oledStatusWriter, runtime, "Next item");
        return true;
    }

    if (command == "service prev") {
        runtime.selectedIndex = static_cast<uint8_t>((runtime.selectedIndex + Count - 1) % Count);
        printMenu(runtime);
        publishStatusToOled(oledStatusWriter, runtime, "Prev item");
        return true;
    }

    if (command.startsWith("service set ")) {
        const int value = command.substring(strlen("service set ")).toInt();
        if (value < 0 || value > 100) {
            Serial.println("Service set expects 0-100");
            return true;
        }

        runtime.selectedIndex = Volume;
        runtime.pendingVolumePercent = static_cast<uint8_t>(value);
        runtime.dirty = (runtime.pendingVolumePercent != runtime.committedVolumePercent);
        Serial.printf("Service pending volume set to %u%%\n", runtime.pendingVolumePercent);
        if (runtime.dirty) {
            Serial.println("Service pending changes exist; run 'service save' to commit");
        }
        publishStatusToOled(oledStatusWriter, runtime, runtime.dirty ? "Pending save" : "No changes");
        return true;
    }

    if (command == "service save") {
        runtime.committedVolumePercent = runtime.pendingVolumePercent;
        runtime.dirty = false;

        captain::audio::setMasterGain(audioRuntime, static_cast<float>(runtime.committedVolumePercent) / 100.0f);
        Serial.printf("Service saved volume: %u%%\n", runtime.committedVolumePercent);
        Serial.println("Persistence TODO: commit to NVS in persistence module");
        publishStatusToOled(oledStatusWriter, runtime, "Saved");
        return true;
    }

    if (command == "service cancel") {
        runtime.pendingVolumePercent = runtime.committedVolumePercent;
        runtime.dirty = false;
        Serial.printf("Service reverted pending values to saved state (%u%%)\n", runtime.committedVolumePercent);
        publishStatusToOled(oledStatusWriter, runtime, "Canceled");
        return true;
    }

    if (command == "service run") {
        runAction(runtime, audioRuntime, oledStatusWriter, "");
        return true;
    }

    if (command.startsWith("service run ")) {
        const String target = command.substring(strlen("service run "));
        runAction(runtime, audioRuntime, oledStatusWriter, target);
        return true;
    }

    return false;
}

}  // namespace service
}  // namespace captain