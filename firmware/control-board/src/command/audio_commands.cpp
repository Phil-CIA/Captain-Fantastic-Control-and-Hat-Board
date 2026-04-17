#include "audio_commands.h"

namespace captain {
namespace command {

bool handleAudioCommand(const String& command, CommandContext& context) {
    if (command == "mp3 storage") {
        Serial.printf("Audio storage: SPIFFS=%s, W25Q128=%s\n",
                      captain::audio::isInternalSpiffsReady(context.audioRuntime) ? "ready" : "down",
                      captain::audio::isExternalFlashReady(context.audioRuntime) ? "ready" : "down");
        Serial.printf("Audio policy: strict-canonical-names=%s\n",
                      captain::audio::isStrictAssetNamingMode() ? "yes" : "no");
        Serial.printf("Audio gain: %.2f\n", captain::audio::getMasterGain(context.audioRuntime));
        return true;
    }

    if (command == "mp3 volume") {
        Serial.printf("Audio gain: %.2f\n", captain::audio::getMasterGain(context.audioRuntime));
        return true;
    }

    if (command.startsWith("mp3 volume ")) {
        const int percent = command.substring(strlen("mp3 volume ")).toInt();
        if (percent < 0 || percent > 100) {
            Serial.println("Audio gain expects 0-100");
            return true;
        }

        captain::audio::setMasterGain(context.audioRuntime, static_cast<float>(percent) / 100.0f);
        Serial.printf("Audio gain set to %d%%\n", percent);
        return true;
    }

    if (command == "mp3 stop") {
        captain::audio::stop(context.audioRuntime);
        Serial.println("Audio MP3: stopped by command");
        return true;
    }

    if (command == "mp3 startup") {
        captain::audio::playEvent(context.audioRuntime, captain::audio::Event::Startup, false);
        return true;
    }

    if (command == "mp3 attract") {
        captain::audio::playEvent(context.audioRuntime, captain::audio::Event::Attract, true);
        return true;
    }

    if (command == "mp3 start") {
        captain::audio::playEvent(context.audioRuntime, captain::audio::Event::GameStart, false);
        return true;
    }

    if (command == "mp3 bonus") {
        captain::audio::playEvent(context.audioRuntime, captain::audio::Event::Bonus, false);
        return true;
    }

    if (command == "mp3 gameover") {
        captain::audio::playEvent(context.audioRuntime, captain::audio::Event::GameOver, false);
        return true;
    }

    if (command == "mp3 hiscore") {
        captain::audio::playEvent(context.audioRuntime, captain::audio::Event::HighScore, false);
        return true;
    }

    return false;
}

}  // namespace command
}  // namespace captain