#include <Arduino.h>

#include "freqcount.h"
#define PIN_INPUT 0

FreqCountIRQ freq_count;  // using IRQ

void setup() {
  Serial.begin(9600);

  // NOTE:
  // This method reduces count of observations.
  // But it does NOT reduce load of core because IRQs continue to be generated.
  // To reduce load, you should call `end` method or use FreqCountPIO class.
  freq_count.set_max_observation_count(10);

  freq_count.begin(PIN_INPUT);
}

void loop() {
  delay(1000);

  if (freq_count.update()) {
    Serial.println(freq_count.get_observed_frequency(), 3);
  }
}
