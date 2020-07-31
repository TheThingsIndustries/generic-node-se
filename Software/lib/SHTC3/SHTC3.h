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
 * \brief Sensirion SHT driver interface
 *
 * This module provides access to the SHT functionality over a generic I2C
 * interface. It supports measurements without clock stretching only.
 */

#ifndef SHTC3_H
#define SHTC3_H

#include "sensirion_i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SHTC3_STATUS_OK 0
#define SHTC3_STATUS_ERR_BAD_DATA (-1)
#define SHTC3_STATUS_CRC_FAIL (-2)
#define SHTC3_STATUS_UNKNOWN_DEVICE (-3)
#define SHTC3_MEASUREMENT_DURATION_USEC 14400

/**
 * Detects if a sensor is connected by reading out the ID register.
 * If the sensor does not answer or if the answer is not the expected value,
 * the test fails.
 *
 * @return 0 if a sensor was detected
 */
int16_t SHTC3_probe(void);

/**
 * Starts a measurement and then reads out the results. This function blocks
 * while the measurement is in progress. The duration of the measurement depends
 * on the sensor in use, please consult the datasheet.
 * Temperature is returned in [degree Celsius], multiplied by 1000,
 * and relative humidity in [percent relative humidity], multiplied by 1000.
 *
 * @param temperature   the address for the result of the temperature
 * measurement
 * @param humidity      the address for the result of the relative humidity
 * measurement
 * @return              0 if the command was successful, else an error code.
 */
int16_t SHTC3_measure_blocking_read(int32_t *temperature, int32_t *humidity);

/**
 * Starts a measurement in high precision mode. Use SHTC3_read() to read out the
 * values, once the measurement is done. The duration of the measurement depends
 * on the sensor in use, please consult the datasheet.
 *
 * @return     0 if the command was successful, else an error code.
 */
int16_t SHTC3_measure(void);

/**
 * Reads out the results of a measurement that was previously started by
 * SHTC3_measure(). If the measurement is still in progress, this function
 * returns an error.
 * Temperature is returned in [degree Celsius], multiplied by 1000,
 * and relative humidity in [percent relative humidity], multiplied by 1000.
 *
 * @param temperature   the address for the result of the temperature
 * measurement
 * @param humidity      the address for the result of the relative humidity
 * measurement
 * @return              0 if the command was successful, else an error code.
 */
int16_t SHTC3_read(int32_t *temperature, int32_t *humidity);

/**
 * Send the sensor to sleep, if supported.
 *
 * Note: DESPITE THE NAME, THIS COMMAND IS ONLY AVAILABLE FOR THE SHTC3
 *
 * Usage:
 * ```
 * int16_t ret;
 * int32_t temperature, humidity;
 * ret = SHTC3_wake_up();
 * if (ret) {
 *     // error waking up
 * }
 * ret = SHTC3_measure_blocking_read(&temperature, &humidity);
 * if (ret) {
 *     // error measuring
 * }
 * ret = SHTC3_sleep();
 * if (ret) {
 *     // error sending sensor to sleep
 * }
 * ```
 *
 * @return  0 if the command was successful, else an error code.
 */
int16_t SHTC3_sleep(void);

/**
 * Wake the sensor from sleep
 *
 * Note: DESPITE THE NAME, THIS COMMAND IS ONLY AVAILABLE FOR THE SHTC3
 *
 * Usage:
 * ```
 * int16_t ret;
 * int32_t temperature, humidity;
 * ret = SHTC3_wake_up();
 * if (ret) {
 *     // error waking up
 * }
 * ret = SHTC3_measure_blocking_read(&temperature, &humidity);
 * if (ret) {
 *     // error measuring
 * }
 * ret = SHTC3_sleep();
 * if (ret) {
 *     // error sending sensor to sleep
 * }
 * ```
 *
 * @return  0 if the command was successful, else an error code.
 */
int16_t SHTC3_wake_up(void);

/**
 * Enable or disable the SHT's low power mode
 *
 * @param enable_low_power_mode 1 to enable low power mode, 0 to disable
 */
void SHTC3_enable_low_power_mode(uint8_t enable_low_power_mode);

/**
 * Read out the serial number
 *
 * @param serial    the address for the result of the serial number
 * @return          0 if the command was successful, else an error code.
 */
int16_t SHTC3_read_serial(uint32_t *serial);

/**
 * Return the driver version
 *
 * @return Driver version string
 */
const char *SHTC3_get_driver_version(void);

/**
 * Returns the configured SHT address.
 *
 * @return The configured i2c address
 */
uint8_t SHTC3_get_configured_address(void);

#ifdef __cplusplus
}
#endif

#endif /* SHTC3_H */
