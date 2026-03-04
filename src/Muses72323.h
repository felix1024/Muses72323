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
#ifndef INCLUDED_MUSES_72323
#define INCLUDED_MUSES_72323

#include <Arduino.h>
#include <SPI.h>


// control select addresses, chip address (low 2) ignored.
const uint8_t s_control_attenuation_l 	= 0b00000000;
const uint8_t s_control_attenuation_r 	= 0b00000100;
const uint8_t s_control_gain_lr       	= 0b00001000;
const uint8_t s_control_states        	= 0b00001100;

// control state bits.
const uint8_t s_gain_bit_link			= 7; // 0 = L/R independent control (default)
const uint8_t s_gain_bit_zc				= 0; // 0 = zero cross detection (default)
const uint8_t s_att_bit_ss				= 4; // 0 = softstep disabled (default)
const uint8_t s_gain_mask_bits			= 0b10000001; 
const uint8_t s_state_bit_ssclk         = 1; // 0 = soft-step clk external (default)
const uint8_t s_state_mask_clkdiv		= 0b11100011;
const uint8_t s_state_mask_zerowin		= 0b10011111;

class Muses72323 {

private:

    void transfer(uint16_t data);
	void printbin(uint16_t data);

	// for multiple chips on the same bus line.
	uint8_t _address;
    uint8_t _ss;
	uint32_t _sclk;
	SPISettings _spiSettings;
	SPIClass * _spi;

	// muses state bits: (hiorder byte)
	//	 [6-5]:	zero window bits
	//   [4-2]: clk divider bits
	//   1:     clock select enable internal
	//   7: 	not used
	//	 0:		not used
	uint8_t _states;
	
	// muses gain bits: (hiorder byte)
	//	 7:		left/right volume link
	//	 [6-4]	left channel gain
	//	 [3-1]	right channel gain
	//	 0:		zero crossing enable 
	uint8_t _gain;
	uint8_t _softstep;

public:
	// specify a connection over an address using three output pins.
	Muses72323(SPIClass* spi, uint32_t, uint8_t, uint8_t);

	// set the pins in their correct states.
	void begin();

	// set the volume using the following formula:
	//   (-0.25 * volume) db
	// audio level goes from [-111.75, 0.0] dB
	// input goes from -447 to 0.
	void setVolume(int16_t left, int16_t right);
	inline void setVolume(int16_t volume) { setVolume(volume, volume); }

	// gain is constrained to [0, 21] dB in 3db steps
	// input goes from 0 to 7.
	void setGain(int8_t left, int8_t right);
	inline void setGain(int8_t volume) { setGain(volume, volume); }

    void mute();

	// enable or disable zero crossing.
	// softsteps internal should be enabled first
	// softclock divider range is 0..7
	// zero window range is 0..3
	void setZeroCrossing(bool enabled);
	void setSoftStep(bool enabled);
	void setAttenuationLink(bool enabled);
	void setInternalClock(bool enabled);
	void setSoftClockDivider(uint8_t divider);
	void setZeroWindow(uint8_t window);
};

#endif // INCLUDED_MUSES_72323