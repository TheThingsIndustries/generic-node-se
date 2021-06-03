/** Copyright © 2021 The Things Industries B.V.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file basic_sensor.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#include "app.h"
#include "vl53l1_api.h"

typedef union
{
    int16_t i16bit[3];
    uint8_t u8bit[6];
} axis3bit16_t;

typedef union
{
    int16_t i16bit;
    uint8_t u8bit[2];
} axis1bit16_t;

VL53L1_Dev_t tof_sensor = {
    .I2cDevAddr = 0x52,
    .new_data_ready_poll_duration_ms = 1000,
};

void temperature_sensor_read_data_polling(uint8_t n_reads, uint32_t read_delay)
{
    int32_t temperature = 0;
    int32_t humidity = 0;
    int16_t status = 0;
    uint8_t read_counter = 0;
    sensirion_i2c_init();
    if (SHTC3_probe() != SHTC3_STATUS_OK)
    {
        APP_PPRINTF("\r\n Failed to initialize SHTC3 Temperature Sensor\r\n");
    }
    for (read_counter = 0; read_counter < n_reads; read_counter++)
    {

        status = SHTC3_measure_blocking_read(&temperature, &humidity);
        if (status == SHTC3_STATUS_OK)
        {
            //Remove the division by 1000 to observe the higher resolution
            APP_PPRINTF("\r\n Measured Temperature: %d'C & Relative Humidity: %d \r\n", temperature / 1000, humidity / 1000);
        }
        else
        {
            APP_PPRINTF("\r\n Failed to read data from SHTC3 sensor \r\n");
        }
        HAL_Delay(read_delay);
    }
}

void accelerometer_read_data_polling(uint8_t n_reads, uint32_t read_delay)
{
    axis3bit16_t data_raw_acceleration;
    axis1bit16_t data_raw_temperature;
    float acceleration_mg[3];
    float temperature_degC;
    uint8_t whoamI;
    stmdev_ctx_t dev_ctx;
    uint8_t read_counter = 0;
    LIS2DH12_init(&dev_ctx);

    /* Check device ID */
    lis2dh12_device_id_get(&dev_ctx, &whoamI);
    if (whoamI != LIS2DH12_ID)
    {
        APP_PPRINTF("\r\n Failed to initialize LIS2DH12 Accelerometer\r\n");
        return;
    }

    /* Enable Block Data Update. */
    lis2dh12_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);

    /* Set Output Data Rate to 1Hz. */
    lis2dh12_data_rate_set(&dev_ctx, LIS2DH12_ODR_1Hz);

    /* Set full scale to 2g. */
    lis2dh12_full_scale_set(&dev_ctx, LIS2DH12_2g);

    /* Enable temperature sensor. */
    lis2dh12_temperature_meas_set(&dev_ctx, LIS2DH12_TEMP_ENABLE);

    /* Set device in continuous mode with 12 bit resol. */
    lis2dh12_operating_mode_set(&dev_ctx, LIS2DH12_HR_12bit);

    /* Read samples in polling mode (no int) */
    for (read_counter = 0; read_counter < n_reads; read_counter++)
    {
        lis2dh12_reg_t reg;

        /* Read output only if new value available */
        lis2dh12_xl_data_ready_get(&dev_ctx, &reg.byte);
        if (reg.byte)
        {
            /* Read accelerometer data */
            memset(data_raw_acceleration.u8bit, 0x00, 3 * sizeof(int16_t));
            lis2dh12_acceleration_raw_get(&dev_ctx, data_raw_acceleration.u8bit);
            acceleration_mg[0] =
                lis2dh12_from_fs2_hr_to_mg(data_raw_acceleration.i16bit[0]);
            acceleration_mg[1] =
                lis2dh12_from_fs2_hr_to_mg(data_raw_acceleration.i16bit[1]);
            acceleration_mg[2] =
                lis2dh12_from_fs2_hr_to_mg(data_raw_acceleration.i16bit[2]);

            APP_PPRINTF("\r\n Accelerometer acceleration [mg]: X: %4.2f\t Y: %4.2f\t Z: %4.2f\r\n",
                        acceleration_mg[0], acceleration_mg[1], acceleration_mg[2]);
        }

        lis2dh12_temp_data_ready_get(&dev_ctx, &reg.byte);
        if (reg.byte)
        {
            /* Read temperature data */
            memset(data_raw_temperature.u8bit, 0x00, sizeof(int16_t));
            lis2dh12_temperature_raw_get(&dev_ctx, data_raw_temperature.u8bit);
            temperature_degC =
                lis2dh12_from_lsb_hr_to_celsius(data_raw_temperature.i16bit);

            APP_PPRINTF("\r\n Accelerometer Temperature sensor [degC]: %6.2f\r\n",
                        temperature_degC);
        }
        HAL_Delay(read_delay);
    }
}

void setupSensor(VL53L1_DEV dev)
{
    uint16_t wordData;
    uint8_t byteData;
    int status = 0;

    /* Those basic I2C read functions can be used to check your own I2C functions */
    status += VL53L1_RdByte(dev, 0x010F, &byteData);
    APP_PPRINTF("\r\n VL53L1X Model_ID: %X \r\n", byteData);
    status += VL53L1_RdByte(dev, 0x0110, &byteData);
    APP_PPRINTF("\r\n VL53L1X Module_Type: %X \r\n", byteData);
    status += VL53L1_RdWord(dev, 0x010F, &wordData);
    APP_PPRINTF("\r\n VL53L1X: %X \r\n", wordData);
    status += VL53L1_WaitDeviceBooted(dev);
    APP_PPRINTF("\r\n Chip booted \r\n");

    /* This function must to be called to initialize the sensor with the default setting  */
    status += VL53L1_DataInit(dev);
    status += VL53L1_StaticInit(dev);
    /* Optional functions to be used to change the main ranging parameters according the application requirements to get the best ranging performances */
    status += VL53L1_SetPresetMode(dev, VL53L1_PRESETMODE_LITE_RANGING);
    status += VL53L1_SetDistanceMode(dev, VL53L1_DISTANCEMODE_SHORT);
    status += VL53L1_SetMeasurementTimingBudgetMicroSeconds(dev, 50000);
    status += VL53L1_SetInterMeasurementPeriodMilliSeconds(dev, 0);

    //  status = VL53L1X_SetOffset(dev,20); /* offset compensation in mm */
    //  status = VL53L1X_SetROI(dev, 16, 16); /* minimum ROI 4,4 */
    //	status = VL53L1X_CalibrateOffset(dev, 140, &offset); /* may take few second to perform the offset cal*/
    //	status = VL53L1X_CalibrateXtalk(dev, 1000, &xtalk); /* may take few second to perform the xtalk cal */
    status += VL53L1_StartMeasurement(dev); /* This function has to be called to enable the ranging */

    if (status)
    {
        APP_PPRINTF("\r\n The senor was not initalized \r\n");
    }
}

void tof_sensor_test(void)
{
    setupSensor(&tof_sensor);
    int status = 0;

    while (1)
    {
        VL53L1_RangingMeasurementData_t data;
        status += VL53L1_WaitMeasurementDataReady(&tof_sensor);
        status += VL53L1_GetRangingMeasurementData(&tof_sensor, &data);
        VL53L1_ClearInterruptAndStartMeasurement(&tof_sensor);
        if (data.RangeStatus == 0)
        {
            if (data.RangeMilliMeter < 100)
            {
                BUZZER_SetState(BUZZER_STATE_DANGER);
            }
            else if (data.RangeMilliMeter > 100 && data.RangeMilliMeter < 200)
            {
                BUZZER_SetState(BUZZER_STATE_WARNING);
            }
            else if (data.RangeMilliMeter > 200 && data.RangeMilliMeter < 300)
            {
                BUZZER_SetState(BUZZER_STATE_RING);
            }
            else
            {
                BUZZER_SetState(BUZZER_STATE_OFF);
            }
            APP_PPRINTF("\r\n TOF range in Millimeter: %d \r\n", data.RangeMilliMeter);
        }
        HAL_Delay(400);
    }
}
