#include <Arduino.h>

#include "freqcount.h"

FreqCountPIO freq_count[8];  // using PIOs

void setup() {
  Serial.begin(9600);

  for (size_t i = 0; i < 8; i++) { freq_count[i].begin(i); }
}

void print_value(double value) {
  static char str[16];
  int32_t length = sprintf(str, "%.3f", value);

  for (size_t i = 0; i < 7 - length; i++) { Serial.print(" "); }

  Serial.print(str);
}

void print_freq(double freq) {
  if (!isfinite(freq)) {
    Serial.print("---.--- ");
  } else if (freq >= 1.0e6) {
    print_value(freq / 1.0e6);
    Serial.print("M");
  } else if (freq >= 1.0e3) {
    print_value(freq / 1.0e3);
    Serial.print("k");
  } else {
    print_value(freq);
    Serial.print(" ");
  }
}

void loop() {
  static uint64_t display_count = 0;

  delay(1000);

  if (++display_count % 10 == 0) {
    Serial.println("");
    Serial.println("|  pin #0  |  pin #1  |  pin #2  |  pin #3  |  pin #4  |  pin #5  |  pin #6  |  pin #7  |");
  }

  Serial.print("| ");

  for (size_t i = 0; i < 8; i++) {
    freq_count[i].update();
    print_freq(freq_count[i].get_observed_frequency());
    Serial.print(" | ");
  }

  Serial.println("");
}
