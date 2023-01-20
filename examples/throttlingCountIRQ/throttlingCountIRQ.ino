#include <Arduino.h>

#include "freqcount.h"
#define PIN_INPUT 0

FreqCountIRQ freq_count;  // using IRQ

void setup() {
  Serial.begin(9600);

  // Count each time level of the input pin changes
  freq_count.begin(PIN_INPUT, CHANGE);
}

void loop() {
  delay(1000);

  if (freq_count.update()) {
    // The result is doubled and it should be divided by 2
    Serial.println(freq_count.get_observated_frequency() / 2.0, 3);
  }
}
