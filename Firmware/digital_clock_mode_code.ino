#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <RTClib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <WiFiClient.h>

#define LED_PIN D4
#define NUM_LEDS 130  
#define DIGITS 6
#define SEGMENTS 7
#define SEGMENT_LEDS 3
#define FIRST_COLON_INDEX 42
#define SECOND_COLON_INDEX 86
#define MINUTE_LEDS_START 44
#define SECOND_LEDS_START 88

const char* ssid = "Airtel_airtel";
const char* password = "Connected37";

WiFiUDP ntpUDP;
RTC_DS1307 rtc;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

uint8_t segmentMap[10][SEGMENTS] = {
  {1, 1, 1, 0, 1, 1, 1}, // 0
  {0, 0, 1, 0, 0, 0, 1}, // 1
  {1, 1, 0, 1, 0, 1, 1}, // 2
  {0, 1, 1, 1, 0, 1, 1}, // 3
  {0, 0, 1, 1, 1, 0, 1}, // 4
  {0, 1, 1, 1, 1, 1, 0}, // 5
  {1, 1, 1, 1, 1, 1, 0}, // 6
  {0, 0, 1, 0, 0, 1, 1}, // 7
  {1, 1, 1, 1, 1, 1, 1}, // 8
  {0, 1, 1, 1, 1, 1, 1}, // 9
};

WiFiClient client;

void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.show();
  
  Wire.begin(D2, D1); 
  
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC is not running, setting time...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); 
  }
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  DateTime now = rtc.now();
  Serial.print("Current Time: ");
  Serial.print(now.hour());
  Serial.print(":");
  Serial.print(now.minute());
  Serial.print(":");
  Serial.println(now.second());

  displayTime(now.hour(), now.minute(), now.second());
  delay(1000);
}

void displayTime(int hour, int minute, int second) {
  strip.clear();
  int digits[DIGITS] = {hour / 10, hour % 10, minute / 10, minute % 10, second / 10, second % 10};

  uint32_t digitColor = strip.Color(240, 0, 0);
  uint32_t colonColor = strip.Color(240, 0, 0);

  for (int i = 0; i < 2; i++) {
    displayDigit(digits[i], i * SEGMENTS * SEGMENT_LEDS, digitColor);
  }

  strip.setPixelColor(FIRST_COLON_INDEX, colonColor);
  strip.setPixelColor(FIRST_COLON_INDEX + 1, colonColor);

  for (int i = 2; i < 4; i++) {
    displayDigit(digits[i], MINUTE_LEDS_START + (i - 2) * SEGMENTS * SEGMENT_LEDS, digitColor);
  }

  strip.setPixelColor(SECOND_COLON_INDEX, colonColor);
  strip.setPixelColor(SECOND_COLON_INDEX + 1, colonColor);

  for (int i = 4; i < 6; i++) {
    displayDigit(digits[i], SECOND_LEDS_START + (i - 4) * SEGMENTS * SEGMENT_LEDS, digitColor);
  }

  strip.show();
}

void displayDigit(int number, int startIndex, uint32_t color) {
  for (int seg = 0; seg < SEGMENTS; seg++) {
    if (segmentMap[number][seg]) {
      for (int led = 0; led < SEGMENT_LEDS; led++) {
        strip.setPixelColor(startIndex + seg * SEGMENT_LEDS + led, color);
      }
    }
  }
}
