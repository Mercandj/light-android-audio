# Light Audio Android Sound-system

----

<p align="center">
	<a margin="20px 0" href="https://github.com/Mercandj/light-android-audio/tree/master/soundsystemnative/src/main/jni">
		<img  src="https://raw.github.com/Mercandj/light-android-audio/master/screenshot.png" width="560" />
	</a>
</p>

## Extract + Decode


### Techno

* OpenSL
* MediaCodec
* FFmpeg

### Benchmark

Result are avg on 2, 3 launches...
Extract and decode from internal storage the tracks:
 * `app/src/main/assets/over_the_horizon.aac`
 * `app/src/main/assets/over_the_horizon.mp3`
 * `app/src/main/assets/over_the_horizon.wav`

| Device \ Techno on AAC       | OpenSL 1 Native thread | MediaCodec 1 Native thread | FFmpeg 1 Java thread |
|------------------------------|------------------------|----------------------------|----------------------|
| Pixel 8.0.0 op3              | 90.0s                  | 20.0s                      | 01.0s                |
| Nexus 5X 8.0.0 op3           | 118 s                  | 42.0s                      | 04.2s                |  
| Samsung gs7 SM-G930T 7.0     | 116 s                  | 27.0s                      | 01.1s                |
| Samsung tab s2 SM-T810 6.0.1 | 05.4s                  | 04.3s                      | 01.9s                |
| Nexus 5 5.1.1                | 05.7s                  | /                          | 03.2s                |
| Wiko FEVER 6.0               | /                      | /                          | /                    |

| Device \ Techno on MP3       | OpenSL 1 Native thread | MediaCodec 1 Native thread | FFmpeg 1 Java thread |
|------------------------------|------------------------|----------------------------|----------------------|
| Pixel 8.0.0 op3              | 75.0s                  | 18.0s                      | 01.5s                |
| Nexus 5X 8.0.0 op3           | 99.0s                  | 30.0s                      | 01.9s                |
| Samsung gs7 SM-G930T 7.0     | 71.0s                  | 35.0s                      | 01.5s                |
| Samsung tab s2 SM-T810 6.0.1 | 04.9s                  | 03.6s                      | 02.5s                |
| Nexus 5 5.1.1                | 04.5s                  | 07.5s                      | 06.0s                |
| Wiko FEVER 6.0               | crash                  | 04.0s                      | 03.0s                |

| Device \ Techno on WAV       | OpenSL 1 Native thread | MediaCodec 1 Native thread | FFmpeg 1 Java thread |
|------------------------------|------------------------|----------------------------|----------------------|
| Pixel 8.0.0 op3              | 00.3s                  | 01.9s                      | 00.7s                |
| Nexus 5X 8.0.0 op3           | 00.5s                  | 01.7s                      | 02.7s                |
| Samsung gs7 SM-G930T 7.0     | 01.9s                  | 05.5s                      | 00.8s                |
| Samsung tab s2 SM-T810 6.0.1 | 00.1s                  | 01.4s                      | 01.4s                |
| Nexus 5 5.1.1                | 00.3s                  | 01.0s                      | 02.2s                |
| Wiko FEVER 6.0               | /                      |     s                      |     s                |


----

## Player

* OpenSL
* AAudio API >= 26

----

## Source
 
 * https://github.com/bowserf/Mini-sound-system
 * https://github.com/googlesamples/android-audio-high-performance/tree/master/aaudio