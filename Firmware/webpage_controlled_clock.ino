#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <RTClib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <WiFiClient.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>

#define LED_PIN D4
#define NUM_LEDS 130  
#define DIGITS 6
#define SEGMENTS 7
#define SEGMENT_LEDS 3
#define FIRST_COLON_INDEX 42
#define SECOND_COLON_INDEX 86
#define MINUTE_LEDS_START 44
#define SECOND_LEDS_START 88

const char* ssid = "ESP8266_Clock";
const char* password = "password123";

IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
AsyncWebServer server(80);

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

uint32_t digitColor = strip.Color(240, 0, 0);
uint32_t colonColor = strip.Color(240, 0, 0);

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
  
  // Set up WiFi
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  dnsServer.start(53, "*", apIP);
  
  // Serve the HTML page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", getHTML());
  });

  // Handle color updates
  server.on("/updateColors", HTTP_POST, [](AsyncWebServerRequest *request){
    String digitColorR = request->arg("digitColorR");
    String digitColorG = request->arg("digitColorG");
    String digitColorB = request->arg("digitColorB");
    String colonColorR = request->arg("colonColorR");
    String colonColorG = request->arg("colonColorG");
    String colonColorB = request->arg("colonColorB");

    digitColor = strip.Color(digitColorR.toInt(), digitColorG.toInt(), digitColorB.toInt());
    colonColor = strip.Color(colonColorR.toInt(), colonColorG.toInt(), colonColorB.toInt());
    request->send(200, "text/plain", "Colors updated");
  });

  // Handle time format updates
  server.on("/updateFormat", HTTP_POST, [](AsyncWebServerRequest *request){
    String format = request->arg("format");
    // Handle time format logic here
    request->send(200, "text/plain", "Time format updated to " + format);
  });

  // Handle setting mode
  server.on("/setMode", HTTP_POST, [](AsyncWebServerRequest *request){
    String mode = request->arg("mode");
    String customTime = request->arg("customTime");
    // Handle mode logic here
    request->send(200, "text/plain", "Mode set to " + mode + " with time " + customTime);
  });

  // Handle updating to current time
  server.on("/updateToCurrentTime", HTTP_POST, [](AsyncWebServerRequest *request){
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Update RTC to current time
    request->send(200, "text/plain", "Clock updated to current time");
  });

  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
  DateTime now = rtc.now();
  displayTime(now.hour(), now.minute(), now.second());
  delay(1000);
}

void displayTime(int hour, int minute, int second) {
  strip.clear();
  int digits[DIGITS] = {hour / 10, hour % 10, minute / 10, minute % 10, second / 10, second % 10};

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

String getHTML() {
  return R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <title>Clock Configuration</title>
    <style>
      body {
        font-family: Arial, sans-serif;
        background-image: url(https://png.pngtree.com/thumb_back/fh260/background/20240103/pngtree-vivid-abstract-texture-a-burst-of-colorful-background-image_13892083.png);
        background-size: cover;
        background-position: center;
        background-repeat: no-repeat;
        margin: 0;
        padding: 0;
        display: flex;
        justify-content: center;
        align-items: center;
        min-height: 100vh;
      }
      .container {
        text-align: center;
        background-image: url(https://i.pinimg.com/736x/7e/82/46/7e8246933f34cf596fe6041a76c294e2.jpg);
        background-size: cover;
        background-position: center;
        background-repeat: no-repeat;
        border: 2px solid #090000;
        border-radius: 10px;
        box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
        padding: 20px;
        max-width: 500px;
      }
      h1 { font-size: 32px; color: #000000; margin-bottom: 20px; }
      h3 { font-size: 18px; color: #000000; margin-bottom: 10px; }
      input[type="number"], input[type="time"], select, button {
        font-size: 16px;
        padding: 8px 9px;
        background-color: #a1c3eb;
        margin: 5px 0;
        border: 1px solid #030000;
        border-radius: 5px;
      }
      button {
        background-color: #3b4b5c;
        color: #ffffff;
        border: 1px solid #030000;
        cursor: pointer;
        transition: background-color 0.3s ease;
      }
      button:hover {
        background-color: #9500b3;
      }
      .form-group { margin-bottom: 15px; }
      .update-button { margin-top: 15px; }
    </style>
  </head>
  <body>
    <div class="container">
      <h1>CLOCK CONFIGURATION</h1>
      <h3>CHANGE COLORS</h3>
      <form id="colorForm">
        <div class="form-group">
          Digit Color:<br>
          RED: <input type="number" name="digitColorR" min="0" max="255" placeholder="0-255"> 
          GREEN: <input type="number" name="digitColorG" min="0" max="255" placeholder="0-255"> 
          BLUE: <input type="number" name="digitColorB" min="0" max="255" placeholder="0-255">
        </div>
        <div class="form-group">
          Colon Color:<br>
          R: <input type="number" name="colonColorR" min="0" max="255" placeholder="0-255"> 
          G: <input type="number" name="colonColorG" min="0" max="255" placeholder="0-255"> 
          B: <input type="number" name="colonColorB" min="0" max="255" placeholder="0-255">
        </div>
        <button type="button" onclick="updateColors()">Update Colors</button>
      </form>
      <h3>CHANGE TIME FORMAT</h3>
      <div class="form-group">
        <select id="timeFormat">
          <option value="24-hour">24-hour</option>
          <option value="12-hour">12-hour</option>
        </select>
        <button onclick="updateFormat()">Update Format</button>
      </div>
      <h3>SET TIME</h3>
      <div class="form-group">
        <input type="time" id="customTime">
      </div>
      <div>
        <button onclick="setMode('timer')">Timer</button>
        <button onclick="setMode('countdown')">Countdown</button>
      </div>
      <div class="update-button">
        <button onclick="updateToCurrentTime()">Update to Current Time</button>
      </div>
    </div>
    <script>
      function updateColors() {
        const form = document.getElementById('colorForm');
        const data = new FormData(form);
        fetch('/updateColors', { method: 'POST', body: data })
          .then(response => response.text())
          .then(data => alert(data));
      }
      function updateFormat() {
        const format = document.getElementById('timeFormat').value;
        fetch('/updateFormat', { 
          method: 'POST', 
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
          body: 'format=' + format 
        })
        .then(response => response.text())
        .then(data => alert(data));
      }
      function setMode(mode) {
        const time = document.getElementById('customTime').value;
        fetch('/setMode', { 
          method: 'POST', 
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
          body: 'mode=' + mode + '&customTime=' + time 
        })
        .then(response => response.text())
        .then(data => alert(data));
      }
      function updateToCurrentTime() {
        fetch('/updateToCurrentTime', { method: 'POST' })
          .then(response => response.text())
          .then(data => alert(data));
      }
    </script>
  </body>
  </html>
  )rawliteral";
}
