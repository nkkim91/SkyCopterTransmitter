//#include "config.h"

#define JOYSTICK_CALIBRATION_TIME (7000)    // 7 sec
#define DEBUG_LED_BLINK_PERIOD  (100)       // 100 ms

int JS_THR_MIN_RAW, JS_THR_MAX_RAW;
int JS_RUD_MIN_RAW, JS_RUD_MAX_RAW;
int JS_ELE_MIN_RAW, JS_ELE_MAX_RAW;
int JS_AIL_MIN_RAW, JS_AIL_MAX_RAW;

unsigned long ulButtonEventStartTime;
unsigned long ulCalibrationTime;

int nCurrentMSPArmState = MSP_DISARMED; // 

int DEBUG_LED_Pin = D5;
int JS_Button_Pin = D3;
int JS_Button_Event = 0;

extern unsigned int gunChannelData[MAX_TX_CHANNEL];

void ToggleDebugLed()
{
  static int toggle = 0;

  if( toggle++ % 2 ) {
    digitalWrite(DEBUG_LED_Pin, HIGH);
    
  } else {
    digitalWrite(DEBUG_LED_Pin, LOW);
  }
}

void JS_Setup()
{
#if 0
  pinMode(JS_Throttle_Pin, INPUT);
  pinMode(JS_RUDDER_Pin, INPUT);
  pinMode(JS_ELEVATOR_Pin, INPUT);
  pinMode(JS_AILERON_Pin, INPUT);
#endif

  pinMode(JS_Button_Pin, INPUT_PULLUP);

  pinMode(DEBUG_LED_Pin, OUTPUT);
  digitalWrite(DEBUG_LED_Pin, LOW);

#ifdef JOYSTICK_BUTTON
  attachInterrupt(digitalPinToInterrupt(JS_Button_Pin), JS_Button_Handler, FALLING);
#endif /* JOYSTICK_BUTTON */

  Serial.printf("T2C : 0x%x\n", T2C);

//  JS_Calibration();
}


#ifdef JOYSTICK_BUTTON
ICACHE_RAM_ATTR void JS_Button_Handler(void)
{
  unsigned long ulStartTime;
  unsigned int unButtonState;
  unsigned int unSavedT2V;
  unsigned int unSavedT2L;

#ifdef DEBUG_NK
  if( gunDebugMask & NK_DEBUG_DEBUG ) {
    Serial.printf("%d][+] JS_Button_Handler\n", T2V);
  }
#endif // DEBUG_NK

/***************************************************************************************
//Timer 2 Registers (32bit CountUp Timer)
#define T2L  ESP8266_REG(0x620) //Load Value (Starting Value of Counter), FRC2_LOAD_ADDRESS
#define T2V  ESP8266_REG(0x624) //(RO) Current Value, FRC2_COUNT_ADDRESS
#define T2C  ESP8266_REG(0x628) //Control Register, FRC2_CTRL_ADDRESS
#define T2I  ESP8266_REG(0x62C) //Interrupt Status Register (1bit) write to clear, FRC2_INT_ADDRESS
#define T2A  ESP8266_REG(0x630) //Alarm Value, FRC2_ALARM_ADDRESS
 */

#if 1
  for( T2L=0, ulStartTime = millis(); (unButtonState = digitalRead(JS_Button_Pin)) != BUTTON_HIGH && (unSavedT2V = T2V) < TIMER1_1SEC; ) { // 0x3E80 (16000) = 16000000 / 1024 = 16000
  }
#endif
  delayMicroseconds(500);

  
#ifdef DEBUG_NK
  if( gunDebugMask & NK_DEBUG_DEBUG ) {
    Serial.printf("%d\n", T2V);
  }
#endif // DEBUG_NK

  if( unSavedT2V < TIMER1_1SEC ) {  // TIMER1_1SEC : 0x3E80 (16000)

    if( millis() - ulLastIRQTime <= SPURIOUS_IRQ_FILTER_TIME ) {  // 50 ms
#ifdef DEBUG_NK
      if( gunDebugMask & NK_DEBUG_DEBUG ) {
        Serial.printf("[-] JS_Button_Handler - Spurious\n");
      }
#endif // DEBUG_NK
      return;
    } else {

      if( JS_Button_Event == 0 ) {
        JS_Button_Event = 1;
        ulLastIRQTime = millis();
        ulButtonEventStartTime = millis();
//        Serial.printf("JS_Button_Event = 1 !!\n");
      } else {
        // if it's the case, something wrong (?)
//        JS_Button_Event = 0;
      }
    }
  }

#ifdef DEBUG_NK
  if( gunDebugMask & NK_DEBUG_DEBUG ) {
    Serial.printf("%d][-] JS_Button_Handler\n", T2V);
  }
#endif // DEBUG_NK
}
#endif  /* JOYSTICK_BUTTON */

void JS_Input()
{

  int nThrottleRawData = 0;
  int nThrottleAdjData = 0;

  int nRudderRawData = 0;
  int nRudderAdjData = 0;

  int nElevatorRawData = 0;
  int nElevatorAdjData = 0;

  int nAileronRawData = 0;
  int nAileronAdjData = 0;

  struct ADS1115_nVoltaData stTmpAdsVoltData;

#ifndef ADS1115
  nThrottleRawData = analogRead(JS_Throttle_Pin);
  
  if( nThrottleRawData < JS_THR_MIToggleDebugLedN_RAW ) nThrottleRawData = JS_THR_MIN_RAW;
  if( nThrottleRawData > JS_THR_MAX_RAW ) nThrottleRawData = JS_THR_MAX_RAW;

  nThrottleRawData = JS_THR_MAX_RAW - nThrottleRawData + JS_THR_MIN_RAW;
  nThrottleAdjData = map(nThrottleRawData, JS_THR_MIN_RAW, JS_THR_MAX_RAW, THROTTLE_MIN, THROTTLE_MAX);  

  nRudderRawData = analogRead(JS_RUDDER_Pin);

  if( nRudderRawData < JS_RUD_MIN_RAW ) nRudderRawData = JS_RUD_MIN_RAW;
  if( nRudderRawData > JS_RUD_MAX_RAW ) nRudderRawData = JS_RUD_MAX_RAW;
  nRudderRawData = JS_RUD_MAX_RAW - nRudderRawData + JS_RUD_MIN_RAW;
  nRudderAdjData = map(nRudderRawData, JS_RUD_MIN_RAW, JS_RUD_MAX_RAW, RUDDER_MIN, RUDDER_MAX);

  nElevatorRawData = analogRead(JS_ELEVATOR_Pin);

  if( nElevatorRawData < JS_ELE_MIN_RAW ) nElevatorRawData = JS_ELE_MIN_RAW;
  if( nElevatorRawData > JS_ELE_MAX_RAW ) nElevatorRawData = JS_ELE_MAX_RAW;
  nElevatorRawData = JS_ELE_MAX_RAW - nElevatorRawData + JS_ELE_MIN_RAW;
  nElevatorAdjData = map(nElevatorRawData, JS_ELE_MIN_RAW, JS_ELE_MAX_RAW, ELEVATOR_MIN, ELEVATOR_MAX);

  nAileronRawData = analogRead(JS_AILERON_Pin);

  if( nAileronRawData < JS_AIL_MIN_RAW ) nAileronRawData = JS_AIL_MIN_RAW;
  if( nAileronRawData > JS_AIL_MAX_RAW ) nAileronRawData = JS_AIL_MAX_RAW;
  nAileronRawData = JS_AIL_MAX_RAW - nAileronRawData + JS_AIL_MIN_RAW;
  nAileronAdjData = map(nAileronRawData, JS_AIL_MIN_RAW, JS_AIL_MAX_RAW, AILERON_MIN, AILERON_MAX);
#else
  ADS1115_nRead(&stTmpAdsVoltData);

  nThrottleRawData = stTmpAdsVoltData.unVoltage0;
  nThrottleAdjData = map(nThrottleRawData, gunVMin0, gunVMax0, THROTTLE_MIN, THROTTLE_MAX);

  nRudderRawData = stTmpAdsVoltData.unVoltage1;
  nRudderAdjData = map(nRudderRawData, gunVMin1, gunVMax1, RUDDER_MIN, RUDDER_MAX);

  nElevatorRawData = stTmpAdsVoltData.unVoltage2;
  nElevatorAdjData = map(nElevatorRawData, gunVMin2, gunVMax2, ELEVATOR_MIN, ELEVATOR_MAX);

  nAileronRawData = stTmpAdsVoltData.unVoltage3;
  nAileronAdjData = map(nAileronRawData, gunVMin3, gunVMax3, AILERON_MIN, AILERON_MAX);
#endif

#ifdef DEBUG_NK
  if( gunDebugMask & NK_DEBUG_DEBUG ) {
    Serial.printf("Raw(TREA), Adj(TREA) - (%4d %4d %4d %4d), (%4d %4d %4d %4d)\n", nThrottleRawData, nRudderRawData, nElevatorRawData, nAileronRawData, nThrottleAdjData, nRudderAdjData, nElevatorAdjData, nAileronAdjData);
  }
#endif // DEBUG_NK

  gunChannelData[THROTTLE] = nThrottleAdjData;
  gunChannelData[RUDDER]   = nRudderAdjData;
  gunChannelData[ELEVATOR] = nElevatorAdjData;
  gunChannelData[AILERON]  = nAileronAdjData;

#ifdef JOYSTICK_BUTTON
  if( JS_Button_Event ) {
    if( millis() - ulButtonEventStartTime > BUTTON_EVENT_DURATION ) { // BUTTON_EVENT_DURATION : 50
      if( nCurrentMSPArmState == MSP_DISARMED ) {
        gunChannelData[ARM] = CHANNEL_MAX_100; /* CHANNEL_MAX_100 = 1844 */
        nCurrentMSPArmState = MSP_ARMED;
//        Serial.printf("Button ARMED\n");
      } else if( nCurrentMSPArmState == MSP_ARMED ) {
        gunChannelData[DISARM] = CHANNEL_MAX_100; /* CHANNEL_MAX_100 = 1844 */
        nCurrentMSPArmState = MSP_DISARMED;
//        Serial.printf("Button DisARMED\n");        
      }
      JS_Button_Event = 0;
#ifdef DEBUG_NK
      if( gunDebugMask & NK_DEBUG_DEBUG ) {
        Serial.printf("%d ) ## NK Button Event On !!\n", micros());
      }
#endif // DEBUG_NK
    }
  } else {
//    Serial.printf("JS_Button_Event : %d,  nCurrentMSPArmState : %d\n", JS_Button_Event, nCurrentMSPArmState);
    if( nCurrentMSPArmState == MSP_ARMED ) {
      gunChannelData[ARM] = CHANNEL_MIN_100; /* CHANNEL_MIN_100 = 204 */
//      Serial.printf("Button ARMED released\n");
    } else if( nCurrentMSPArmState == MSP_DISARMED ) {
      gunChannelData[DISARM] = CHANNEL_MIN_100; /* CHANNEL_MIN_100 = 204 */    
//      Serial.printf("Button DISARMED released\n");      
    }
    if( gunDebugMask & NK_DEBUG_DEBUG ) {
//        Serial.printf("%d ) ## NK Button Event Off !!\n", micros());
    }
  }
#endif /* JOYSTICK_BUTTON */
}
