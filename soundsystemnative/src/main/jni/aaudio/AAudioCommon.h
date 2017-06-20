/*
 * Copyright 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef AAUDIO

#ifndef SOUND_SYSTEM_AAUDIO_COMMON_H
#define SOUND_SYSTEM_AAUDIO_COMMON_H

#include <aaudio/AAudio.h>
#include <utils/android_debug.h>

/*
 * Audio Sample Controls...
 */
#define AUDIO_SAMPLE_CHANNELS 2

uint16_t SampleFormatToBpp(aaudio_format_t format);

/*
 * GetSystemTicks(void):  return the time in micro sec
 */
__inline__ uint64_t GetSystemTicks(void) {
    struct timeval Time;
    gettimeofday(&Time, NULL);

    return (static_cast<uint64_t>(1000000) * Time.tv_sec + Time.tv_usec);
}

/*
 * flag to enable file dumping
 */
//#define ENABLE_LOG  1

void PrintAudioStreamInfo(const AAudioStream *stream);

#endif // SOUND_SYSTEM_AAUDIO_COMMON_H

#endif // AAUDIO
