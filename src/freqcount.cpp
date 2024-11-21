#include "freqcount.h"

#if PICO_PIO_VERSION
#include "freqcount_pio.h"

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
    this->observed_frequency = NAN;
    return false;
  }

  uint32_t count = pio_sm_get(this->pio, this->sm);  // non-blocking
  pio_sm_set_enabled(this->pio, this->sm, false);

  this->observed_frequency = 1.0 / ((UINT32_MAX - count) * this->clock_pulse_duration * /* 2 clocks */ 2.0 * 2.0);

  this->restart_pio();
  return true;
}

double FreqCountPIO::get_observed_frequency() {
  return this->observed_frequency;
}

#endif
