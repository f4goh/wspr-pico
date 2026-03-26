/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Rtc.h
 * Author: ale
 *
 * Created on 3 septembre 2025, 20:18
 */

#ifndef RTC_H
#define RTC_H

#include <Wire.h>

#define DS3231_ADDRESS 0x68

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t dayOfWeek;   // 1 = dimanche
    uint8_t day;
    uint8_t month;
    uint16_t year;
} RTC_DateTime;


class Rtc {
public:
    Rtc();
    Rtc(const Rtc& orig);
    virtual ~Rtc();
    void begin();
    void read(RTC_DateTime* dt);
    void write(const RTC_DateTime* dt);
    void format(char* buffer, const RTC_DateTime* dt);
    void configSQW_1Hz();
    uint8_t calculateDayOfWeek(uint16_t y, uint8_t m, uint8_t d);
    
private:
    uint8_t bcdToDec(uint8_t val);
    uint8_t decToBcd(uint8_t val);

};

#endif /* RTC_H */
