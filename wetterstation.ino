//
//
// Arduino Wetterstation:
//
// Sensoren:
// - Temperatur + Feuchtigkeit mit "DTH22"
//   + Innen:  PIN  9
//   + Aussen: PIN 10
//
// - Luftdruck über BMP085
//   + I2C PIN-A4/-A5
//
// - Blitze über AS3935
//   + I2C PIN-A4/-A5
//
// - Real-Time Clock / EEPROM (32k):
//   + I2C PIN-A4/-A5
//
// Output:
// - LCD
//   + PIN 2-7
//
// - Ethernet
//   + VCC -   3.3V
//   + GND -    GND
//   + SCK - Pin 13
//   + SO  - Pin 12
//   + SI  - Pin 11
//   + CS  - Pin  8
//
// - Serial
//   + PIN J,K
//

#include <LiquidCrystal.h>
int offset = 2;
LiquidCrystal lcd( 4 + offset, 5 + offset, 0 + offset
                 , 1 + offset, 2 + offset, 3 + offset);
char dataString[7];  // gelesene Temperatur als String aufbereitet: (-xx)x.x

/*
#include <Wire.h> // I2C
#include "LM75.h"    // Temperature sensors
double temp;         // gelesene Temperatur als double
*/

#include "BMP085.h"  // Preasure sensor
BMP085 bmp;          // Global object from BMP085

#include "DHT.h"
DHT dht_inside ( 9, DHT11);  // Humidity + Temperature inside
float last_inside_temp = 0;
DHT dht_outside(10, DHT22);  // Humidity + Temperature outside

/*
#include "EtherCard.h"
// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
static byte myip[] = { 192,168,1,203 };

byte Ethernet::buffer[500];
BufferFiller bfill;
uint32_t timer;
Stash stash;

// change these settings to match your own setup
char website[] PROGMEM = "api.pachube.com";
#define APIKEY  "Zk_xpOV65l7VgnA2INRbqxJnG7ySAKxBeVR2YjhaVHhWaz0g"
#define FEED    "105082"
*/

void setup() { 

  Serial.begin(9600);
  
  dht_inside.begin();
  dht_outside.begin();
  
  // Initial value for cont. average
  last_inside_temp = dht_inside.readTemperature();
  
/*  
  //
  // Ethernet setup
  //
  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) {
    Serial.println( "Failed to access Ethernet controller");
  } else {
    Serial.println( "Sucess to access Ethernet controller");
  }
  if (!ether.dhcpSetup()) {
    Serial.println("DHCP failed");
    ether.staticSetup(myip);
  } else {
    Serial.println("DHCP successful");
  }    
  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);  
  ether.printIp("DNS: ", ether.dnsip);  
*/
  //
  // BMP085 setup
  //
  if (!bmp.begin()) {
	Serial.println("Could not find a valid BMP085 sensor, check wiring!");
  }  
  delay(1000);
  Serial.println("Start Messung...");

  //
  // LCD size setup
  //
  lcd.begin(16, 2);
} 

/*
void readPrintLM75(int device, int line, int offset) {

  temp = get_LM75_temperature(device, TEMP); // (Device)Wert vom 1. Temperatursensor lesen (0-7, je nach Jumperstellung am Board, 2. Parameter wie oben definiert)
  dtostrf(temp, 4, 1, dataString);           // dtostrf(floatVar, minStringWidthIncDecimalPoint, numVarsAfterDecimal, charBuf); (standard avr-libc function)
  Serial.print("LM75 Temperatur (");
  Serial.print(device);
  Serial.print("): ");
  Serial.println(dataString);
  
  lcd.setCursor (offset, line);
  lcd.print("    ");
  lcd.setCursor (offset, line);
  lcd.print(dataString);  
  lcd.print((char)223);  
}

void readPrintBMA085(int line, int offset) {
  
  temp = bmp.readTemperature();
  dtostrf(temp, 4, 1, dataString);  
  Serial.print("BMA085 Temperature: ");
  Serial.println(dataString);
  
  temp  = bmp.readPressure();
  temp /= 100;
  temp += 65; // Korrektur für Bern
  dtostrf(temp, 4, 2, dataString);  
  Serial.print("BMA085 Preasure: ");
  Serial.println(dataString);

  lcd.setCursor (offset, line);
  lcd.print(dataString);  

}
*/

void setLCD(String line_one, String line_two)
{
  // 1st line
  lcd.setCursor (0,0);
  lcd.print(line_one);

  // 2nd line
  lcd.setCursor (0,1);
  lcd.print(line_two);
}

void printTemp(float value, int line)
{
  dtostrf(value, 4, 1, dataString);  
  lcd.setCursor (10, line);
  lcd.print(dataString);  
  lcd.print((char)223);  
}

void printHumidity(float value, int line)
{
  dtostrf(value, 4, 1, dataString);  
  lcd.setCursor (10, line);
  lcd.print(dataString);  
  lcd.print("%");  
}

void printPreasure(float value, int line)
{
  dtostrf(value, 4, 2, dataString);  
  lcd.setCursor (10, line);
  lcd.print(dataString);  
}

void loop()  
{  

  // --------------- Read all values ------------------  
  // Reading temperature or humidity takes about 250 ms
  // Sensor readings may also be up to 2 seconds 'old',
  // its a very slow sensor.
  
  float inside_humidity     = dht_inside.readHumidity();
  float inside_temperature  = dht_inside.readTemperature();
  
  // Increase DHT11 precision with running average
  inside_temperature = 0.2*inside_temperature + 0.8*last_inside_temp;
  last_inside_temp   = inside_temperature;  

  float outside_humidity    = dht_outside.readHumidity();
  float outside_temperature = dht_outside.readTemperature();

  float preasure = bmp.readPressure();
  preasure /= 100;
  preasure +=  65;


  // --------------- Debug all values ---------------
  Serial.print("Humidity outside: ");
  Serial.println(outside_humidity);

  Serial.print("Temperature outside: ");
  Serial.println(outside_temperature);

  Serial.print("Humidity inside: ");
  Serial.println(inside_humidity);

  Serial.print("Temperature inside: ");
  Serial.println(inside_temperature);

  Serial.print("Preasure: ");
  Serial.println(preasure);


  // --------------- Display all values ---------------
  setLCD("Hasihaus:       ", "Aussen:         ");
  printTemp(inside_temperature,  0);
  printTemp(outside_temperature, 1);
  delay(2000);

  setLCD("Hasihaus:       ", "Aussen:         ");
  printHumidity(inside_humidity,  0);
  printHumidity(outside_humidity, 1);
  delay(2000);

  setLCD("Druck:          ", "                ");
  printTemp(preasure, 0);
  delay(2000);
/*
  setLCD("Regen:      23mm", "Blitze:      24x");
  delay(500);
  setLCD("Helligkeit: xxxx", "                ");
  delay(500);
  setLCD("W.richtung:   <-", "W.stärke: 100kmh");
  delay(500);
*/

/*
  // Serve homepage
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);

  if (pos)  // check if valid tcp data is received
    ether.httpServerReply(homePage()); // send web page data

    // generate two fake values as payload - by using a separate stash,
    // we can determine the size of the generated message ahead of time
    byte sd = stash.create();
    stash.print("0,");
    stash.println((word) millis() / 123);
    stash.print("1,");
    stash.println((word) micros() / 456);
    stash.print("2,");
    stash.println((word) micros() / 789);
    stash.save();
    
    // generate the header with payload - note that the stash size is used,
    // and that a "stash descriptor" is passed in as argument using "$H"
    Stash::prepare(PSTR("PUT http://$F/v2/feeds/$F.csv HTTP/1.0" "\r\n"
                        "Host: $F" "\r\n"
                        "X-PachubeApiKey: $F" "\r\n"
                        "Content-Length: $D" "\r\n"
                        "\r\n"
                        "$H"),
            website, PSTR(FEED), website, PSTR(APIKEY), stash.size(), sd);

    // send the packet - this also releases all stash buffers once done
    ether.tcpSend();

/*/
}

/*
static word homePage() {
  long t = millis() / 1000;
  word h = t / 3600;
  byte m = (t / 60) % 60;
  byte s = t % 60;
  bfill = ether.tcpOffset();
  bfill.emit_p(PSTR(
    "HTTP/1.0 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Pragma: no-cache\r\n"
    "\r\n"
    "<meta http-equiv='refresh' content='1'/>"
    "<title>RBBB server</title>" 
    "<h1>$D$D:$D$D:$D$D</h1>"),
      h/10, h%10, m/10, m%10, s/10, s%10);
  return bfill.position();
}
*/
