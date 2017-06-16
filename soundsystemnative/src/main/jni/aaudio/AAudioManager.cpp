#include "AAudioManager.h"

aaudio_data_callback_result_t dataCallback(
        AAudioStream *stream,
        void *userData,
        void *audioData,
        int32_t numFrames) {

    assert(userData && audioData);
    AAudioEngine *eng = reinterpret_cast<AAudioEngine *>(userData);
    assert(stream == eng->playStream_);

    // Tuning the buffer size for low latency...
    int32_t underRun = AAudioStream_getXRunCount(eng->playStream_);
    if (underRun > eng->underRunCount_) {
        /*
         * Underrun happened since last callback:
         * try to increase the buffer size.
         */
        eng->underRunCount_ = underRun;

        aaudio_result_t actSize = AAudioStream_setBufferSizeInFrames(
                stream, eng->bufSizeInFrames_ + eng->framesPerBurst_);
        if (actSize > 0) {
            eng->bufSizeInFrames_ = actSize;
        } else {
            LOGE("[AAudioManager] Output stream buffer tuning error: %s",
                 AAudio_convertResultToText(actSize));
        }
    }

    int32_t samplesPerFrame = eng->sampleChannels_;
    AUDIO_HARDWARE_SAMPLE_TYPE *bufferDest = static_cast<AUDIO_HARDWARE_SAMPLE_TYPE *>(audioData);

    if (eng->playAudio_) {
        PrintAudioStreamInfo(engine.playStream_);

        aaudio_stream_state_t aaudioStreamState = AAudioStream_getState(eng->playStream_);
        LOGV("[AAudioManager] currentState=%d %s", aaudioStreamState,
             AAudio_convertStreamStateToText(aaudioStreamState));
//        if (aaudioStreamState == AAUDIO_STREAM_STATE_STARTING) {
//            memset(static_cast<AUDIO_HARDWARE_SAMPLE_TYPE *>(audioData), 0,
//                   sizeof(AUDIO_HARDWARE_SAMPLE_TYPE) * samplesPerFrame * numFrames);
//            return AAUDIO_CALLBACK_RESULT_CONTINUE;
//        }

        int32_t framesPerBurst = AAudioStream_getFramesPerBurst(eng->playStream_);
        int32_t channelCount = AAudioStream_getSamplesPerFrame(eng->playStream_);

        LOGI("[AAudioManager] channel count %d", channelCount);
        LOGI("[AAudioManager] framesPerBurst %d", framesPerBurst);

        int numberElements = numFrames * channelCount;
        size_t sizeBuffer = numberElements * sizeof(AUDIO_HARDWARE_SAMPLE_TYPE);
        aaudio_result_t framesWritten;

        if (eng->playAudio_ && eng->playPosition_ < eng->soundSystem_->getTotalNumberFrames()) {

            // End of track
            if (eng->playPosition_ > eng->soundSystem_->getTotalNumberFrames() - numberElements) {
                sizeBuffer = (eng->soundSystem_->getTotalNumberFrames() - eng->playPosition_) *
                             sizeof(AUDIO_HARDWARE_SAMPLE_TYPE);
            }
            memmove(bufferDest, eng->soundSystem_->getExtractedData() + eng->playPosition_,
                    sizeBuffer);
            eng->playPosition_ += numberElements;
        } else {
            memset(bufferDest, 0, sizeBuffer);
        }
    } else {
        memset(static_cast<AUDIO_HARDWARE_SAMPLE_TYPE *>(audioData), 0,
               sizeof(AUDIO_HARDWARE_SAMPLE_TYPE) * samplesPerFrame * numFrames);
    }

    return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

/*
 * Create sample engine and put application into started state:
 * audio is already rendering -- rendering silent audio.
 */
bool AAudioManager::createEngine(SoundSystem *soundSystem) {
    memset(&engine, 0, sizeof(engine));

    engine.soundSystem_ = soundSystem;
    engine.sampleChannels_ = AUDIO_SAMPLE_CHANNELS;
#ifdef FLOAT_PLAYER
    engine.sampleFormat_ = AAUDIO_FORMAT_PCM_FLOAT;
#else
    engine.sampleFormat_ = AAUDIO_FORMAT_PCM_I16;
#endif
    engine.bitsPerSample_ = SampleFormatToBpp(engine.sampleFormat_);

    StreamBuilder builder;
    engine.playStream_ = builder.CreateStream(
            engine.sampleFormat_,
            engine.sampleChannels_,
            AAUDIO_SHARING_MODE_SHARED,
            AAUDIO_DIRECTION_OUTPUT,
            INVALID_AUDIO_PARAM,
            dataCallback,
            &engine);

    assert(engine.playStream_);

    PrintAudioStreamInfo(engine.playStream_);
    engine.sampleRate_ = AAudioStream_getSampleRate(engine.playStream_);
    engine.framesPerBurst_ = AAudioStream_getFramesPerBurst(engine.playStream_);
    engine.defaultBufSizeInFrames_ = AAudioStream_getBufferSizeInFrames(engine.playStream_);
    AAudioStream_setBufferSizeInFrames(engine.playStream_, engine.framesPerBurst_);
    engine.bufSizeInFrames_ = engine.framesPerBurst_;

    aaudio_result_t result = AAudioStream_requestStart(engine.playStream_);
    if (result != AAUDIO_OK) {
        assert(result == AAUDIO_OK);
        return false;
    }

    aaudio_stream_state_t inputState = AAUDIO_STREAM_STATE_STARTING;
    aaudio_stream_state_t nextState = AAUDIO_STREAM_STATE_UNINITIALIZED;
    result = AAudioStream_waitForStateChange(engine.playStream_, inputState, &nextState, 1000);
    /*if (result != AAUDIO_OK) {
        assert(result == AAUDIO_OK);
        return false;
    }*/
    aaudio_stream_state_t aaudioStreamState = AAudioStream_getState(engine.playStream_);
    LOGV("[AAudioManager] currentState2=%d %s", aaudioStreamState,
         AAudio_convertStreamStateToText(aaudioStreamState));

    engine.underRunCount_ = AAudioStream_getXRunCount(engine.playStream_);
    return !(result != AAUDIO_OK);
}

/*
 * start():
 *   start to render sine wave audio.
 */
bool AAudioManager::start() {
    if (!engine.playStream_) {
        return false;
    }

    engine.playAudio_ = true;
    return true;
}

/*
 * stop():
 *   stop rendering sine wave audio ( resume rendering silent audio )
 */
bool AAudioManager::stop() {
    if (!engine.playStream_) {
        return true;
    }

    engine.playAudio_ = false;
    return true;
}

/*
 * delete()
 *   clean-up sample: application is going away. Simply setup stop request
 *   flag and rendering thread will see it and perform clean-up
 */
void AAudioManager::deleteEngine() {
    if (!engine.playStream_) {
        return;
    }
    engine.requestStop_ = true;
}
