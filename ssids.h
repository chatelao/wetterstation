//
// All configuration parameters for the build
//
#define ESP_SSID "mynetwwork" // Your network name here
#define ESP_PASS "mysecret"   // Your network password here
#define PHANT_PUBLIC_KEY "phantpublickey"
#define PHANT_PRIVAT_KEY "phantprivatekey"

#ifdef ESP8266
#define DHTPIN  0          // What pin we're connected to
#else
#define DHTPIN A1 // DHT-Sensor
#define DHTVCC A0 // DHT-VCC
#define DHTGND A3 // DHT-Ground
#endif
