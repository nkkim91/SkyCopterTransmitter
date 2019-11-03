struct ADS1115_RawData {
  
  uint16_t u16Adc0;
  uint16_t u16Adc1;
  uint16_t u16Adc2;
  uint16_t u16Adc3;
  
};

struct ADS1115_nVoltaData {
  
  unsigned int unVoltage0; 
  unsigned int unVoltage1;
  unsigned int unVoltage2;
  unsigned int unVoltage3;
  
};

struct ADS1115_fVoltData {
  
  float fVoltage0; 
  float fVoltage1;
  float fVoltage2;
  float fVoltage3;
  
};

