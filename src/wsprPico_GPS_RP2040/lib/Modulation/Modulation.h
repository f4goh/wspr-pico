/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Modulation.h
 * Author: ale
 *
 * Created on 28 août 2025, 07:55
 */

#ifndef MODULATION_H
#define MODULATION_H

#include "Dds.h"
#include "Menu.h"
#include <JTEncode.h>
#include <Arduino.h>
#include "hardware/timer.h"
#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "hardware/structs/timer.h"


// exemples pas mal https://github.com/vha3/Hunter-Adams-RP2040-Demos/


#define TEST_PIN 14
#define ALARM_NUM 0
#define ALARM_IRQ TIMER_IRQ_0


#define PREAMBULE 0x55

typedef enum{
  SLOW =100,
  FAST =50
}duration;

typedef enum{
  BYTES_NO_CRC = 0x70,  //plutot texte a revoir
  BYTES_WITH_CRC = 0x71,
  VARICODE = 0x72,
  GEOLOC = 0x73,
  CONVOLUTION = 0x74,
  ACK=0x7F
}mode;




class Modulation : public Dds {
public:
    Modulation(config* _cfg);
    Modulation(const Modulation& orig);
    virtual ~Modulation();

    void sendWspr(char *locator);
    void sendRtty(char* stringRtty);
    void sendFt8(char *call, char *locator);
    void sendHell(char * stringHell);
    void sendCw(char * stringCw, int cwWpm);
    void setDbm(uint8_t _dbm);
    
    
private:
    
    static void marshall();    //Merci F4JRE !!!
    void timerCallback();
    static Modulation* anchor;
    
    
    void startTimerIRQ(uint32_t us);
    void stopTimerIRQ();
    
    
    void rttyTxByte(char c);
    void waitIrq();
    
    config* cfg;
    JTEncode jtencode;
    uint8_t txBuffer[WSPR_SYMBOL_COUNT]; //constante dans lib JTEncode
    volatile bool flagIrq;    
        
    static uint32_t period_us;
       
    byte tempo;
    byte txMode;  
    uint8_t dbm;
};



#endif /* MODULATION_H */
