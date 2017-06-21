# Light Audio Android Sound-system

----

## Extract + Decode


### Techno

* OpenSL
* MediaCodec
* FFMPEG

### Benchmark

Result are avg on 2, 3 launches...
Extract and decode from internal storage the tracks
 * `app/src/main/assets/over_the_horizon.aac`
 * `app/src/main/assets/over_the_horizon.mp3`
 * `app/src/main/assets/over_the_horizon.wav`

| Device \ Techno on AAC       | OpenSL 1 Native thread | MediaCodec 1 Native thread | FFMPEG 1 Java thread |
|------------------------------|------------------------|----------------------------|----------------------|
| Pixel 8.0.0 op3              | 90.0s                  | crash                      | 01.0s                |
| Nexus 5X 8.0.0 op3           | 118 s                  | crash                      | 04.2s                |  
| Samsung gs7 SM-G930T 7.0     | 116 s                  | crash                      | 01.1s                |
| Samsung tab s2 SM-T810 6.0.1 |     s                  |     s                      |     s                |
| Wiko FEVER 6.0               | /                      |     s                      |     s                |
| Nexus 5 5.1.1                |     s                  |     s                      |     s                |

| Device \ Techno on MP3       | OpenSL 1 Native thread | MediaCodec 1 Native thread | FFMPEG 1 Java thread |
|------------------------------|------------------------|----------------------------|----------------------|
| Pixel 8.0.0 op3              | 75.0s                  | 18.0s                      | 01.5s                |
| Nexus 5X 8.0.0 op3           | 99.0s                  | 30.0s                      | 01.9s                |
| Samsung gs7 SM-G930T 7.0     | 71.0s                  | 35.0s                      | 01.5s                |
| Samsung tab s2 SM-T810 6.0.1 | 04.9s                  | 03.6s                      | 02.5s                |
| Wiko FEVER 6.0               | crash                  | 04.0s                      | 03.0s                |
| Nexus 5 5.1.1                | 04.5s                  | 07.5s                      | 06.0s                |

| Device \ Techno on WAV       | OpenSL 1 Native thread | MediaCodec 1 Native thread | FFMPEG 1 Java thread |
|------------------------------|------------------------|----------------------------|----------------------|
| Pixel 8.0.0 op3              | 00.3s                  | 01.9s                      | 00.7s                |
| Nexus 5X 8.0.0 op3           | 00.5s                  | 01.7s                      | 02.7s                |
| Samsung gs7 SM-G930T 7.0     | 01.9s                  | 05.5s                      | 00.8s                |
| Samsung tab s2 SM-T810 6.0.1 |     s                  |     s                      |     s                |
| Wiko FEVER 6.0               | /                      |     s                      |     s                |
| Nexus 5 5.1.1                |     s                  |     s                      |     s                |


----

## Player

* OpenSL
* AAudio API >= 26

----

## Source
 
 * https://github.com/bowserf/Mini-sound-system
 * https://github.com/googlesamples/android-audio-high-performance/tree/master/aaudio