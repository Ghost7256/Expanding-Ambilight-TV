#include <Arduino.h>
#include <ArduinoJson.h> // <- Ezt a libraryt le kell tölteni! "ArduinoJson by Benoit Blanchon 6.20.1"
#include <FastLED.h>  // <- Ezt a libraryt le kell tölteni! "FastLED by Daniel Garcia 3.5.0"

#ifdef ARDUINO_ARCH_ESP32
  #include <WiFi.h>
  #include <WiFiMulti.h>
  #include <HTTPClient.h>
#else //ESP8266 boards
  #include <ESP8266WiFiMulti.h>
  #include <ESP8266HTTPClient.h>
  #define FASTLED_ESP8266_RAW_PIN_ORDER
#endif


#define LED_PINL 2  // <-- ESP KIMENETI LÁBA (GPIO láb, nem a D jelölésű)
#define LED_PINR 16  // <-- ESP KIMENETI LÁBA (GPIO láb, nem a D jelölésű)
#define NUM_LEDSL 240 // Ledek száma a szalagon bal
#define NUM_LEDSR 240 // Ledek száma a szalagon jobb
#define BRIGHTNESS 30 // Fényerő 0-255 ig
#define UPDATES_PER_SECOND 30 
unsigned long tvupdate = 100; // Lekérési időköz a TV-ről (ms)


CRGB ledsL[NUM_LEDSL];
CRGB ledsR[NUM_LEDSR];

#if ARDUINO_ARCH_ESP32
  WiFiMulti wifiMulti;
#else
  ESP8266WiFiMulti wifiMulti;
#endif

unsigned long lastMillis = 0;
unsigned long currentMillis = 0;

int counter = 0;

const char* ssid = "ssid"; // WIFI hálózat neve
const char* password = "password"; // WIFI jelszó
const char* AmbilightSource = "http://192.168.1.178:1925/6/ambilight/processed"; //"http://192.168.1.XXX:1925/6/ambilight/processed"; TV IP-JE

int left_rgb[3]  ={0, 0, 0}; 
int top_rgb[3]   ={0, 0, 0}; 
int right_rgb[3] ={0, 0, 0};

void setup() {

  Serial.begin(115200);

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  FastLED.addLeds<WS2812, LED_PINL, GRB>(ledsL, NUM_LEDSL);
  LEDS.setBrightness(BRIGHTNESS);

  wifiMulti.addAP(ssid, password);


}

void loop() {
  currentMillis = millis();

  if (currentMillis - lastMillis >= tvupdate) {
    lastMillis = currentMillis;

  // wait for WiFi connection
    if ((wifiMulti.run() == WL_CONNECTED)) {
      WiFiClient client;
      HTTPClient httpAmbilight;
      //Serial.println("[HTTP] begin...");
      httpAmbilight.begin(client, AmbilightSource); //HTTP

      //Serial.println("[HTTP] GET...");
      // start connection and send HTTP header
      int httpCode = httpAmbilight.GET();
      //Serial.print("HTTP Code: ");
      //Serial.println(httpCode);
      // httpCode will be negative on error
      if (httpCode > 0) {
        //if source reached without error:
       if (httpCode == HTTP_CODE_OK) {

          String payload = httpAmbilight.getString();
          //Serial.println(payload);
          DynamicJsonDocument doc(1024);
          deserializeJson(doc, payload);
          JsonObject layer1 = doc["layer1"];
          JsonObject layer1_left = layer1["left"];   
          JsonObject layer1_left_1 = layer1_left["1"];
          left_rgb[0] = layer1_left_1["r"]; //RED value of the left-center LED
          left_rgb[1] = layer1_left_1["g"]; //GREEN value of the left-center LED
          left_rgb[2] = layer1_left_1["b"]; //BLUE value of the left-center LED

          for (int i = 0; i <= NUM_LEDSL -1; i++) {
            ledsL[i] = CRGB ( left_rgb[0], left_rgb[1], left_rgb[2]);
          }


          JsonObject layer1_right = layer1["right"];       
          JsonObject layer1_right_1 = layer1_right["1"];
          right_rgb[0] = layer1_right_1["r"]; //RED value of the right-center LED
          right_rgb[1] = layer1_right_1["g"]; //GREEN value of the right-center LED
          right_rgb[2] = layer1_right_1["b"]; //BLUE value of the right-center LED         
        
          for (int i = 0; i <= NUM_LEDSR -1; i++) {
            ledsR[i] = CRGB ( right_rgb[0], right_rgb[1], right_rgb[2]);
          }

       // JsonObject layer1_top = layer1["top"];
        
       // JsonObject layer1_top_3 = layer1_top["3"];
       // top_rgb[0] = layer1_top_3["r"]; //RED value of the top-center LED
       // top_rgb[1] = layer1_top_3["g"]; //GREEN value of the top-center LED
       // top_rgb[2] = layer1_top_3["b"]; //BLUE value of the top-center LED

       
        }
      }
      else {
        FastLED.clear();
        Serial.printf("[HTTP] GET... failed, error: %s\n", httpAmbilight.errorToString(httpCode).c_str());
      }
      httpAmbilight.end();
    }else {
      FastLED.clear();
      Serial.println("Not Connected");
      delay(1000);
    }
  }
  FastLED.show();

  delay(50);
}
