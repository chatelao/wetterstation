/*------------------------------------------------------------------------
  Simple ESP8266 test.  Requires SoftwareSerial and an ESP8266 that's been
  flashed with recent 'AT' firmware operating at 9600 baud.  Only tested
  w/Adafruit-programmed modules: https://www.adafruit.com/product/2282

  The ESP8266 is a 3.3V device.  Safe operation with 5V devices (most
  Arduino boards) requires a logic-level shifter for TX and RX signals.
  ------------------------------------------------------------------------*/

//     
//     ----------------------     ---------------------------
//     |                 SCL|->---| I2C Sensoren:           |
//     |    Arduino      SDA|<>---| Blitzmesser, Druck, ... |
//     |      UNO       AREF|     ---------------------------
//     |                 GND|
//     |reserved          13|                                   \ | /        
//     |IOREF             12|->---/RST WLAN-Module \             \|/         
//     |RESET           ~ 11|->---|RX  ESP8266     |              |          
//     |3.3V            ~ 10|<----\TX              /--------------.          
//     |5.0V            ~  9|->---/EN--------------\
//     |GND                8|->---|RS  LCD         |
//     |GND                 |     |    HD44780     |
//     |Vin                7|->---|D7              |
//     |                ~  6|->---|D6              |
//     |A0              ~  5|->---|D5              |
//     |A1                 4|->---\D4--------------/
//     |A2              ~  3|<----<Regenmesser via HALL-Sensor>
//     |A3                 2|<----<DHT22 - Feuchtigkeit / Temperatur>
//     |A4 SDA          TX 1|->---/RX  PC          \
//     |A5 SCL  ICSP    RX 0|<----\TX  ArduinoIDE  /
//     ----------------------
//

#include "Adafruit_ESP8266.h"
#include <SoftwareSerial.h>

#define LED_PIN         13

#define ARD_RX_ESP_TX   10
#define ARD_TX_ESP_RX   11
#define ESP_RST         12
SoftwareSerial softser(ARD_RX_ESP_TX, ARD_TX_ESP_RX); // Arduino RX = ESP TX, Arduino TX = ESP RX

// Must declare output stream before Adafruit_ESP8266 constructor; can be
// a SoftwareSerial stream, or Serial/Serial1/etc. for UART.
Adafruit_ESP8266 wifi(&softser, &Serial, ESP_RST);

#include "Phant.h"
// Arduino example stream
// hostname, public key, private key
// http://data.sparkfun.com/streams/g6Mbja4o52IgVRjaY85j
// "g6Mbja4o52IgVRjaY85j", "qz9lrjWdqkUemWXlpyZX"

#define ESP_SSID "mynetwwork" // Your network name here
#define ESP_PASS "mysecret"   // Your network password here
#define PHANT_PUBLIC_KEY "phantpublickey"
#define PHANT_PRIVAT_KEY "phantprivatekey"

#define DHTPIN A1 // DHT-Sensor
#define DHTVCC A0 // DHT-VCC
#define DHTGND A3 // DHT-Ground

#include "ssids.h"

Phant phant("data.sparkfun.com", PHANT_PUBLIC_KEY, PHANT_PRIVAT_KEY);

#include <DHT.h>
#define DHTTYPE DHT22     // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor for normal 16mhz Arduino

int cycle = 0;

//
// Regenmesser
//
unsigned long _lastRainMillis = millis();
unsigned long _nbreRain       = 0;
unsigned long _startRain   = millis();
//
// Interrupt driven raincounter
//
void countRain() {
  
  // Debounce with ms-Overflow detection
  if(_lastRainMillis + 100 < millis() || millis() < _lastRainMillis) {
    _nbreRain++;
    _lastRainMillis = millis();
  }
}

void setup() {

  char buffer[50];

  // Flash LED on power-up
  pinMode(LED_PIN, OUTPUT);
  for(uint8_t i=0; i<3; i++) {
    digitalWrite(13, HIGH); delay(50);
    digitalWrite(13, LOW);  delay(100);
  }

  attachInterrupt(1, countRain, RISING); // Pin 3
  
#ifdef DHTVCC
  pinMode(DHTVCC, OUTPUT);
  digitalWrite(DHTVCC, HIGH);
#endif
#ifdef DHTGND
  pinMode(DHTGND, OUTPUT);
  digitalWrite(DHTGND, LOW);
#endif


  dht.begin();

  // This might work with other firmware versions (no guarantees)
  // by providing a string to ID the tail end of the boot message:
  
  // comment/replace this if you are using something other than v 0.9.2.4!
  wifi.setBootMarker(F("Version:0.9.2.4]\r\n\r\nready"));

  softser.begin(9600); // Soft serial connection to ESP8266
  Serial.begin(57600); while(!Serial); // UART serial debug

}

void loop() {

  Serial.println("start loop");
  
  // periode:    (seit letztem Aufruf)
  // rain_cnt:   Anzahl Regenimpulse
  // rain_pp:    Regenimpulse gewichtet pro Minute
  // humidity:   xx%
  // temperatur: xx%
  // flash_cnt:  0
  // flash_pp:   0
  
  phant.add(F("humidity"), dht.readHumidity());
  phant.add(F("temp"), dht.readTemperature());
  phant.add(F("flashes"),"0");
  phant.add(F("raincount"), _nbreRain);
#ifndef WETTERSTATION_BERN
  phant.add(F("duration"), millis()-_startRain);
#endif
  phant.add(F("timestamp"),"");

   wifi.hardReset();
   wifi.softReset();   
   wifi.connectToAP(F(ESP_SSID), F(ESP_PASS));
   wifi.connectTCP(F("data.sparkfun.com"), 80);

   if(wifi.requestURL((char*)phant.url().substring(7+17).c_str())) {
     Serial.println(F("TCP send successful"));
     _nbreRain = 0;
     _startRain = millis();
   } else {
     Serial.println(F("TCP send failed"));
   }

   wifi.closeTCP();
   wifi.closeAP();

  Serial.print(F("end of cycle "));
  Serial.println(cycle++);
  Serial.println(F("before delay"));
  delay(53000);  
  Serial.println(F("after delay"));
}
