/* 
 * File:   Ledcouleur.h
 * Author: ale
 *
 * Created on 3 septembre 2025, 20:18
 */

#ifndef LEDCOULEUR_H
#define LEDCOULEUR_H

#include <Adafruit_NeoPixel.h>

#define NUM_LEDS 1
#define LED_PIN 16
#define LUMINOSITE 16

class LedCouleur {
public:
    LedCouleur(uint8_t pin = LED_PIN, uint8_t numLeds = NUM_LEDS);
    LedCouleur(const LedCouleur& orig);
    virtual ~LedCouleur();
    void begin();
    void jaune();
    void vert();
    void rouge();
    void bleu();
    void setLuminosite(uint8_t brightness);
private:
    Adafruit_NeoPixel strip;
};

#endif /* LEDCOULEUR_H */

