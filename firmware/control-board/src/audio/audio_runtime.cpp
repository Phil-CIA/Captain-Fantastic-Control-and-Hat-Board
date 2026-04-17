#include "audio_runtime.h"

#include <AudioFileSource.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2S.h>
#include <FS.h>
#include <SPI.h>
#include <SPIFFS.h>
#include <AudioFileSourceSPIFFS.h>
#include <SerialFlash.h>

#include "audio_i2s_config.h"
#include "external_flash_config.h"

namespace captain {
namespace audio {
namespace {

constexpr float CAPTAIN_AUDIO_MP3_GAIN = 0.12f;

#ifndef CAPTAIN_AUDIO_STRICT_CANONICAL_NAMES
#define CAPTAIN_AUDIO_STRICT_CANONICAL_NAMES 0
#endif

class AudioFileSourceSerialFlashCompat : public AudioFileSource {
   public:
    AudioFileSourceSerialFlashCompat() = default;
    explicit AudioFileSourceSerialFlashCompat(const char* filename) {
        open(filename);
    }

    ~AudioFileSourceSerialFlashCompat() override {
        close();
    }

    bool open(const char* filename) override {
        close();
        if (filename == nullptr || filename[0] == '\0') {
            return false;
        }

        file_ = SerialFlash.open(filename);
        if (!file_) {
            return false;
        }

        sizeBytes_ = file_.size();
        position_ = 0;
        return true;
    }

    uint32_t read(void* data, uint32_t len) override {
        if (!file_ || data == nullptr || len == 0) {
            return 0;
        }

        const uint32_t bytesRead = file_.read(data, len);
        position_ += bytesRead;
        return bytesRead;
    }

    bool seek(int32_t pos, int dir) override {
        if (!file_) {
            return false;
        }

        int32_t target = 0;
        if (dir == SEEK_SET) {
            target = pos;
        } else if (dir == SEEK_CUR) {
            target = static_cast<int32_t>(position_) + pos;
        } else if (dir == SEEK_END) {
            target = static_cast<int32_t>(sizeBytes_) + pos;
        } else {
            return false;
        }

        if (target < 0) {
            target = 0;
        }
        if (static_cast<uint32_t>(target) > sizeBytes_) {
            target = static_cast<int32_t>(sizeBytes_);
        }

        file_.seek(static_cast<uint32_t>(target));
        position_ = static_cast<uint32_t>(target);
        return true;
    }

    bool close() override {
        if (file_) {
            file_.close();
        }

        position_ = 0;
        sizeBytes_ = 0;
        return true;
    }

    bool isOpen() override {
        return static_cast<bool>(file_);
    }

    uint32_t getSize() override {
        return sizeBytes_;
    }

    uint32_t getPos() override {
        return position_;
    }

   private:
    SerialFlashFile file_;
    uint32_t position_ = 0;
    uint32_t sizeBytes_ = 0;
};

struct AudioFileChoice {
    const char* spiffsPath;
    const char* externalName;
    bool aliasName;
};

const char* eventName(Event event) {
    switch (event) {
        case Event::Startup:
            return "startup";
        case Event::Attract:
            return "attract";
        case Event::GameStart:
            return "start";
        case Event::Bonus:
            return "bonus";
        case Event::GameOver:
            return "gameover";
        case Event::HighScore:
            return "hiscore";
        default:
            return "unknown";
    }
}

const AudioFileChoice* selectEventMp3(const Runtime& runtime, Event event, bool* fromExternal) {
    static const AudioFileChoice startupFiles[] = {
        {"/startup.mp3", "startup.mp3", false},
        {"/rocket_man.mp3", "rocket.mp3", true},
        {"/crocodile_rock.mp3", "crock.mp3", true}
    };
    static const AudioFileChoice attractFiles[] = {
        {"/attract.mp3", "attract.mp3", false},
        {"/rocket_man.mp3", "rocket.mp3", true}
    };
    static const AudioFileChoice gameStartFiles[] = {
        {"/start.mp3", "start.mp3", false},
        {"/crocodile_rock.mp3", "crock.mp3", true}
    };
    static const AudioFileChoice bonusFiles[] = {
        {"/bonus.mp3", "bonus.mp3", false},
        {"/crocodile_rock.mp3", "crock.mp3", true}
    };
    static const AudioFileChoice gameOverFiles[] = {
        {"/game_over.mp3", "gmovr.mp3", false},
        {"/rocket_man.mp3", "rocket.mp3", true}
    };
    static const AudioFileChoice highScoreFiles[] = {
        {"/high_score.mp3", "hiscore.mp3", false},
        {"/rocket_man.mp3", "rocket.mp3", true}
    };

    const AudioFileChoice* candidateFiles = nullptr;
    size_t candidateCount = 0;

    switch (event) {
        case Event::Startup:
            candidateFiles = startupFiles;
            candidateCount = sizeof(startupFiles) / sizeof(startupFiles[0]);
            break;
        case Event::Attract:
            candidateFiles = attractFiles;
            candidateCount = sizeof(attractFiles) / sizeof(attractFiles[0]);
            break;
        case Event::GameStart:
            candidateFiles = gameStartFiles;
            candidateCount = sizeof(gameStartFiles) / sizeof(gameStartFiles[0]);
            break;
        case Event::Bonus:
            candidateFiles = bonusFiles;
            candidateCount = sizeof(bonusFiles) / sizeof(bonusFiles[0]);
            break;
        case Event::GameOver:
            candidateFiles = gameOverFiles;
            candidateCount = sizeof(gameOverFiles) / sizeof(gameOverFiles[0]);
            break;
        case Event::HighScore:
            candidateFiles = highScoreFiles;
            candidateCount = sizeof(highScoreFiles) / sizeof(highScoreFiles[0]);
            break;
        default:
            return nullptr;
    }

    for (size_t index = 0; index < candidateCount; index++) {
        const AudioFileChoice* candidate = &candidateFiles[index];

        if (CAPTAIN_AUDIO_STRICT_CANONICAL_NAMES && candidate->aliasName) {
            continue;
        }

        if (runtime.externalFlashReady && SerialFlash.exists(candidate->externalName)) {
            if (fromExternal != nullptr) {
                *fromExternal = true;
            }
            return candidate;
        }

        if (runtime.internalSpiffsReady && SPIFFS.exists(candidate->spiffsPath)) {
            if (fromExternal != nullptr) {
                *fromExternal = false;
            }
            return candidate;
        }
    }

    return nullptr;
}

bool copySpiffsFileToExternal(Runtime& runtime, const char* spiffsPath, const char* externalName) {
    if (!runtime.internalSpiffsReady || !runtime.externalFlashReady) {
        return false;
    }
    if (!SPIFFS.exists(spiffsPath)) {
        return false;
    }

    File source = SPIFFS.open(spiffsPath, "r");
    if (!source) {
        Serial.printf("Audio MP3 extflash: open failed for %s\n", spiffsPath);
        return false;
    }

    const uint32_t sourceSize = static_cast<uint32_t>(source.size());
    if (sourceSize == 0U) {
        source.close();
        return false;
    }

    if (SerialFlash.exists(externalName)) {
        SerialFlashFile existing = SerialFlash.open(externalName);
        if (existing) {
            const uint32_t existingSize = existing.size();
            existing.close();
            if (existingSize == sourceSize) {
                source.close();
                return true;
            }
        }
        SerialFlash.remove(externalName);
    }

    if (!SerialFlash.createErasable(externalName, sourceSize)) {
        if (!runtime.externalFlashFormatAttempted) {
            runtime.externalFlashFormatAttempted = true;
            Serial.println("Audio MP3 extflash: create failed, attempting one-time chip erase...");
            SerialFlash.eraseAll();
            const uint32_t eraseStartedMs = millis();
            while (!SerialFlash.ready()) {
                if (millis() - eraseStartedMs > 30000U) {
                    Serial.println("Audio MP3 extflash: erase timeout; keeping SPIFFS fallback");
                    source.close();
                    return false;
                }
                delay(25);
            }

            if (!SerialFlash.createErasable(externalName, sourceSize)) {
                Serial.printf("Audio MP3 extflash: create failed after erase for %s (%lu bytes)\n",
                              externalName,
                              static_cast<unsigned long>(sourceSize));
                source.close();
                return false;
            }
        } else {
            Serial.printf("Audio MP3 extflash: create failed for %s (%lu bytes)\n",
                          externalName,
                          static_cast<unsigned long>(sourceSize));
            source.close();
            return false;
        }
    }

    SerialFlashFile destination = SerialFlash.open(externalName);
    if (!destination) {
        Serial.printf("Audio MP3 extflash: open destination failed for %s\n", externalName);
        source.close();
        return false;
    }

    uint8_t buffer[256];
    uint32_t written = 0;
    while (source.available()) {
        const int bytesRead = source.read(buffer, sizeof(buffer));
        if (bytesRead <= 0) {
            break;
        }

        const uint32_t bytesWritten = destination.write(buffer, static_cast<uint32_t>(bytesRead));
        if (bytesWritten != static_cast<uint32_t>(bytesRead)) {
            Serial.printf("Audio MP3 extflash: write failed for %s\n", externalName);
            destination.close();
            source.close();
            return false;
        }

        written += bytesWritten;
    }

    destination.close();
    source.close();

    if (written != sourceSize) {
        Serial.printf("Audio MP3 extflash: short copy for %s (%lu/%lu)\n",
                      externalName,
                      static_cast<unsigned long>(written),
                      static_cast<unsigned long>(sourceSize));
        return false;
    }

    Serial.printf("Audio MP3 extflash: synced %s -> %s (%lu bytes)\n",
                  spiffsPath,
                  externalName,
                  static_cast<unsigned long>(sourceSize));
    return true;
}

void syncAudioAssetsToExternalFlash(Runtime& runtime) {
    static const AudioFileChoice filesToSync[] = {
        {"/startup.mp3", "startup.mp3"},
        {"/attract.mp3", "attract.mp3"},
        {"/start.mp3", "start.mp3"},
        {"/bonus.mp3", "bonus.mp3"},
        {"/game_over.mp3", "gmovr.mp3"},
        {"/high_score.mp3", "hiscore.mp3"},
        {"/rocket_man.mp3", "rocket.mp3"},
        {"/crocodile_rock.mp3", "crock.mp3"}
    };

    if (!runtime.internalSpiffsReady || !runtime.externalFlashReady) {
        return;
    }

    for (const AudioFileChoice& file : filesToSync) {
        copySpiffsFileToExternal(runtime, file.spiffsPath, file.externalName);
    }
}

void initExternalFlashStorage(Runtime& runtime, uint32_t serialFlashSpiHz) {
    pinMode(CAPTAIN_FLASH_CS_PIN, OUTPUT);
    digitalWrite(CAPTAIN_FLASH_CS_PIN, HIGH);

    SPI.begin(CAPTAIN_FLASH_SCK_PIN, CAPTAIN_FLASH_MISO_PIN, CAPTAIN_FLASH_MOSI_PIN, CAPTAIN_FLASH_CS_PIN);
    if (!SerialFlash.begin(CAPTAIN_FLASH_CS_PIN)) {
        Serial.println("Audio MP3 extflash: SerialFlash begin failed; using SPIFFS");
        return;
    }

    uint8_t jedecId[5] = {0, 0, 0, 0, 0};
    SerialFlash.readID(jedecId);
    Serial.printf("Audio MP3 extflash JEDEC: %02X %02X %02X\n", jedecId[0], jedecId[1], jedecId[2]);
    Serial.printf("Audio MP3 extflash SPI clock: %lu Hz\n", static_cast<unsigned long>(serialFlashSpiHz));

    runtime.externalFlashReady = true;
    Serial.println("Audio MP3 extflash: mounted (W25Q128 path)");
}

void initAudioStorage(Runtime& runtime, uint32_t serialFlashSpiHz) {
    if (!SPIFFS.begin(true)) {
        Serial.println("Audio MP3: SPIFFS mount failed");
    } else {
        runtime.internalSpiffsReady = true;
        Serial.println("Audio MP3: SPIFFS mounted");
    }

    initExternalFlashStorage(runtime, serialFlashSpiHz);
    syncAudioAssetsToExternalFlash(runtime);
}

void writeAudioTone(Runtime& runtime, uint16_t frequencyHz, uint16_t durationMs) {
    if (!runtime.i2sReady || frequencyHz == 0 || durationMs == 0) {
        return;
    }

    const uint32_t frames = (CAPTAIN_AUDIO_SAMPLE_RATE * durationMs) / 1000U;
    if (frames == 0) {
        return;
    }

    const uint32_t phaseStep = static_cast<uint32_t>((static_cast<uint64_t>(frequencyHz) << 32U) /
                                                     CAPTAIN_AUDIO_SAMPLE_RATE);
    uint32_t phase = 0;
    int16_t stereoFrame[2] = {0, 0};

    for (uint32_t frame = 0; frame < frames; frame++) {
        const int16_t sample = (phase & 0x80000000U)
                                   ? static_cast<int16_t>(CAPTAIN_AUDIO_TEST_AMPLITUDE)
                                   : static_cast<int16_t>(-static_cast<int16_t>(CAPTAIN_AUDIO_TEST_AMPLITUDE));
        phase += phaseStep;

        stereoFrame[0] = sample;
        stereoFrame[1] = sample;

        size_t bytesWritten = 0;
        i2s_write(CAPTAIN_AUDIO_I2S_PORT,
                  stereoFrame,
                  sizeof(stereoFrame),
                  &bytesWritten,
                  portMAX_DELAY);
    }
}

void writeAudioSilence(Runtime& runtime, uint16_t durationMs) {
    if (!runtime.i2sReady || durationMs == 0) {
        return;
    }

    const uint32_t frames = (CAPTAIN_AUDIO_SAMPLE_RATE * durationMs) / 1000U;
    int16_t stereoFrame[2] = {0, 0};
    for (uint32_t frame = 0; frame < frames; frame++) {
        size_t bytesWritten = 0;
        i2s_write(CAPTAIN_AUDIO_I2S_PORT,
                  stereoFrame,
                  sizeof(stereoFrame),
                  &bytesWritten,
                  portMAX_DELAY);
    }
}

}  // namespace

void initialize(Runtime& runtime) {
    runtime.lastAudioDiagnosticMs = 0;
    runtime.masterGain = CAPTAIN_AUDIO_MP3_GAIN;
    runtime.i2sReady = false;
    runtime.diagnosticFlip = false;
    runtime.internalSpiffsReady = false;
    runtime.externalFlashReady = false;
    runtime.externalFlashFormatAttempted = false;
    runtime.mp3Ready = false;
    runtime.mp3Playing = false;
    runtime.mp3Loop = false;
    runtime.mp3FromExternal = false;
    runtime.mp3 = nullptr;
    runtime.mp3File = nullptr;
    runtime.mp3Output = nullptr;
    runtime.mp3CurrentFile[0] = '\0';
}

void stop(Runtime& runtime) {
    if (runtime.mp3 != nullptr && runtime.mp3->isRunning()) {
        runtime.mp3->stop();
    }

    delete runtime.mp3;
    runtime.mp3 = nullptr;

    if (runtime.mp3File != nullptr) {
        runtime.mp3File->close();
        delete runtime.mp3File;
        runtime.mp3File = nullptr;
    }

    delete runtime.mp3Output;
    runtime.mp3Output = nullptr;
    runtime.mp3Playing = false;
    runtime.mp3Loop = false;
    runtime.mp3FromExternal = false;
    runtime.mp3CurrentFile[0] = '\0';
}

bool playEvent(Runtime& runtime, Event event, bool loop) {
    if (!runtime.internalSpiffsReady && !runtime.externalFlashReady) {
        return false;
    }

    bool playFromExternal = false;
    const AudioFileChoice* eventFile = selectEventMp3(runtime, event, &playFromExternal);
    if (eventFile == nullptr) {
        Serial.printf("Audio MP3: no file found for %s; using tone fallback\n", eventName(event));
        return false;
    }

    stop(runtime);

    const char* selectedName = playFromExternal ? eventFile->externalName : eventFile->spiffsPath;
    if (playFromExternal) {
        runtime.mp3File = new AudioFileSourceSerialFlashCompat(selectedName);
    } else {
        runtime.mp3File = new AudioFileSourceSPIFFS(selectedName);
    }
    runtime.mp3Output = new AudioOutputI2S();
    runtime.mp3 = new AudioGeneratorMP3();

    if (runtime.mp3File == nullptr || !runtime.mp3File->isOpen() || runtime.mp3Output == nullptr || runtime.mp3 == nullptr) {
        Serial.println("Audio MP3: allocation failed; using tone fallback");
        stop(runtime);
        return false;
    }

    const int bclkPin = CAPTAIN_AUDIO_SWAP_BCLK_LRCK_FOR_TEST ? CAPTAIN_AUDIO_LRCK_PIN : CAPTAIN_AUDIO_BCLK_PIN;
    const int lrckPin = CAPTAIN_AUDIO_SWAP_BCLK_LRCK_FOR_TEST ? CAPTAIN_AUDIO_BCLK_PIN : CAPTAIN_AUDIO_LRCK_PIN;
    runtime.mp3Output->SetPinout(bclkPin, lrckPin, CAPTAIN_AUDIO_DIN_PIN);
    runtime.mp3Output->SetGain(runtime.masterGain);

    if (!runtime.mp3->begin(runtime.mp3File, runtime.mp3Output)) {
        Serial.printf("Audio MP3: failed to start %s for %s\n", selectedName, eventName(event));
        stop(runtime);
        return false;
    }

    runtime.mp3Ready = true;
    runtime.mp3Playing = true;
    runtime.mp3Loop = loop;
    runtime.mp3FromExternal = playFromExternal;
    snprintf(runtime.mp3CurrentFile, sizeof(runtime.mp3CurrentFile), "%s", selectedName);
    Serial.printf("Audio MP3: playing %s (%s)%s\n",
                  selectedName,
                  eventName(event),
                  loop ? " [loop]" : "");
    Serial.printf("Audio MP3: source=%s\n", playFromExternal ? "W25Q128" : "SPIFFS");
    return true;
}

void initPath(Runtime& runtime, uint32_t serialFlashSpiHz) {
    initAudioStorage(runtime, serialFlashSpiHz);
    if (playEvent(runtime, Event::Startup, false)) {
        return;
    }

    const int bclkPin = CAPTAIN_AUDIO_SWAP_BCLK_LRCK_FOR_TEST ? CAPTAIN_AUDIO_LRCK_PIN : CAPTAIN_AUDIO_BCLK_PIN;
    const int lrckPin = CAPTAIN_AUDIO_SWAP_BCLK_LRCK_FOR_TEST ? CAPTAIN_AUDIO_BCLK_PIN : CAPTAIN_AUDIO_LRCK_PIN;

    Serial.printf("Audio I2S pins: DIN=%u BCLK=%d LRCK=%d\n",
                  CAPTAIN_AUDIO_DIN_PIN,
                  bclkPin,
                  lrckPin);

    const i2s_config_t i2sConfig = {
        .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = static_cast<int>(CAPTAIN_AUDIO_SAMPLE_RATE),
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 8,
        .dma_buf_len = 256,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pinConfig = {};
    pinConfig.bck_io_num = bclkPin;
    pinConfig.ws_io_num = lrckPin;
    pinConfig.data_out_num = CAPTAIN_AUDIO_DIN_PIN;
    pinConfig.data_in_num = I2S_PIN_NO_CHANGE;

    const esp_err_t installResult = i2s_driver_install(CAPTAIN_AUDIO_I2S_PORT, &i2sConfig, 0, nullptr);
    if (installResult != ESP_OK) {
        Serial.printf("Audio I2S driver install failed: %d\n", static_cast<int>(installResult));
        return;
    }

    const esp_err_t pinResult = i2s_set_pin(CAPTAIN_AUDIO_I2S_PORT, &pinConfig);
    if (pinResult != ESP_OK) {
        Serial.printf("Audio I2S pin config failed: %d\n", static_cast<int>(pinResult));
        i2s_driver_uninstall(CAPTAIN_AUDIO_I2S_PORT);
        return;
    }

    i2s_zero_dma_buffer(CAPTAIN_AUDIO_I2S_PORT);
    runtime.i2sReady = true;
    Serial.println("Audio I2S initialized (MAX98357 path)");

    if (!CAPTAIN_AUDIO_STARTUP_TEST_ENABLED) {
        return;
    }

    for (uint8_t pass = 0; pass < CAPTAIN_AUDIO_STARTUP_TEST_REPEATS; pass++) {
        writeAudioTone(runtime, (pass % 2U) == 0U ? 880U : 1320U, CAPTAIN_AUDIO_STARTUP_TONE_MS);
        writeAudioSilence(runtime, CAPTAIN_AUDIO_STARTUP_GAP_MS);
    }
}

void updateDiagnostic(Runtime& runtime, uint32_t nowMs) {
    if (runtime.mp3Playing) {
        return;
    }

    if (!runtime.i2sReady || !CAPTAIN_AUDIO_CONTINUOUS_DIAGNOSTIC) {
        return;
    }

    if (nowMs - runtime.lastAudioDiagnosticMs < CAPTAIN_AUDIO_CONTINUOUS_INTERVAL_MS) {
        return;
    }

    runtime.lastAudioDiagnosticMs = nowMs;
    runtime.diagnosticFlip = !runtime.diagnosticFlip;
    writeAudioTone(runtime, runtime.diagnosticFlip ? 760U : 1180U, CAPTAIN_AUDIO_CONTINUOUS_TONE_MS);
}

void updatePlayback(Runtime& runtime) {
    if (!runtime.mp3Playing || runtime.mp3 == nullptr) {
        return;
    }

    if (runtime.mp3->isRunning()) {
        if (runtime.mp3->loop()) {
            return;
        }
    }

    if (runtime.mp3Loop && runtime.mp3CurrentFile[0] != '\0') {
        Serial.printf("Audio MP3: restarting loop track %s\n", runtime.mp3CurrentFile);
        runtime.mp3->stop();
        runtime.mp3File->close();
        delete runtime.mp3File;
        if (runtime.mp3FromExternal) {
            runtime.mp3File = new AudioFileSourceSerialFlashCompat(runtime.mp3CurrentFile);
        } else {
            runtime.mp3File = new AudioFileSourceSPIFFS(runtime.mp3CurrentFile);
        }
        if (runtime.mp3File != nullptr && runtime.mp3->begin(runtime.mp3File, runtime.mp3Output)) {
            return;
        }
        Serial.println("Audio MP3: loop restart failed; stopping playback");
    }

    Serial.println("Audio MP3: playback complete");
    stop(runtime);
}

bool isExternalFlashReady(const Runtime& runtime) {
    return runtime.externalFlashReady;
}

bool isInternalSpiffsReady(const Runtime& runtime) {
    return runtime.internalSpiffsReady;
}

void setMasterGain(Runtime& runtime, float gain) {
    if (gain < 0.0f) {
        gain = 0.0f;
    }
    if (gain > 1.0f) {
        gain = 1.0f;
    }

    runtime.masterGain = gain;
    if (runtime.mp3Output != nullptr) {
        runtime.mp3Output->SetGain(runtime.masterGain);
    }
}

float getMasterGain(const Runtime& runtime) {
    return runtime.masterGain;
}

bool isStrictAssetNamingMode() {
    return CAPTAIN_AUDIO_STRICT_CANONICAL_NAMES != 0;
}

}  // namespace audio
}  // namespace captain
