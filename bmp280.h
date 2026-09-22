#ifndef BMP280_H
#define BMP280_H

#include <stdint.h>

/*
 * BMP280 chip ID
 */
#define BMP280_CHIP_ID 0x58


/*
 * Initialize BMP280.
 *
 * Returns:
 * 0 -> BMP280 detected and initialized
 * 1 -> BMP280 not detected
 */
uint8_t BMP280_Init(void);


/*
 * Read compensated temperature.
 *
 * Return value is in hundredths
 * of a degree Celsius.
 *
 * Example:
 *
 * 2534 = 25.34 °C
 */
int32_t BMP280_ReadTemperature(void);


/*
 * Read compensated pressure.
 *
 * Return value is in Pa.
 *
 * Example:
 *
 * 101325 = 101325 Pa
 */
uint32_t BMP280_ReadPressure(void);


/*
 * Read both temperature and pressure.
 *
 * temperature:
 *     hundredths of a degree Celsius
 *
 * pressure:
 *     Pa
 */
uint8_t BMP280_ReadData(int32_t *temperature,
                        uint32_t *pressure);

#endif