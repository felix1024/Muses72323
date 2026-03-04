/*
  The MIT License (MIT)

  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the "Software"), to deal in
  the Software without restriction, including without limitation the rights to
  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
  the Software, and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
  FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  The library interface and documentation was taken from the existing library by
  Copyright (c) 2016 Christoffer Hjalmarsson see https://github.com/qhris/Muses72320/
  
  Adaption for the newer Muses72323 (c) 2026 by Felix Thommen. This includes:
  - changed control data registers
  - volume changes in 0.25dB steps
  - reduced addressing to 2bits
  - output gain changed to 8 3dB steps
  - handling of soft dB steps
  - SPI interface now for any bus possible 
*/

#include "Muses72323.h"

static inline uint16_t volume_to_attenuation(int16_t volume)
{
	// volume to attenuation data conversion: (9 bit)
	// #========================================#
	// |    0.00 dB | in: [   0] -> 0b000100000 |
	// | -111.75 dB | in: [-447] -> 0b111011111 |
	// #========================================#
	return static_cast<uint16_t>(min(-volume, 447) + 0x20)<<7;
}

Muses72323::Muses72323(SPIClass* spi, uint32_t clock, uint8_t address, uint8_t ss)  {
	_address = address & 0b00000011;
	_ss = ss;
	_sclk = clock;
	_spi = spi;
  _states = 0;
	_gain = 0;
  _softstep = 0;
}

void Muses72323::begin()
{
  Serial.println("Muses72323 begin()");
  Serial.println("CS Pin: " + String(_ss));
  Serial.println("Address selected: " + String(_address));
  ::digitalWrite(_ss, HIGH);
  ::pinMode(_ss, OUTPUT);
  // define SPI clockspeed, endian and mode
  _spiSettings = SPISettings(_sclk, MSBFIRST, SPI_MODE2);
  _spi->begin();
}

void Muses72323::setVolume(int16_t lch, int16_t rch)
{
	// save volume settings
	if (bitRead(_gain, s_gain_bit_link)) {
		// interconnected left and right channels.
		transfer(volume_to_attenuation(lch) | s_control_attenuation_l | _softstep);
	} else {
		// independent left and right channels.
		transfer(volume_to_attenuation(lch) | s_control_attenuation_l | _softstep);
		transfer(volume_to_attenuation(rch) | s_control_attenuation_r | _softstep);
	}
}

void Muses72323::setGain(int8_t lch, int8_t rch)
{
	// always independent left and right channels.
	uint8_t _tgain = (((uint8_t)lch << 4) | ((uint8_t)rch << 1) | (_gain & s_gain_mask_bits));
	
	transfer(((uint16_t)_tgain << 8) | s_control_gain_lr);
	// save actual value
	_gain = _tgain;
}

void Muses72323::mute()
{
	if (bitRead(_gain, s_gain_bit_link)) {
    // interconnected left and right channels.
		transfer(0xff80 | s_control_attenuation_l | _softstep);
  } else {
	  transfer(0xff80 | s_control_attenuation_l | _softstep);
	  transfer(0xff80 | s_control_attenuation_r | _softstep);
  }
}

void Muses72323::setZeroCrossing(bool enabled)
{
	// 0 is enabled, 1 is disabled.
	bitWrite(_gain, s_gain_bit_zc, !enabled);
	transfer(((uint16_t)_gain << 8) | s_control_gain_lr);
}

void Muses72323::setAttenuationLink(bool enabled)
{
	// 1 is enabled, 0 is disabled.
	bitWrite(_gain, s_gain_bit_link, enabled);
	transfer(((uint16_t)_gain << 8) | s_control_gain_lr);
}

void Muses72323::setSoftStep(bool enabled){
  // written with any attentuator change
  bitWrite(_softstep,  s_att_bit_ss, enabled);
}

void Muses72323::setInternalClock(bool enabled){
  // 0 -> external clock (default)
  // 1 -> internal clock
  bitWrite(_states,  s_state_bit_ssclk, enabled);
  transfer(((uint16_t)_states << 8) | s_control_states);
}

void Muses72323::setSoftClockDivider(uint8_t divider){
  // 0  ->  default
  // 1  ->  /4
  // ...
  // 7  ->  /256
  uint8_t _tdiv = (divider & 0x7) << 2;
  _states = _states & s_state_mask_clkdiv | _tdiv;
  transfer(((uint16_t)_states << 8) | s_control_states);
}

void Muses72323::setZeroWindow(uint8_t window){
  // 0  ->  default
  // 1  ->  *2
  // 2  ->  *4
  // 3  ->  *8
  uint8_t _tzero = (window & 0x3) << 5;
  _states = _states & s_state_mask_zerowin | _tzero;
  transfer(((uint16_t)_states << 8) | s_control_states);
}

void Muses72323::transfer(uint16_t  data)
{
  uint8_t _tmp;  
  _spi->beginTransaction(_spiSettings);
    ::digitalWrite(_ss, LOW);
    // debug
    // printbin(data | _address);
    _spi->transfer(data >> 8);
    _spi->transfer(data | _address);
    ::digitalWrite(_ss, HIGH);
  _spi->endTransaction();
}

void Muses72323::printbin(uint16_t data)
{
  Serial.print("0b");
  for (int b= 15; b >=0; b--)
  {
    Serial.print(bitRead(data, b));
    if (b == 8){Serial.print(".");}
  }
  Serial.println();
}
