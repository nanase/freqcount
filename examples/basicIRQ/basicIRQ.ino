#include <Arduino.h>

#include "freqcount.h"
#define PIN_INPUT 0

FreqCountIRQ freq_count;  // using IRQ

void setup() {
  Serial.begin(9600);
  freq_count.begin(PIN_INPUT);
}

void loop() {
  delay(1000);

  if (freq_count.update()) {
    Serial.println(freq_count.get_observed_frequency(), 3);
  }
}
