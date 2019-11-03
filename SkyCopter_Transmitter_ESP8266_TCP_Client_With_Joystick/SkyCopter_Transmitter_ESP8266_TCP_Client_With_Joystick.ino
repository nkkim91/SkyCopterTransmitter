/*************************************************************************************
 * 
 * Once the debug mask is enabled and debug message is printed, the loop cycle time increases and consequently, increase the delay for TX.
 * So, it's recommended to enable the mask, only necessary
 * 
 *************************************************************************************/

#include <Arduino.h>
#include "StreamData.h"
#include "config.h"

#include <SPI.h>
#include <ESP8266WiFi.h>

#define BAR_LED0  (D6)  // 4
#define BAR_LED1  (D7)  // 0
#define BAR_LED2  (D8)  // 2

#define TX_PERIOD (11)    // 11 ms, smaller number is not working because of the operation time in the loop
#define DBG_LED_PERIOD (1000)
#define DBG_PERIOD  (100)

#define MSP_SET_RAW_RC_TINY_MSG_SZ  (11)
#define MSP_ARM_MSG_SZ  (18)
#define MSP_DISARM_MSG_SZ  (18)

#define RX_BUF_SZ     (64) // decrease the buffer size from 64 to 32 from the experiment


const char* ssid     = "FaryLink_031C56";
const char* password = "";

const char* host = "192.168.4.1";
const uint16_t port = 4143;

char inData[RX_BUF_SZ]; // Allocate some space for the string
int SerialBufferCnt = 0; // Index into array; where to store the character

unsigned long ulBarLedState = 0;
unsigned int unRet;

WiFiClient client;


#ifdef DEBUG_NK
/***************************************************
 *
 * Debug MASK Data structure / Macro       
 *
 ***************************************************/

#define NK_DBG_TRANSMITTER (1 << 14)
#define NK_DBG_ADS1115     (1 << 15)
#define NK_DBG_JS          (1 << 16)
 
#define NK_DEBUG_SDBG_LED_PERIODPOT  (1 << 1)
#define NK_DEBUG_INFO  (1 << 2)
#define NK_DEBUG_DEBUG  (1 << 3)
#define NK_DEBUG_DELTA (1 << 4)

//static uint32_t  gunDebugMask = NK_DEBUG_INFO | NK_DEBUG_SPOT | NK_DEBUG_DELTA | NK_DEBUG_DEBUG;
//static uint32_t  gunDebugMask = NK_DEBUG_DEBUG | NK_DEBUG_DELTA;
//static uint32_t  gunDebugMask = NK_DEBUG_DEBUG | NK_DEBUG_SPOT;
//static uint32_t  gunDebugMask = NK_DEBUG_DEBUG | NK_DEBUG_INFO;
static uint32_t  gunDebugMask = NK_DEBUG_INFO;
#endif

unsigned long ulLastTxTime = 0;
unsigned long ulLastDbgLedUpdateTime = 0;
unsigned long ulLastDbgUpdateTime = 0;
union TxStreamData stTxStreamData;

unsigned int    gunChannelData[MAX_TX_CHANNEL] = {             // 16
                            0x0400,  0x0400,  0x00CC,  0x0400,
                            0x00CC,  0x0400,  0x00CC,  0x00CC,
                            0x00CC,  0x0400,  0x0400,  0x0400,
                            0x0400,  0x0400,  0x0400,  0x0400,
                            };

unsigned long ulLastIRQTime = millis();

#define MSP_CMD_SET_RAW_RC_TINY (0x96)
#define MSP_CMD_ARM             (0x97)
#define MSP_CMD_DISARM          (0x98)

#define MSP_CMD_TRIM_UP         (0x99)
#define MSP_CMD_TRIM_DOWN       (0x9A)
#define MSP_CMD_TRIM_LEFT       (0x9B)
#define MSP_CMD_TRIM_RIGHT      (0x9C)

#define MSP_CMD_SKYCOPTER       (0xC7)

unsigned char TX_MSG[MAX_MSG_BUF_SIZE];

unsigned long ullCounter = 0;
//unsigned long ullErrorCounter = 0;
//unsigned long ullCurrentmS = 0;

unsigned char ucMSP_Header[] = { '$', 'M', '<' };

#ifdef SKYCOPTER
unsigned char ucMSP_SET_RAW_RC_TINY[] = { '$', 'M', '<', 0x05, MSP_CMD_SET_RAW_RC_TINY, 0x7D, 0x7D, 0x7D, 0x00, 0x55, 0xBB };
#elif defined(AIRCOPTER)
unsigned char ucMSP_SET_RAW_RC_TINY[] = { '$', 'M', '<', 0x05, MSP_CMD_SET_RAW_RC_TINY, 0x00, 0x00, 0x00, 0x00, 0x05, 0xEB };
#endif

unsigned char ucMSP_ARM[] = { '$', 'M', '<', 0x00, MSP_CMD_ARM, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };  
unsigned char ucMSP_DISARM[] = { '$', 'M', '<', 0x00, MSP_CMD_DISARM, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };  
unsigned char ucMSP_Dummy[] = { 'A' };


unsigned char CalcMSPCRC(unsigned char *ucData, int nLength)
{
  int i;
  unsigned char ucCRC;
  
  for(i = 1, ucCRC = ucData[0]; i < nLength; i++) {
    ucCRC ^= ucData[i];
#ifdef DEBUG_NK
  // within your code, wherever necessary:
    if( gunDebugMask & NK_DEBUG_DEBUG ) {
      Serial.printf("0x%02x ", ucData[i]);
    }
#endif // DEBUG_NK
  }

#ifdef DEBUG_NK
  // within your code, wherever necessary:
  if( gunDebugMask & NK_DEBUG_DEBUG ) {
    Serial.printf("\n");
  }
#endif // DEBUG_NK

  return ucCRC;
}

void setup() {

  Serial.begin(115200);

  pinMode(BAR_LED0, OUTPUT);
  pinMode(BAR_LED1, OUTPUT);
  pinMode(BAR_LED2, OUTPUT);

  digitalWrite(BAR_LED0, LOW);
  digitalWrite(BAR_LED1, LOW);
  digitalWrite(BAR_LED2, LOW);

  for(int i = 0; i < 10; i++) {
    digitalWrite(BAR_LED0, HIGH);
    digitalWrite(BAR_LED1, HIGH);
    digitalWrite(BAR_LED2, HIGH);
    delay(100);
    digitalWrite(BAR_LED0, LOW);
    digitalWrite(BAR_LED1, LOW);
    digitalWrite(BAR_LED2, LOW);
    delay(100);
  }

  ADS1115_Setup();
  JS_Setup();
#if 0
  JS_Calibration();
#else
  ADS1115_Calibration();
#endif

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

#ifdef DEBUG_NK
  if( gunDebugMask & NK_DEBUG_INFO ) {
    Serial.printf("setup() done !!\n");
  }
#endif // DEBUG_NK
}

void loop() {

  unsigned char ucCRC;
  unsigned char ucCmd;
  unsigned char ucMsgLength;
  unsigned char *pucMSPMsg;


  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    Serial.println("wait 5 sec...");
    delay(5000);
    return;
  }

  while( client.connected() ) {

    /* 
     * Still have no idea why the line below helps the external interrupt to exit normally without stuttering.
     * Without this, it takes abnromally long time to resume after handling the ISR.
     */
    T2L = 0;

    JS_Input();
  
    if( gunChannelData[ARM] == CHANNEL_MAX_100 ) {
  
      ucCRC = CalcMSPCRC(&(ucMSP_ARM[MSP_ARM_SIZE]), MSP_ARM_CRC_SZ);  //
      ucMSP_ARM[MSP_ARM_CRC] = ucCRC;
  
      ucCmd = MSP_ARM;
      ucMsgLength = MSP_ARM_MSG_SZ;
      
    } else if ( gunChannelData[DISARM] == CHANNEL_MAX_100 ) {
  
      ucCRC = CalcMSPCRC(&(ucMSP_DISARM[MSP_DISARM_SIZE]), MSP_DISARM_CRC_SZ);  //
      ucMSP_DISARM[MSP_DISARM_CRC] = ucCRC;
  
      ucCmd = MSP_DISARM;
      ucMsgLength = MSP_DISARM_MSG_SZ;
  
    } else {
      ucMSP_SET_RAW_RC_TINY[MSP_SET_RAW_RC_TINY_AILERON] = gunChannelData[AILERON];
      ucMSP_SET_RAW_RC_TINY[MSP_SET_RAW_RC_TINY_ELEVATOR] = gunChannelData[ELEVATOR];
      ucMSP_SET_RAW_RC_TINY[MSP_SET_RAW_RC_TINY_RUDDER] = gunChannelData[RUDDER];
      ucMSP_SET_RAW_RC_TINY[MSP_SET_RAW_RC_TINY_THROTTLE] = gunChannelData[THROTTLE];
  
      ucCRC = CalcMSPCRC(&(ucMSP_SET_RAW_RC_TINY[MSP_SET_RAW_RC_TINY_SIZE]), MSP_SET_RAW_RC_TINY_CRC_SZ);  //
      ucMSP_SET_RAW_RC_TINY[MSP_SET_RAW_RC_TINY_CRC] = ucCRC;
  
      ucCmd = MSP_SET_RAW_RC_TINY;
      ucMsgLength = MSP_SET_RAW_RC_TINY_MSG_SZ;
    }
  
    switch(ucCmd) {
      case MSP_ARM :
        pucMSPMsg = ucMSP_ARM;
        break;
  
      case MSP_DISARM :
        pucMSPMsg = ucMSP_DISARM;
        break;
  
      case MSP_SET_RAW_RC_TINY :
        pucMSPMsg = ucMSP_SET_RAW_RC_TINY;
        break;
  
      default :
        pucMSPMsg = ucMSP_Dummy; ucMsgLength = 1;
        break;
    }

#if 1
    ulBarLedState++;        
    if( !(ulBarLedState % 10) ) {
      digitalWrite(BAR_LED0, HIGH);
    } else {
      digitalWrite(BAR_LED0, LOW);
    }
    if( !(ulBarLedState % 100) ) {
      digitalWrite(BAR_LED1, HIGH);
    } else {
      digitalWrite(BAR_LED1, LOW);
    }
    if( !(ulBarLedState % 1000) ) {
      digitalWrite(BAR_LED2, HIGH);
    } else {
      digitalWrite(BAR_LED2, LOW);
    }
#endif
  
//    ullCurrentmS = millis();

    unRet = client.write(pucMSPMsg, ucMsgLength);
    if( unRet != ucMsgLength)
      Serial.printf("Packet write error !! (%d/%d written)\n", unRet/ucMsgLength);

    ullCounter++;

    if( millis() - ulLastDbgLedUpdateTime >= DBG_LED_PERIOD ) {
      ToggleDebugLed();
      ulLastDbgLedUpdateTime = millis();
//      Serial.printf("Loop count  : %d/s\n", ullCounter);
      ullCounter = 0;
    }

    // seems it has nothting to do with the flight control
//    delayMicroseconds(5000);
  }
}
