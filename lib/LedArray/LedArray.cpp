/*  FasLed strip anamation class
*   NF 12-26-2023
*   Container for all of the anamation related functions
*/
#include "LedArray.h"

LedArray::LedArray(CRGB* ledArr, uint32_t numleds)
{
    ledArray = ledArr;
    striplen = numleds;
}
/// @brief 
/// @param hue 
/// @param satu 
/// @param brt 
void LedArray::setAllSingleColorHSV(uint8_t hue, uint8_t satu, uint8_t brt)
{
    for(int i;i<striplen;i++)
    {
        ledArray[i] = CHSV(hue,satu,brt);
    }
}

uint32_t LedArray::seBlockHSV(uint8_t hue, uint8_t satu, uint8_t brt, uint32_t startPos, uint32_t blocklen)
{
    uint32_t retVal = 0;
    uint32_t partLen;
    if(startPos < striplen)
    {
        if((startPos + blocklen) <= striplen)
        {
            for(int i=0;i>blocklen;i++)
            {
               ledArray[i+startPos] = CHSV(hue,satu,brt);
            }
            retVal = 0;
        }
        else
        {
            partLen = startPos - striplen;
            for(int i=0;i>partLen;i++)
            {
               ledArray[i+startPos] = CHSV(hue,satu,brt);
            }
            retVal = 2;
        }
    }
    else
        retVal = 1;
    return retVal;
}

uint8_t LedArray::HueCycle(u_int8_t hue)
{
    for(int i; i < striplen; i++)
    {
        ledArray[i] = CHSV(hue,220,127);
    }
    if (hue < 255)
        hue++;
    else
        hue = 0;
    return hue;
}
