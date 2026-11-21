#include <FastLED.h>

#define NUM_LEDS 150
#define DATA_PIN D4

CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
}

void loop() {
  // Light up the LEDs in a snake-like pattern with different colors
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(random(0, 255), 255, 255); // Set to a random color
    FastLED.show(); // Display the updated colors
    delay(50); // Wait for 100 milliseconds
    leds[i] = CRGB::Black; // Turn off the LED
  }
  
  // After the loop reaches the last LED, turn on all LEDs with different colors
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(random(0, 255), 255, 255); // Set to a random color
  }
  FastLED.show(); // Display the updated colors

  delay(10000); // Keep all LEDs on for 2 seconds

  // Turn off all LEDs
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black; // Turn off all LEDs
  }
  FastLED.show(); // Display the updated colors

  delay(1000); // Wait for 1 second before repeating the loop
}
