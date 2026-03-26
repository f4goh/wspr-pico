/* 
 * File:   Afficheur.cpp
 * Author: F4GOH
 * 
 * spécialisation de SSD1306Wire
 */

#ifndef AFFICHEUR_H
#define AFFICHEUR_H

#include <Arduino.h>
#include <U8g2lib.h>
#include "Menu.h"


#define SCREEN_ADDRESS 0x3C // I2C address for most 128x64 OLEDs

#define MSGX 2
#define MSGY 2

class Afficheur {
public:
    Afficheur();  // Tu peux ajouter modePWR si nécessaire
    virtual ~Afficheur();

    void begin(); // Initialisation de l'écran
    void timeDisplay(char * txt);
    void modeDisplay(const char* txt);
    void modeEfface();
    void configDisplay(config& cfg);
    void clearDisplay();
    
private:
    //Adafruit_SSD1306 display;
     //U8G2_SSD1306_128X64_NONAME_F_HW_I2C display; //gfx
     U8X8_SSD1306_128X64_NONAME_HW_I2C display;     //txt

};

#endif /* AFFICHEUR_H */


