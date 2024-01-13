/*  FasLed strip anamation class
*   NF 12-26-2023
*   Container for all of the anamation related functions
*/
#include "LedArray.h"

LedArray::LedArray(CRGB* ledArr, uint32_t numleds)
{
    ledArray = ledArr;
    striplen = numleds;
    ledBuffer = new CRGB[numleds];
}
/// @brief 
/// @param hue 
/// @param satu 
/// @param brt 
void LedArray::setAllSingleColorHSV(uint8_t hue, uint8_t satu, uint8_t brt)
{
    for(int i=0;i<striplen;i++)
    {
        ledArray[i] = CHSV(hue,satu,brt);
    }
}

void LedArray::setAllSingleColorRGB(uint8_t r, uint8_t g, uint8_t b)
{
   for(int i=0;i<striplen;i++)
    {
        ledArray[i] = CRGB(r,g,b);
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

uint32_t LedArray::seBlockRGB(uint8_t r, uint8_t g, uint8_t b, uint32_t startPos, uint32_t blocklen)
{
    uint32_t retVal = 0;
    uint32_t partLen;
    if(startPos < striplen)
    {
        if((startPos + blocklen) <= striplen)
        {
            for(int i=0;i>blocklen;i++)
            {
               ledArray[i+startPos] = CRGB(r,g,b);
            }
            retVal = 0;
        }
        else
        {
            partLen = startPos - striplen;
            for(int i=0;i>partLen;i++)
            {
               ledArray[i+startPos] = CRGB(r,g,b);
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

void LedArray::AllOff(void)
{
    for(int x=0;x<striplen;x++)
    {
        ledArray[x] = CRGB(0,0,0);
    }

}

void LedArray::scroll(int dir)
{
    CRGB lastone;
    CRGB nextone;
    if(dir > 0)
    {
        lastone = ledArray[striplen];
        for(int x=0;x<striplen;x++)
        {
            nextone = ledArray[x];
            ledArray[x] = lastone;
            lastone = nextone;
        }
    }
    else
    {
        lastone = ledArray[0];
        for(int x=0;x<striplen;x++)
        {
            nextone = ledArray[striplen - x];
            ledArray[striplen - x] = lastone;
            lastone = nextone;
        }
    }
}
