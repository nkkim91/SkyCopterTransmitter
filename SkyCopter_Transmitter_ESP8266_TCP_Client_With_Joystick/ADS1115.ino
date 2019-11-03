#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include "config.h"
#include "ADS1115.h"

#define ADS1115_BIT_VALUE (0.1875)  // 0.125
#define ADS1115_CALIBRATION_TIME  (5000)

Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
//Adafruit_ADS1015 ads;     /* Use thi for the 12-bit version */

float gfVMax0, gfVMax1, gfVMax2, gfVMax3;
float gfVMin0, gfVMin1, gfVMin2, gfVMin3;

unsigned int gunVMin0, gunVMin1, gunVMin2, gunVMin3;
unsigned int gunVMax0, gunVMax1, gunVMax2, gunVMax3;


void ADS1115_Setup(void) 
{
  Serial.println("Getting single-ended readings from AIN0..3");
  Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)");
  
  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  
  ads.begin();
} 

void ADS1115_Calibration(void)
{
  unsigned long ulCalibrationTime;
  
  uint16_t u16Adc0, u16Adc1, u16Adc2, u16Adc3;
  float fVoltage0, fVoltage1, fVoltage2, fVoltage3;

  
  gfVMin0 = gfVMin1 = gfVMin2 = gfVMin3 = 4200.0;
  gfVMax0 = gfVMax1 = gfVMax2 = gfVMax3 = 3000.0;

  Serial.println("# Calibration Start !! -----------------------------------------------------------------");

  ulCalibrationTime = millis();
  for( ; millis() - ulCalibrationTime < ADS1115_CALIBRATION_TIME ; ) {

    /************************************************************************/
    u16Adc0 = ads.readADC_SingleEnded(0);
    fVoltage0 = (float)(u16Adc0 * ADS1115_BIT_VALUE);

    if( round(gfVMax0) < round(fVoltage0) ) {
      gfVMax0 = fVoltage0;
    }

    if( round(gfVMin0) > round(fVoltage0) ) {
      gfVMin0 = fVoltage0;
    }

    /************************************************************************/
    u16Adc1 = ads.readADC_SingleEnded(1);
    fVoltage1 = (float)(u16Adc1 * ADS1115_BIT_VALUE);

    if( round(gfVMax1) < round(fVoltage1) ) {
      gfVMax1 = fVoltage1;
    }

    if( round(gfVMin1) > round(fVoltage1) ) {
      gfVMin1 = fVoltage1;
    }

    /************************************************************************/
    u16Adc2 = ads.readADC_SingleEnded(2);
    fVoltage2 = (float)(u16Adc2 * ADS1115_BIT_VALUE);

    if( round(gfVMax2) < round(fVoltage2) ) {
      gfVMax2 = fVoltage2;
    }

    if( round(gfVMin2) > round(fVoltage2) ) {
      gfVMin2 = fVoltage2;
    }

    /************************************************************************/
    u16Adc3 = ads.readADC_SingleEnded(3);
    fVoltage3 = (float)(u16Adc3 * ADS1115_BIT_VALUE);

    if( round(gfVMax3) < round(fVoltage3) ) {
      gfVMax3 = fVoltage3;
    }

    if( round(gfVMin3) > round(fVoltage3) ) {
      gfVMin3 = fVoltage3;
    }
    delay(50);
  }

  gunVMax0 = (unsigned int)gfVMax0;
  gunVMax1 = (unsigned int)gfVMax1;
  gunVMax2 = (unsigned int)gfVMax2;
  gunVMax3 = (unsigned int)gfVMax3;

  gunVMin0 = (unsigned int)gfVMin0;
  gunVMin1 = (unsigned int)gfVMin1;
  gunVMin2 = (unsigned int)gfVMin2;
  gunVMin3 = (unsigned int)gfVMin3;
  
  Serial.println("# Calibration Done !! ------------------------------------------------------------------");
  Serial.println("");
  Serial.print("fVMax0/fMin0/gunVMax0/gunVMin0 : "); Serial.print(gfVMax0); Serial.print("/"); Serial.print(gfVMin0); Serial.print("/"); Serial.print(gunVMax0); Serial.print("/"); Serial.println(gunVMin0);
  Serial.print("fVMax1/fMin1/gunVMax1/gunVMin1 : "); Serial.print(gfVMax1); Serial.print("/"); Serial.print(gfVMin1); Serial.print("/"); Serial.print(gunVMax1); Serial.print("/"); Serial.println(gunVMin1);
  Serial.print("fVMax2/fMin2/gunVMax2/gunVMin2 : "); Serial.print(gfVMax2); Serial.print("/"); Serial.print(gfVMin2); Serial.print("/"); Serial.print(gunVMax2); Serial.print("/"); Serial.println(gunVMin2);
  Serial.print("fVMax3/fMin3/gunVMax3/gunVMin3 : "); Serial.print(gfVMax3); Serial.print("/"); Serial.print(gfVMin3); Serial.print("/"); Serial.print(gunVMax3); Serial.print("/"); Serial.println(gunVMin3);
  Serial.println("");
}

int ADS1115_nRead(struct ADS1115_nVoltaData *pstAdsVoltData)
{
  struct ADS1115_fVoltData stTmpAdsVoltData;
  
  ADS1115_fRead(&stTmpAdsVoltData);

  pstAdsVoltData->unVoltage0 = (unsigned int)stTmpAdsVoltData.fVoltage0;
  pstAdsVoltData->unVoltage1 = (unsigned int)stTmpAdsVoltData.fVoltage1;
  pstAdsVoltData->unVoltage2 = (unsigned int)stTmpAdsVoltData.fVoltage2;  
  pstAdsVoltData->unVoltage3 = (unsigned int)stTmpAdsVoltData.fVoltage3;

  return 0;
}

int ADS1115_fRead(struct ADS1115_fVoltData *pstAdsVoltData)
{
  struct ADS1115_RawData stAdsRawData;
//  ADS1115_VoltData_t stTmpAdsVoltData;

  stAdsRawData.u16Adc0 = ads.readADC_SingleEnded(0);
  stAdsRawData.u16Adc1 = ads.readADC_SingleEnded(1);
  stAdsRawData.u16Adc2 = ads.readADC_SingleEnded(2);
  stAdsRawData.u16Adc3 = ads.readADC_SingleEnded(3);

  pstAdsVoltData->fVoltage0 = (float)(stAdsRawData.u16Adc0 * ADS1115_BIT_VALUE);
  pstAdsVoltData->fVoltage1 = (float)(stAdsRawData.u16Adc1 * ADS1115_BIT_VALUE);
  pstAdsVoltData->fVoltage2 = (float)(stAdsRawData.u16Adc2 * ADS1115_BIT_VALUE);
  pstAdsVoltData->fVoltage3 = (float)(stAdsRawData.u16Adc3 * ADS1115_BIT_VALUE);


  if( pstAdsVoltData->fVoltage0 < gfVMin0 ) pstAdsVoltData->fVoltage0 = gfVMin0;
  if( pstAdsVoltData->fVoltage0 > gfVMax0 ) pstAdsVoltData->fVoltage0 = gfVMax0;
  pstAdsVoltData->fVoltage0 = gfVMax0 - pstAdsVoltData->fVoltage0 + gfVMin0;

  if( pstAdsVoltData->fVoltage1 < gfVMin1 ) pstAdsVoltData->fVoltage1 = gfVMin1;
  if( pstAdsVoltData->fVoltage1 > gfVMax1 ) pstAdsVoltData->fVoltage1 = gfVMax1;
  pstAdsVoltData->fVoltage1 = gfVMax1 - pstAdsVoltData->fVoltage1 + gfVMin1;

  if( pstAdsVoltData->fVoltage2 < gfVMin2 ) pstAdsVoltData->fVoltage2 = gfVMin2;
  if( pstAdsVoltData->fVoltage2 > gfVMax2 ) pstAdsVoltData->fVoltage2 = gfVMax2;
  pstAdsVoltData->fVoltage2 = gfVMax2 - pstAdsVoltData->fVoltage2 + gfVMin2;

  if( pstAdsVoltData->fVoltage3 < gfVMin3 ) pstAdsVoltData->fVoltage3 = gfVMin3;
  if( pstAdsVoltData->fVoltage3 > gfVMax3 ) pstAdsVoltData->fVoltage3 = gfVMax3;
  pstAdsVoltData->fVoltage3 = gfVMax3 - pstAdsVoltData->fVoltage3 + gfVMin3;

  return 0;   // return 0 on success
}
