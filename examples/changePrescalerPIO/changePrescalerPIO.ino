#include <Arduino.h>

#include "freqcount.h"
#define PIN_INPUT 0

FreqCountPIO freq_count;  // using PIO

void setup() {
  Serial.begin(9600);

  // set PIO clock frequency to 10MHz
  freq_count.begin(PIN_INPUT, clock_get_hz(clk_sys) / 10.0);
}

void loop() {
  delay(1000);

  if (freq_count.update()) {
    Serial.println(freq_count.get_observated_frequency(), 3);
  }
}
