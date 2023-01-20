#include "freqcount.h"

#include "freqcount_pio.h"

void FreqCountIRQ::__freq_count_isr(FreqCountIRQ *instance) {
  uint64_t time = micros();

  if (instance->observated_count < instance->max_observation_count) {
    instance->triggerred_spans += time - instance->old_time;
    instance->observated_count++;
  }

  instance->old_time = time;
}

bool FreqCountIRQ::begin(pin_size_t pin, PinStatus mode) {
  this->attached_pin     = pin;
  this->observated_count = 0;
  this->triggerred_spans = 0;
  this->old_time         = micros();
  attachInterrupt(digitalPinToInterrupt(pin), FreqCountIRQ::__freq_count_isr, mode, this);

  return true;
}

void FreqCountIRQ::end() {
  detachInterrupt(digitalPinToInterrupt(this->attached_pin));
}

void FreqCountIRQ::set_max_observation_count(uint32_t max_observation_count) {
  this->max_observation_count = max_observation_count;
}

bool FreqCountIRQ::update() {
  if (this->observated_count == 0) {
    this->observated_frequency = NAN;
    return false;
  }

  noInterrupts();
  uint64_t spans_total   = triggerred_spans;
  uint32_t count         = observated_count;
  this->triggerred_spans = 0;
  this->observated_count = 0;
  interrupts();

  this->observated_frequency = 1.0 / ((spans_total / (double)count) * 1e-6);

  return true;
}

double FreqCountIRQ::get_observated_frequency() {
  return this->observated_frequency;
}

//

uint8_t FreqCountPIO::claimed_sm[2]     = { 0, 0 };
int32_t FreqCountPIO::program_offset[2] = { 0, 0 };

void FreqCountPIO::restart_pio() {
  pio_sm_clear_fifos(this->pio, this->sm);
  pio_sm_set_enabled(this->pio, this->sm, true);
  pio_sm_put(this->pio, this->sm, UINT32_MAX);
}

bool FreqCountPIO::select_pio_and_sm(PIO pio, uint32_t *offset) {
  uint8_t pio_index = pio_get_index(pio);

  if (FreqCountPIO::claimed_sm[pio_index] == 0) {
    if (!pio_can_add_program(pio, &freq_count_pio_program)) {
      return false;
    }

    FreqCountPIO::program_offset[pio_index] = pio_add_program(pio, &freq_count_pio_program);
  }

  if ((this->sm = pio_claim_unused_sm(pio, false)) >= 0) {
    *offset = FreqCountPIO::program_offset[pio_index];
    FreqCountPIO::claimed_sm[pio_index]++;
    this->pio        = pio;
    this->sm_claimed = true;

    return true;
  } else {
    if (FreqCountPIO::claimed_sm[pio_index] == 0) {
      pio_remove_program(pio, &freq_count_pio_program, FreqCountPIO::program_offset[pio_index]);
      FreqCountPIO::program_offset[pio_index] = 0;
    }

    return false;
  }
}

bool FreqCountPIO::begin(pin_size_t pin, double clock_div) {
  if (this->sm_claimed) {
    return false;
  }

  this->clock_pulse_duration = (1.0 / ((double)clock_get_hz(clk_sys) / clock_div));
  uint32_t offset;

  if (this->select_pio_and_sm(pio0, &offset) || this->select_pio_and_sm(pio1, &offset)) {
    freq_count_pio_program_init(this->pio, this->sm, offset, pin, clock_div);
    this->restart_pio();
    return true;
  } else {
    return false;
  }
}

void FreqCountPIO::end() {
  if (!this->sm_claimed)
    return;

  pio_sm_set_enabled(this->pio, this->sm, false);
  pio_sm_unclaim(this->pio, this->sm);
  this->sm_claimed  = false;
  uint8_t pio_index = pio_get_index(this->pio);
  FreqCountPIO::claimed_sm[pio_index]--;

  if (FreqCountPIO::claimed_sm[pio_index] == 0) {
    pio_remove_program(this->pio, &freq_count_pio_program, FreqCountPIO::program_offset[pio_index]);
    FreqCountPIO::program_offset[pio_index] = 0;
  }
}

bool FreqCountPIO::update() {
  if (pio_sm_get_rx_fifo_level(this->pio, this->sm) == 0) {
    this->observated_frequency = NAN;
    return false;
  }

  uint32_t count = pio_sm_get(this->pio, this->sm);  // non-blocking
  pio_sm_set_enabled(this->pio, this->sm, false);

  this->observated_frequency = 1.0 / ((UINT32_MAX - count) * this->clock_pulse_duration * /* 2 clocks */ 2.0 * 2.0);

  this->restart_pio();
  return true;
}

double FreqCountPIO::get_observated_frequency() {
  return this->observated_frequency;
}
