#include "LM75.h"

// LM75 Temperatur auslesen. Device = 0-7, regx = TEMP, OBEN, UNTEN (Registerauswahl)  
double get_LM75_temperature(int device, int regx) 
{
  int8_t  msb;
  int8_t  lsb;
  int8_t  msb1;
  Wire.beginTransmission(SensorAdresse + device);
  Wire.write(regx);
  Wire.endTransmission();
  Wire.beginTransmission(SensorAdresse + device);
  Wire.requestFrom(SensorAdresse + device, 2); 
  if (Wire.available()) {
     msb1 = Wire.read();
     msb = msb1 & 0x7F; // Vorzeichenbit entfernen
     lsb = Wire.read();
  }
  // höchstes bit von lsb sagt aus, ob 0,5 Grad dazu addiert werden sollen
  Wire.endTransmission();
  if ((msb1 & 0x80) != 0x80) { // Positiver Wert?
    return double(msb) + double(lsb)/256; // positiver Wert
  }  
  else {
    return double(msb) + double(lsb)/256 - 128.0; // negativer Wert
  }  
}

// LM75 Konfigurationsregister setzen, Werte wie oben definiert
void set_LM75_config(int device, byte value) 
{
  Wire.beginTransmission(SensorAdresse + device);
  Wire.write(1); // Select Konfigurationsregister
  Wire.write(value);
  Wire.endTransmission();
}

// LM75 Konfigurationsregister auslesen, device = 0-7
byte get_LM75_config(int device) 
{
  byte reg;
  Wire.beginTransmission(SensorAdresse + device);
  Wire.write(1); // Select Konfigurationsregister
  Wire.endTransmission();
  Wire.requestFrom(SensorAdresse + device, 1);
  if (Wire.available()) { 
     reg = Wire.read();
  }
  Wire.endTransmission();
  return reg;
} 

// LM75 Schaltwerte setzen, device = 0-7, regx = Wert, Grad als double
void set_LM75_schaltwert(int device, byte regx, double grad) 
{
  int8_t msb;
  int8_t lsb = 0;
  uint8_t y = 0;
  boolean neg = false;
  if (grad < 0) {
    msb = abs(int(grad))+128;
  }
  else {  
    msb = abs(int(grad));
  }
  if (grad - abs(int(grad)) > 0) {
    lsb = 0x80;
  }
  Wire.beginTransmission(SensorAdresse + device);
  Wire.write(regx); // Selektiere oberes oder unteres Register
  Wire.write(msb);
  Wire.write(lsb);
  Wire.endTransmission();
}