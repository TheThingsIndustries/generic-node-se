/*
 * Copyright (c) 2018, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 *
 * \brief Sensirion SHTC3 (and compatible) driver implementation
 *
 * This module provides access to the SHTC3 functionality over a generic I2C
 * interface. It supports measurements without clock stretching only.
 *
 * SHTC3 compatible sensors: SHTW1, SHTW2, SHTC1
 */

#include "SHTC3.h"
#include "sensirion_i2c.h"

/* all measurement commands return T (CRC) RH (CRC) */
#if USE_SENSIRION_CLOCK_STRETCHING
#define SHTC3_CMD_MEASURE_HPM 0x7CA2
#define SHTC3_CMD_MEASURE_LPM 0x6458
#else /* USE_SENSIRION_CLOCK_STRETCHING */
#define SHTC3_CMD_MEASURE_HPM 0x7866
#define SHTC3_CMD_MEASURE_LPM 0x609C
#endif /* USE_SENSIRION_CLOCK_STRETCHING */
static const uint16_t SHTC3_CMD_DURATION_USEC = 1000;
static const unsigned SHTC3_WAKEUP_TIME_USEC = 120;

static const uint16_t SHTC3_CMD_SLEEP = 0xB098;
static const uint16_t SHTC3_CMD_WAKEUP = 0x3517;
#ifdef SHT_ADDRESS
static const uint8_t SHTC3_ADDRESS = SHT_ADDRESS;
#else
static const uint8_t SHTC3_ADDRESS = 0x70;
#endif

//static uint16_t SHTC3_cmd_measure = SHTC3_CMD_MEASURE_HPM;

int SHTC3_init(SHTC3_t *shtc, HAL_I2C_bus_t bus, unsigned i2c_timeout)
{
    sensirion_i2c_init(&shtc->s_i2c);
    shtc->s_i2c.bus = bus;
    shtc->s_i2c.timeout = i2c_timeout;
    shtc->s_i2c.address = SHTC3_ADDRESS;

    shtc->SHTC3_cmd_measure = SHTC3_CMD_MEASURE_HPM;
    return 0;
}

int16_t SHTC3_sleep(SHTC3_t *shtc) {
    return sensirion_i2c_write_cmd(&shtc->s_i2c, SHTC3_CMD_SLEEP);
}

int16_t SHTC3_wake_up(SHTC3_t *shtc) {
    int16_t r = sensirion_i2c_write_cmd(&shtc->s_i2c, SHTC3_CMD_WAKEUP);
    sensirion_sleep_usec(SHTC3_WAKEUP_TIME_USEC);
    return r;
}

int16_t SHTC3_measure_blocking_read(SHTC3_t *shtc, int32_t *temperature, int32_t *humidity) {
    int16_t ret;

    ret = SHTC3_measure(shtc);
    if (ret)
        return ret;
#if !defined(USE_SENSIRION_CLOCK_STRETCHING) || !USE_SENSIRION_CLOCK_STRETCHING
    sensirion_sleep_usec(SHTC3_MEASUREMENT_DURATION_USEC);
#endif /* USE_SENSIRION_CLOCK_STRETCHING */
    return SHTC3_read(shtc, temperature, humidity);
}

int16_t SHTC3_measure(SHTC3_t *shtc) {
    return sensirion_i2c_write_cmd(&shtc->s_i2c, shtc->SHTC3_cmd_measure);
}

int16_t SHTC3_read(SHTC3_t *shtc, int32_t *temperature, int32_t *humidity) {
    uint16_t words[2];
    int16_t ret = sensirion_i2c_read_words(&shtc->s_i2c, words,
                                           SENSIRION_NUM_WORDS(words));
    /**
     * formulas for conversion of the sensor signals, optimized for fixed point
     * algebra:
     * Temperature = 175 * S_T / 2^16 - 45
     * Relative Humidity = 100 * S_RH / 2^16
     */
    *temperature = ((21875 * (int32_t)words[0]) >> 13) - 45000;
    *humidity = ((12500 * (int32_t)words[1]) >> 13);

    return ret;
}

int16_t SHTC3_probe(SHTC3_t *shtc) {

    (void)SHTC3_wake_up(shtc); /* Try to wake up the sensor, ignore return value */
    return SHTC3_read_serial(shtc, &shtc->serial);
}

void SHTC3_enable_low_power_mode(SHTC3_t *shtc, uint8_t enable_low_power_mode) {
    shtc->SHTC3_cmd_measure =
        enable_low_power_mode ? SHTC3_CMD_MEASURE_LPM : SHTC3_CMD_MEASURE_HPM;
}

int16_t SHTC3_read_serial(SHTC3_t *shtc, uint32_t *serial) {
    int16_t ret;
    const uint16_t tx_words[] = {0x007B};
    uint16_t serial_words[SENSIRION_NUM_WORDS(*serial)];

    ret = sensirion_i2c_write_cmd_with_args(&shtc->s_i2c, 0xC595, tx_words,
                                            SENSIRION_NUM_WORDS(tx_words));
    if (ret)
        return ret;

    sensirion_sleep_usec(SHTC3_CMD_DURATION_USEC);

    ret = sensirion_i2c_delayed_read_cmd(&shtc->s_i2c,
        0xC7F7, SHTC3_CMD_DURATION_USEC, &serial_words[0], 1);
    if (ret)
        return ret;

    ret = sensirion_i2c_delayed_read_cmd(&shtc->s_i2c,
        0xC7F7, SHTC3_CMD_DURATION_USEC, &serial_words[1], 1);
    if (ret)
        return ret;

    *serial = ((uint32_t)serial_words[0] << 16) | serial_words[1];
    return ret;
}

const char *SHTC3_get_driver_version(SHTC3_t *shtc) {
    return 0;
}

uint8_t SHTC3_get_configured_address(SHTC3_t *shtc) {
    return shtc->s_i2c.address;
}

uint32_t SHTC3_get_probed_serial(SHTC3_t *shtc)
{
    return shtc->serial;
}
