# Light Audio Android Sound-system

----

## Extract + Decode


### Techno

* OpenSL
* MediaCodec
* FFMPEG

### Benchmark

Extract and decode the track `app/src/main/assets/over_the_horizon.mp3` from internal storage.

Result are avg on 2, 3 launches...

| Device \ Techno              | OpenSL 1 Native thread | MediaCodec 1 Native thread | FFMPEG 1 Java thread |
|------------------------------|------------------------|----------------------------|----------------------|
| Pixel 8.0.0 op3              | 75s                    | 18s                        | 1.5s                 |
| Samsung tab s2 SM-T810 6.0.1 | 4.9s                   | 3.6s                       | 2.5s                 |
| Samsung gs7 SM-G930T 7.0     | 71s                    | 35s                        | 1.5s                 |                      |
| Nexus 5X 8.0.0 op3           | 99s                    | 30s                        | 1.9s                 |
| Wiko FEVER 6.0               | /                      | 4s                         | 3s                   |
| Nexus 5 5.1.1                | 4.5s                   | 7.5s                       | 6s                   |


----

## Player

* OpenSL
* AAudio API >= 26

----

## Source
 
 * https://github.com/bowserf/Mini-sound-system
 * https://github.com/googlesamples/android-audio-high-performance/tree/master/aaudio