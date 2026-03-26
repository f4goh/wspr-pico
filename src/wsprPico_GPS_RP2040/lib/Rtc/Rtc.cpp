/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Rtc.cpp
 * Author: ale
 * 
 * Created on 3 septembre 2025, 20:18
 */

#include "Rtc.h"

Rtc::Rtc() {
}

Rtc::Rtc(const Rtc& orig) {
}

Rtc::~Rtc() {
}

void Rtc::begin() {
    Wire.begin();
}

uint8_t Rtc::bcdToDec(uint8_t val) {
    return (val >> 4) * 10 + (val & 0x0F);
}

uint8_t Rtc::decToBcd(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

void Rtc::read(RTC_DateTime* dt) {
    Wire.beginTransmission(DS3231_ADDRESS);
    Wire.write(0x00);
    Wire.endTransmission();

    Wire.requestFrom(DS3231_ADDRESS, 7);
    if (Wire.available() < 7) return;

    dt->seconds    = bcdToDec(Wire.read());
    dt->minutes    = bcdToDec(Wire.read());
    dt->hours      = bcdToDec(Wire.read() & 0x3F);
    dt->dayOfWeek  = bcdToDec(Wire.read());
    dt->day        = bcdToDec(Wire.read());
    dt->month      = bcdToDec(Wire.read() & 0x1F);
    dt->year       = 2000 + bcdToDec(Wire.read());
}

void Rtc::write(const RTC_DateTime* dt) {
    Wire.beginTransmission(DS3231_ADDRESS);
    Wire.write(0x00);

    Wire.write(decToBcd(dt->seconds));
    Wire.write(decToBcd(dt->minutes));
    Wire.write(decToBcd(dt->hours));
    Wire.write(decToBcd(dt->dayOfWeek));
    Wire.write(decToBcd(dt->day));
    Wire.write(decToBcd(dt->month));
    Wire.write(decToBcd(dt->year % 100));

    Wire.endTransmission();
}

void Rtc::format(char* buffer, const RTC_DateTime* dt) {
    sprintf(buffer, "%02d:%02d:%02d", dt->hours, dt->minutes, dt->seconds);
}

void Rtc::configSQW_1Hz() {
    Wire.beginTransmission(DS3231_ADDRESS);
    Wire.write(0x0E);
    Wire.write(0b00000000); // INTCN=0, RS1/RS2=00 → 1Hz
    Wire.endTransmission();
}

uint8_t Rtc::calculateDayOfWeek(uint16_t y, uint8_t m, uint8_t d) {
    if (m < 3) {
        m += 12;
        y -= 1;
    }
    return (d + 2*m + 3*(m+1)/5 + y + y/4 - y/100 + y/400) % 7 + 1;
}

