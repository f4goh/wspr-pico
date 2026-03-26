/* 
 * File:   Ledcouleur.cpp
 * Author: ale
 * 
 * Created on 3 septembre 2025, 20:18
 */

#include "LedCouleur.h"

LedCouleur::LedCouleur(uint8_t pin, uint8_t numLeds)
    : strip(numLeds, pin, NEO_GRB + NEO_KHZ800) {}

LedCouleur::LedCouleur(const LedCouleur& orig) {
}

LedCouleur::~LedCouleur() {
}

void LedCouleur::begin() {
    strip.begin();
    strip.show(); // Éteint la LED au démarrage
    strip.setBrightness(LUMINOSITE); // Luminosité par défaut
}

void LedCouleur::setLuminosite(uint8_t brightness) {
    strip.setBrightness(brightness);
    strip.show();
}

void LedCouleur::jaune() {
    strip.setPixelColor(0, strip.Color(255, 255, 0));
    strip.show();
}

void LedCouleur::vert() {
    strip.setPixelColor(0, strip.Color(0, 255, 0));
    strip.show();
}

void LedCouleur::rouge() {
    strip.setPixelColor(0, strip.Color(255, 0, 0));
    strip.show();
}

void LedCouleur::bleu() {
    strip.setPixelColor(0, strip.Color(0, 0, 255));
    strip.show();
}