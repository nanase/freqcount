#pragma once
#include <Arduino.h>

#ifndef pin_size_t
typedef uint8_t pin_size_t;
#endif

template <pin_size_t pin>
class FreqCountIRQ {
 private:
  static volatile uint64_t old_time;
  static volatile uint32_t observed_count;
  static volatile uint64_t triggered_spans;
  static uint32_t max_observation_count;
  static double observed_frequency;

  static void __freq_count_isr();

 public:
  bool begin(uint8_t mode = RISING) {
    observed_count  = 0;
    triggered_spans = 0;
    old_time        = micros();
    attachInterrupt(digitalPinToInterrupt(pin), __freq_count_isr, mode);

    return true;
  }

  void end() {
    detachInterrupt(digitalPinToInterrupt(pin));
  }

  void set_max_observation_count(uint32_t max_observation_count) {
    FreqCountIRQ::max_observation_count = max_observation_count;
  }

  bool update() {
    if (observed_count == 0) {
      observed_frequency = NAN;
      return false;
    }

    noInterrupts();
    uint64_t spans_total = triggered_spans;
    uint32_t count       = observed_count;
    triggered_spans      = 0;
    observed_count       = 0;
    interrupts();

    observed_frequency = 1.0 / ((spans_total / (double)count) * 1e-6);

    return true;
  }

  double get_observed_frequency() {
    return observed_frequency;
  }
};

template <pin_size_t pin>
volatile uint64_t FreqCountIRQ<pin>::old_time = 0;
template <pin_size_t pin>
volatile uint32_t FreqCountIRQ<pin>::observed_count = 0;
template <pin_size_t pin>
volatile uint64_t FreqCountIRQ<pin>::triggered_spans = 0;
template <pin_size_t pin>
uint32_t FreqCountIRQ<pin>::max_observation_count = UINT32_MAX;
template <pin_size_t pin>
double FreqCountIRQ<pin>::observed_frequency = NAN;

template <pin_size_t pin>
void FreqCountIRQ<pin>::__freq_count_isr() {
  uint64_t time = micros();

  if (observed_count < max_observation_count) {
    triggered_spans += time - old_time;
    observed_count++;
  }

  old_time = time;
}

#if PICO_PIO_VERSION

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
