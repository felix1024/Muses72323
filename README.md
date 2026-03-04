# Muses72323
Arduino library for communication with the audio chip Muses72323.
Written for ESP32 and fully tested.
The data sheets can be found [here](https://www.nisshinbo-microdevices.co.jp/en/pdf/datasheet/MUSES72323_E.pdf) (pdf).

## Download

Download the latest release over at the [Releases](https://github.com/felix1024/Muses72323/releases) page.

## Example

```c++
#include <Arduino.h>
#include <Muses72323.h>

#define VOLUME_ADDRESS      0   // internal chip address
#define VOLUME_CS           21  // GPIO port for chip select (latch)
#define VOL_MAX   0
#define VOL_MIN   -447

// volume attenuator
Muses72323 muses = Muses72323(&SPI, 100000UL, VOLUME_ADDRESS, VOLUME_CS);


void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Booting...");

  muses.begin();

  // enable internal clock for soft stepping
  Serial.println("Set internal clock source for softstep");
  muses.setInternalClock(true);
  // enable soft stepping
  Serial.println("enable softstep");
  muses.setSoftStep(true);
  // mute
  Serial.println("mute output");
  muses.mute();

  // Zerocrossing and independent left & right attentuaion is enabled by default
  // set gain to  +6dB
  Serial.println("Set gain to +6dB");
  muses.setGain(2);

  // set volume to -200
  Serial.println("Set volume to -200");
  muses.setVolume(-200);
  delay(2000);

  // set different values for left and right
  Serial.println("Set left volume to -100 and right volume to -1");
  muses.setVolume(-447, -1);
}

// the loop function runs over and over again until power down or reset
void loop() {

}

```

## Problems

Please post any problems on the [Issues](https://github.com/felix1024/Muses72320/issues) page.

## License

Please read over the LICENSE file included in the project.
