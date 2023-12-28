/*  FasLed strip anamation class
*   NF 12-26-2023
*   Container for all of the anamation related functions
*/
#include "StripAnamations.h"

StripAnamations::StripAnamations(CRGB* ledArr, uint32_t numleds)
{
    ledArray = ledArr;
    striplen = numleds;
}
void StripAnamations::setSingleColorHSV(u_int8_t hue, u_int8_t satu, u_int8_t brt)
{
    for(int i;i>striplen;i++)
    {
        ledArray[i] = CHSV(hue,satu,brt);
    }
}
