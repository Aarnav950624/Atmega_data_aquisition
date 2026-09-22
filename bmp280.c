#include <stdint.h>

#include "bmp280.h"
#include "spi.h"


/*
 * BMP280 register addresses
 */
#define BMP280_REG_CHIP_ID       0xD0
#define BMP280_REG_RESET         0xE0
#define BMP280_REG_STATUS        0xF3
#define BMP280_REG_CTRL_MEAS     0xF4
#define BMP280_REG_CONFIG        0xF5

#define BMP280_REG_CALIB_START   0x88

#define BMP280_REG_PRESS_MSB     0xF7
#define BMP280_REG_PRESS_LSB     0xF8
#define BMP280_REG_PRESS_XLSB    0xF9

#define BMP280_REG_TEMP_MSB      0xFA
#define BMP280_REG_TEMP_LSB      0xFB
#define BMP280_REG_TEMP_XLSB     0xFC


/*
 * BMP280 factory calibration coefficients.
 */
typedef struct
{
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;

    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;

} BMP280_Calibration;


/*
 * Calibration data stored internally
 * by the BMP280 driver.
 */
static BMP280_Calibration calibration;


/*
 * Fine temperature value.
 *
 * Used by pressure compensation.
 */
static int32_t t_fine;


/*
 * Read one BMP280 register.
 */
static uint8_t BMP280_ReadRegister(uint8_t reg)
{
    uint8_t data;

    /*
     * Select BMP280.
     */
    SPI_Select();

    /*
     * Set bit 7 to indicate READ.
     */
    SPI_Transfer(reg | 0x80);

    /*
     * Send dummy byte to generate
     * clock pulses and receive data.
     */
    data = SPI_Transfer(0x00);

    /*
     * End transaction.
     */
    SPI_Deselect();

    return data;
}


/*
 * Write one BMP280 register.
 */
static void BMP280_WriteRegister(uint8_t reg, uint8_t data)
{
    /*
     * Select BMP280.
     */
    SPI_Select();

    /*
     * Bit 7 = 0 indicates WRITE.
     */
    SPI_Transfer(reg & 0x7F);

    /*
     * Send register data.
     */
    SPI_Transfer(data);

    /*
     * End transaction.
     */
    SPI_Deselect();
}


/*
 * Read multiple consecutive registers.
 */
static void BMP280_ReadRegisters(uint8_t reg,
                                 uint8_t *data,
                                 uint8_t length)
{
    uint8_t i;

    /*
     * Select BMP280.
     */
    SPI_Select();

    /*
     * Send starting register address
     * with READ bit set.
     */
    SPI_Transfer(reg | 0x80);

    /*
     * Read consecutive bytes.
     */
    for (i = 0; i < length; i++)
    {
        data[i] = SPI_Transfer(0x00);
    }

    /*
     * End transaction.
     */
    SPI_Deselect();
}


/*
 * Read BMP280 factory calibration data.
 *
 * Calibration registers:
 *
 * 0x88 - 0x9F
 *
 * Total = 24 bytes.
 */
static void BMP280_ReadCalibration(void)
{
    uint8_t data[24];
    uint8_t i;

    /*
     * Select BMP280.
     */
    SPI_Select();

    /*
     * Start reading at 0x88.
     */
    SPI_Transfer(BMP280_REG_CALIB_START | 0x80);

    /*
     * Read all 24 calibration bytes.
     */
    for (i = 0; i < 24; i++)
    {
        data[i] = SPI_Transfer(0x00);
    }

    /*
     * End transaction.
     */
    SPI_Deselect();


    /*
     * Temperature calibration.
     *
     * BMP280 stores values in
     * little-endian format.
     */

    calibration.dig_T1 =
        ((uint16_t)data[1] << 8) |
        data[0];

    calibration.dig_T2 =
        (int16_t)(
            ((uint16_t)data[3] << 8) |
            data[2]
        );

    calibration.dig_T3 =
        (int16_t)(
            ((uint16_t)data[5] << 8) |
            data[4]
        );


    /*
     * Pressure calibration.
     */

    calibration.dig_P1 =
        ((uint16_t)data[7] << 8) |
        data[6];

    calibration.dig_P2 =
        (int16_t)(
            ((uint16_t)data[9] << 8) |
            data[8]
        );

    calibration.dig_P3 =
        (int16_t)(
            ((uint16_t)data[11] << 8) |
            data[10]
        );

    calibration.dig_P4 =
        (int16_t)(
            ((uint16_t)data[13] << 8) |
            data[12]
        );

    calibration.dig_P5 =
        (int16_t)(
            ((uint16_t)data[15] << 8) |
            data[14]
        );

    calibration.dig_P6 =
        (int16_t)(
            ((uint16_t)data[17] << 8) |
            data[16]
        );

    calibration.dig_P7 =
        (int16_t)(
            ((uint16_t)data[19] << 8) |
            data[18]
        );

    calibration.dig_P8 =
        (int16_t)(
            ((uint16_t)data[21] << 8) |
            data[20]
        );

    calibration.dig_P9 =
        (int16_t)(
            ((uint16_t)data[23] << 8) |
            data[22]
        );
}


/*
 * Configure BMP280 measurement settings.
 */
static void BMP280_Configure(void)
{
    /*
     * CTRL_MEAS = 0x27
     *
     * Temperature oversampling = x1
     * Pressure oversampling    = x1
     */
    BMP280_WriteRegister(
        BMP280_REG_CTRL_MEAS,
        0x27
    );


    /*
     * CONFIG = 0x00
     *
     * Standby = 0.5 ms
     * Filter  = OFF
     * 4-wire SPI
     */
    BMP280_WriteRegister(
        BMP280_REG_CONFIG,
        0x00
    );
}


/*
 * Read raw temperature and pressure.
 *
 * Both values are 20-bit ADC values.
 */
static void BMP280_ReadRawData(uint32_t *raw_temperature,
                               uint32_t *raw_pressure)
{
    uint8_t data[6];


    /*
     * Read:
     *
     * 0xF7 = Pressure MSB
     * 0xF8 = Pressure LSB
     * 0xF9 = Pressure XLSB
     * 0xFA = Temperature MSB
     * 0xFB = Temperature LSB
     * 0xFC = Temperature XLSB
     */
    BMP280_ReadRegisters(
        BMP280_REG_PRESS_MSB,
        data,
        6
    );


    /*
     * Build 20-bit raw pressure.
     */
    *raw_pressure =
        ((uint32_t)data[0] << 12) |
        ((uint32_t)data[1] << 4)  |
        ((uint32_t)data[2] >> 4);


    /*
     * Build 20-bit raw temperature.
     */
    *raw_temperature =
        ((uint32_t)data[3] << 12) |
        ((uint32_t)data[4] << 4)  |
        ((uint32_t)data[5] >> 4);
}


/*
 * Calculate compensated temperature.
 *
 * Result:
 *
 * 0.01 degree Celsius
 */
static int32_t BMP280_CompensateTemperature(
    uint32_t raw_temperature)
{
    int32_t var1;
    int32_t var2;
    int32_t temperature;


    /*
     * First temperature compensation term.
     */
    var1 =
        ((((int32_t)(raw_temperature >> 3)) -
          ((int32_t)calibration.dig_T1 << 1)) *
          (int32_t)calibration.dig_T2) >> 11;


    /*
     * Second temperature compensation term.
     */
    var2 =
        (((((int32_t)(raw_temperature >> 4)) -
           (int32_t)calibration.dig_T1) *
          (((int32_t)(raw_temperature >> 4)) -
           (int32_t)calibration.dig_T1)) >> 12) *
          (int32_t)calibration.dig_T3) >> 14;


    /*
     * Calculate t_fine.
     *
     * This value is also required for
     * pressure compensation.
     */
    t_fine = var1 + var2;


    /*
     * Calculate compensated temperature.
     *
     * Result is in 0.01 degree Celsius.
     */
    temperature =
        (t_fine * 5 + 128) >> 8;


    return temperature;
}


/*
 * Calculate compensated pressure.
 *
 * Result:
 *
 * Pa
 */
static uint32_t BMP280_CompensatePressure(
    uint32_t raw_pressure)
{
    int64_t var1;
    int64_t var2;
    int64_t pressure;


    /*
     * First pressure compensation term.
     */
    var1 =
        ((int64_t)t_fine) - 128000;


    var2 =
        var1 * var1 *
        (int64_t)calibration.dig_P6;


    var2 =
        var2 +
        ((var1 *
          (int64_t)calibration.dig_P5) << 17);


    var2 =
        var2 +
        (((int64_t)calibration.dig_P4) << 35);


    /*
     * Second pressure compensation term.
     */
    var1 =
        ((var1 * var1 *
          (int64_t)calibration.dig_P3) >> 8) +
        ((var1 *
          (int64_t)calibration.dig_P2) << 12);


    /*
     * Apply dig_P1.
     */
    var1 =
        (((((int64_t)1) << 47) + var1) *
         (int64_t)calibration.dig_P1) >> 33;


    /*
     * Prevent division by zero.
     */
    if (var1 == 0)
    {
        return 0;
    }


    /*
     * Calculate pressure.
     */
    pressure =
        1048576 - raw_pressure;


    pressure =
        (((pressure << 31) - var2) * 3125) /
        var1;


    /*
     * Final pressure compensation.
     */
    var1 =
        ((int64_t)calibration.dig_P9 *
         (pressure >> 13) *
         (pressure >> 13)) >> 25;


    var2 =
        ((int64_t)calibration.dig_P8 *
         pressure) >> 19;


    pressure =
        ((pressure + var1 + var2) >> 8) +
        ((int64_t)calibration.dig_P7 << 4);


    /*
     * Convert from Q24.8 format
     * to integer Pa.
     */
    return (uint32_t)(pressure >> 8);
}


/*
 * Initialize BMP280.
 */
uint8_t BMP280_Init(void)
{
    uint8_t chip_id;


    /*
     * Read chip ID.
     */
    chip_id =
        BMP280_ReadRegister(
            BMP280_REG_CHIP_ID
        );


    /*
     * Verify BMP280.
     */
    if (chip_id != BMP280_CHIP_ID)
    {
        return 1;
    }


    /*
     * Read factory calibration data.
     */
    BMP280_ReadCalibration();


    /*
     * Configure BMP280.
     */
    BMP280_Configure();


    /*
     * Initialization successful.
     */
    return 0;
}


/*
 * Read compensated temperature.
 */
int32_t BMP280_ReadTemperature(void)
{
    uint32_t raw_temperature;
    uint32_t raw_pressure;


    /*
     * Read raw sensor data.
     */
    BMP280_ReadRawData(
        &raw_temperature,
        &raw_pressure
    );


    /*
     * Compensate temperature.
     */
    return BMP280_CompensateTemperature(
        raw_temperature
    );
}


/*
 * Read compensated pressure.
 */
uint32_t BMP280_ReadPressure(void)
{
    uint32_t raw_temperature;
    uint32_t raw_pressure;


    /*
     * Read raw sensor data.
     */
    BMP280_ReadRawData(
        &raw_temperature,
        &raw_pressure
    );


    /*
     * Calculate t_fine first.
     *
     * Pressure compensation requires it.
     */
    BMP280_CompensateTemperature(
        raw_temperature
    );


    /*
     * Compensate pressure.
     */
    return BMP280_CompensatePressure(
        raw_pressure
    );
}


/*
 * Read temperature and pressure together.
 */
uint8_t BMP280_ReadData(int32_t *temperature,
                        uint32_t *pressure)
{
    uint32_t raw_temperature;
    uint32_t raw_pressure;


    /*
     * Check pointers.
     */
    if (temperature == 0 || pressure == 0)
    {
        return 1;
    }


    /*
     * Read both raw values from one
     * sensor transaction.
     */
    BMP280_ReadRawData(
        &raw_temperature,
        &raw_pressure
    );


    /*
     * Calculate compensated temperature.
     *
     * This also calculates t_fine.
     */
    *temperature =
        BMP280_CompensateTemperature(
            raw_temperature
        );


    /*
     * Calculate compensated pressure
     * using the same measurement's t_fine.
     */
    *pressure =
        BMP280_CompensatePressure(
            raw_pressure
        );


    return 0;
}