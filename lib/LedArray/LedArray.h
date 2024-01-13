// 	LedArray.h
/*  FasLed strip anamation class
*   NF 12-26-2023
*   Container for all of the anamation related functions
*/

#ifndef _LEDARRAY_h
#define _LEDARRAY_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif
#include <FastLED.h>
#include <functional>

class LedArray{
 protected:
    uint32_t striplen;
    CRGB* ledArray;
	CRGB* ledBuffer;
	

 public:
	
	// Constuctor need to pass the leds array in
	LedArray(CRGB* ledArr, uint32_t numleds);
	void setAllSingleColorHSV(uint8_t hue, uint8_t satu, uint8_t brt);
	void setAllSingleColorRGB(uint8_t r, uint8_t g, uint8_t b);
	uint32_t seBlockHSV(uint8_t hue, uint8_t satu, uint8_t brt, uint32_t startPos, uint32_t blocklen);
	uint32_t seBlockRGB(uint8_t r, uint8_t g, uint8_t b, uint32_t startPos, uint32_t blocklen);
	uint8_t HueCycle(u_int8_t hue);
	void AllOff(void);
	void scroll(int dir);

};


#endif