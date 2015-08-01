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

#ifndef ESP8266
//
// Define connection between Arduino and ESP8266
//
// #include <Adafruit_ESP8266.h>

#define ARD_RX_ESP_TX   10
#define ARD_TX_ESP_RX   11
#define ESP_RST         12

#include <SoftwareSerial.h>
SoftwareSerial softser(ARD_RX_ESP_TX, ARD_TX_ESP_RX); // Arduino RX = ESP TX, Arduino TX = ESP RX

// Must declare output stream before Adafruit_ESP8266 constructor; can be
// a SoftwareSerial stream, or Serial/Serial1/etc. for UART.
Adafruit_ESP8266 wifi(&softser, &Serial, ESP_RST);

#define LED_PIN         13
#else
#include <ESP8266WiFi.h>
#endif // ESP8266

#include <Phant.h>
// Arduino example stream
// hostname, public key, private key
// http://data.sparkfun.com/streams/g6Mbja4o52IgVRjaY85j
// "g6Mbja4o52IgVRjaY85j", "qz9lrjWdqkUemWXlpyZX"

#include "ssids.h"

const char* host = "data.sparkfun.com";
Phant phant(host, PHANT_PUBLIC_KEY, PHANT_PRIVAT_KEY);

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

  Serial.begin(115200); while(!Serial); // UART serial debug

#ifndef ESP8266
  // Flash LED on power-up
  pinMode(LED_PIN, OUTPUT);
  for(uint8_t i=0; i<3; i++) {
    digitalWrite(13, HIGH); delay(50);
    digitalWrite(13, LOW);  delay(100);
  }
#endif // ESP8266

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
#ifndef ESP8266
  // This might work with other firmware versions (no guarantees)
  // by providing a string to ID the tail end of the boot message:
  
  // comment/replace this if you are using something other than v 0.9.2.4!
  wifi.setBootMarker(F("Version:0.9.2.4]\r\n\r\nready"));

  softser.begin(9600); // Soft serial connection to ESP8266
#else
  // We start by connecting to a WiFi network
 
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ESP_SSID);
  
  WiFi.begin(ESP_SSID, ESP_PASS);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
#endif
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

#ifndef ESP8266
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
#else
  String url = (char*)phant.url().substring(7+17).c_str();
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
   // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, 80)) {
    Serial.println("connection failed");
    return;
  }
  
  String request = "GET " + url + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" + 
                   "Connection: close\r\n\r\n";
                   
  Serial.println(request);
  
  // This will send the request to the server
  client.print(request);
  delay(10);
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
#endif

  Serial.print(F("end of cycle "));
  Serial.println(cycle++);
  Serial.println(F("before delay"));
  delay(53000);  
  Serial.println(F("after delay"));
}
