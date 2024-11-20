#pragma once
#include <Arduino.h>

class FreqCountIRQ {
 private:
  volatile uint64_t old_time;
  volatile uint32_t observed_count;
  volatile uint64_t triggered_spans;
  uint32_t max_observation_count = UINT32_MAX;
  double observed_frequency      = NAN;
  pin_size_t attached_pin;

  static void __freq_count_isr(FreqCountIRQ *instance);

 public:
  bool begin(pin_size_t pin, PinStatus mode = RISING);
  void end();
  void set_max_observation_count(uint32_t max_observation_count);
  bool update();
  double get_observed_frequency();
};

#if !PICO_NO_HARDWARE

class FreqCountPIO {
 private:
  PIO pio;
  int32_t sm;
  bool sm_claimed = false;
  double clock_pulse_duration;
  double observed_frequency = NAN;
  static uint8_t claimed_sm[2];
  static int32_t program_offset[2];

  void restart_pio();
  bool select_pio_and_sm(PIO pio, uint32_t *offset);

 public:
  bool begin(pin_size_t pin, double clock_div = 1.0);
  void end();
  bool update();
  double get_observed_frequency();
};

#endif
