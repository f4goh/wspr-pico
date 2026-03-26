/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Dds.cpp
 * Author: ale
 * 
 * Created on 28 août 2025, 07:55
 */

#include "Dds.h"

Dds::Dds() {
}

Dds::Dds(const Dds& orig) {
}

Dds::~Dds() {
}

void Dds::coreUnSetup() {
  uint16_t t;
  uint32_t delta;
  PIO pio = pio0;
  gpio_init(0);
  gpio_set_dir(0, GPIO_OUT);
  pio_gpio_init(pio0, 15);         
  uint offset = pio_add_program(pio0, &vfo_program);         
  pio_sm_set_consecutive_pindirs(pio0, 0, 15, 1, true);          
  pio_sm_config c = vfo_program_get_default_config(offset);
  sm_config_set_set_pins(&c, 15, 1);                  
  pio_sm_init(pio0, 0, offset, &c);       
  pio_sm_set_enabled(pio0, 0, true);
  periods = 200 << 24;  //dds off
  for(;;){
    t = (periods+delta) >> 24;  
    pio_sm_put_blocking(pio0, 0, t);
    delta += periods-(t << 24);      
  }

}

//A simplifier pour le calcul des fréquences beaucoup de répétitions

void Dds::setFreqWspr(uint32_t freq) {
    uint64_t ratio = (uint64_t) CLK_KHZ * 1000LL * (1 << 24) / (uint64_t) freq;
    periods = (uint32_t) ratio;
    uint32_t dwsfr = WSPR_TONE_SPACING * (periods / freq) / 100L;  //146 c'est 1.46 Hz
    wsfr[0] = periods - (10 << 24); // + 3 x Abstand 1,46 Hz
    wsfr[1] = wsfr[0] - dwsfr;
    wsfr[2] = wsfr[1] - dwsfr;
    wsfr[3] = wsfr[2] - dwsfr;
}

void Dds::setFreqRtty(uint32_t freq) {
    uint64_t ratio = (uint64_t) CLK_KHZ * 1000LL * (1 << 24) / (uint64_t) freq;
    periods = (uint32_t) ratio;
    uint32_t dwsfr = RTTY_TONE_SPACING * (periods / freq) / 100L; //170 Hz de shift
    rttyfr[0] = periods - (10 << 24);
    rttyfr[1] = rttyfr[0] - dwsfr;
}

void Dds::setFreqFt8(uint32_t freq) {
    uint64_t ratio = (uint64_t) CLK_KHZ * 1000LL * (1 << 24) / (uint64_t) freq;
    periods = (uint32_t) ratio;
    uint32_t dwsfr = FT8_TONE_SPACING * (periods / freq) / 100L; //170 Hz de shift
    ft8fr[0] = periods - (10 << 24);
    ft8fr[1] = ft8fr[0] - dwsfr;
    ft8fr[2] = ft8fr[1] - dwsfr;
    ft8fr[3] = ft8fr[2] - dwsfr;
    ft8fr[4] = ft8fr[3] - dwsfr;
    ft8fr[5] = ft8fr[4] - dwsfr;
    ft8fr[6] = ft8fr[5] - dwsfr;
    ft8fr[7] = ft8fr[6] - dwsfr;

}