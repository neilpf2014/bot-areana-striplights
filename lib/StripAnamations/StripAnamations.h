// 	StripAnamations.h
/*  FasLed strip anamation class
*   NF 12-26-2023
*   Container for all of the anamation related functions
*/

#ifndef _STRIPANAMATIONS_h
#define _STRIPANAMATIONS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif
#include <FastLED.h>
#include <functional>

class StripAnamations{
 protected:
    uint32_t striplen;
    CRGB* ledArray;
	

 public:
	
	// Constuctor need to pass the leds array in
	StripAnamations(CRGB* ledArr);
	CRGB* setSingleColorHSV(u_int8_t hue, u_int8_t satu, u_int8_t brt);


};


#endif