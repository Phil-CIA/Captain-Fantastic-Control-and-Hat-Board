#ifndef AUDIO_RUNTIME_H
#define AUDIO_RUNTIME_H

#include <Arduino.h>

class AudioGeneratorMP3;
class AudioFileSource;
class AudioOutputI2S;

namespace captain {
namespace audio {

enum class Event {
    Startup,
    Attract,
    GameStart,
    Bonus,
    GameOver,
    HighScore
};

struct Runtime {
    uint32_t lastAudioDiagnosticMs;
    bool i2sReady;
    bool diagnosticFlip;
    bool internalSpiffsReady;
    bool externalFlashReady;
    bool externalFlashFormatAttempted;
    bool mp3Ready;
    bool mp3Playing;
    bool mp3Loop;
    bool mp3FromExternal;
    AudioGeneratorMP3* mp3;
    AudioFileSource* mp3File;
    AudioOutputI2S* mp3Output;
    char mp3CurrentFile[40];
};

void initialize(Runtime& runtime);
void initPath(Runtime& runtime, uint32_t serialFlashSpiHz);
void stop(Runtime& runtime);
bool playEvent(Runtime& runtime, Event event, bool loop);
void updatePlayback(Runtime& runtime);
void updateDiagnostic(Runtime& runtime, uint32_t nowMs);

bool isExternalFlashReady(const Runtime& runtime);
bool isInternalSpiffsReady(const Runtime& runtime);

}  // namespace audio
}  // namespace captain

#endif
