// Anschlu� eines I�C-Temperatursensor mit LM75 von Horter & Kalb an Arduino 
// Bei den meisten Arduinos befindet sich der SDA (data line) an Analog Bin 4 und SCL (Clock line) an Analog Bin 5,
// bei Arduino Mega SDA an digital Pin 20 und SCL an digital Pin 21
// I2C wird �ber die Wire Library abgewickelt. Der angegebene Code ist f�r die Version 1.0 des Arduino Compilers (n�chste nach 23)
// In dieser Version wurde durch Vererbung von Streams.h die Funktion Wire.send durch die Funktion Wire.write ersetzt.
// Darauf ist zu achten, wenn man in einer �lteren Version compiliert.
// Es wurden alle Funktionen eingebaut und als Beispiel angef�hrt.
// Liest man nur die Temperatur aus, so kann auf den Gro�teil verzichtet werden.

#if (ARDUINO >= 100)
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif
#include "Wire.h"

#define SensorAdresse 0x48 // Basisadresse f�r ersten Temperatursensor

// Registerparameter (regx) fuer get_LM75_temperature
#define TEMP  0 // Temperaturregister anw�hlen
#define UNTEN 2 // Register f�r den unteren Schaltwert anw�hlen
#define OBEN  3 // Register f�r den oberen Schaltwert anw�hlen
double get_LM75_temperature(int device, int regx);

// LM75 Konfigurationsregister setzen, Werte wie oben definiert
void set_LM75_config(int device, byte value);


// LM75 Configuration Register Registeradresse: 1
// Bit 0: Stromsparmodus, bei 1 geht Temperaturin den Stromsparmodus (keine Messung, aber aktive Steuerung) Ausgang wird auch abgeschaltet
//                        bei 0 geht Temperatursensor aus dem Stromsparmodus (Messung) Ausgang wird wieder freigegeben  
// Bit 1: Interrupt Modus, bei 1 schaltet der Ausgang sowohl bei oberen als auch unteren Schwellwert ein, wird zur�ckgesetzt durch Auslesen des Registers
//                         bei 0 schaltet der Ausgang bei oberen Schaltpunkt ein und bei unteren aus (default 80�C / 75�C)
// Bit 2: OS-Pin bei 1 wird das Verhalten des Ausgangs invertiert, Ausgang ist eingeschalten innerhalb der Schwellwerte
//               bei 0 Ausgang schaltet bei �berschreiten der eingestellten Schwellwerte
// Bit 3 und 4: Wert 0-3, besagt wieviele Messzyklen abgewartet wird, bis Ausgang aktiv/inaktiv wird, wenn die Bedingung erf�llt ist (verhindert Flattern des Ausgangs)
// Bit 5-7 m�ssen 0 sein
// Byte: 7 6 5 4 3 2 1 0

