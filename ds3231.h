/*
 * COPYRIGHT (C) 2026 Aarnav Patel
 *
 * @file ds3231.h
 * @brief DS3231 RTC driver for the ATmega16 multiprotocol embedded data acquisition system
 *
 * provides functions for initializing and using the DS3231 RTC
 *
 * @author Aarnav Patel
 * @date September 2026
 *
 */

#ifndef DS3231_H
#define DS3231_H

#include <stdint.h>

/*
 * DS3231 7-bit I2C address
 */
#define DS3231_ADDRESS 0x68


/*
 * DS3231 time/date structure
 */
typedef struct
{
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;

    uint8_t day;
    uint8_t date;
    uint8_t month;
    uint8_t year;

} DS3231_Time;


/*
 * Set RTC time and date
 */
uint8_t DS3231_SetTime(DS3231_Time *time);


/*
 * Read RTC time and date
 */
uint8_t DS3231_GetTime(DS3231_Time *time);

#endif