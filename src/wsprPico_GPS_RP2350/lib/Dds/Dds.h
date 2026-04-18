/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Dds.h
 * Author: ale
 *
 * Created on 28 août 2025, 07:55
 */

#ifndef DDS_H
#define DDS_H

#include <Arduino.h>
#include "vfo2.pio.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"


//#define CLK_KHZ  125000L
#define CLK_KHZ  180000L


#define RTTY_TONE_SPACING       17000L  //170Hz
#define RTTY_DELAY              22

#define WSPR_TONE_SPACING       146L //1.45 Hz
#define WSPR_DELAY              682

#define FT8_TONE_SPACING        625L  //6.25 Hz
#define FT8_DELAY               160

//https://ufrc.org/info/informations-nouveau-mode-ft2-open-source/
#define FT2_TONE_SPACING        2100L  //21 Hz
#define FT2_DELAY               13

#define HELL_DELAY              4081

class Dds {
public:
    Dds();
    Dds(const Dds& orig);
    virtual ~Dds();
    
   
    void coreUnSetup();
    void setFreqWspr(uint32_t freq);
    void setFreqRtty(uint32_t freq);
    void setFreqFt8(uint32_t freq);
    void setFreqCW(uint32_t freq);
    void changeFreqCW(int8_t sens); 
    uint32_t getFreqCW();
   
    uint32_t wsfr[4];   //4-FSK  les tables sont elles au bon endroit ?
    uint32_t rttyfr[2];  //2-FSK
    uint32_t ft8fr[8];  //8-FSK
    uint32_t cwfr;
    uint32_t freqCW;
    
     
    volatile uint32_t periods;  //pas de setter en lien avec les tables pour le moment mais il faudrait le mettre en private
    
private:
};

#endif // DDS_H