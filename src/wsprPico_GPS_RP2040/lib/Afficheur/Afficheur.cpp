/* 
 * File:   Afficheur.cpp
 * Author: F4GOH
 * 
 */

#include "Afficheur.h"

// Constructeur de Afficheur

Afficheur::Afficheur() :
    //display(U8G2_R0, /* reset=*/ U8X8_PIN_NONE)
    display()
{
    // Rien ici, l'initialisation se fait dans begin()
}

Afficheur::~Afficheur() {
    // Rien à nettoyer
}

void Afficheur::begin() {
    // Initialisation I2C avec adresse 0x3C
    display.begin();
    display.setI2CAddress(SCREEN_ADDRESS << 1); // U8g2 attend l'adresse sur 8 bits
    display.setPowerSave(0); // Active l'écran
    display.setFont(u8x8_font_chroma48medium8_r); // Police rapide
    
    
}

void Afficheur::timeDisplay(char* txt) {
    //version txt
    display.clearLine(0); // Efface la ligne 0
    display.setFont(u8x8_font_chroma48medium8_r);
    display.drawString(4, 0, txt); // Affiche le texte sur la ligne 0  
}

void Afficheur::modeDisplay(const char* txt) {
    display.setFont(u8x8_font_courB24_3x4_f); // Police fixe 8x8
    display.drawString(MSGX, MSGY, txt);
}

void Afficheur::modeEfface() {
    display.setFont(u8x8_font_courB24_3x4_f); // Police plus grande
    display.drawString(MSGX, MSGY, "    "); // 4 espaces pour nettoyer
}


void Afficheur::clearDisplay(){
    display.clear();  // Efface tout l'écran  
}


void Afficheur::configDisplay(config& cfg) {
    char line[17];
    display.clear();  // Efface tout l'écran
    display.setFont(u8x8_font_chroma48medium8_r);

    // Ligne 0 : vide
    //display.drawString(0, 0, "");  // Optionnel, pour clarté

    // Ligne 1 : fréquence et offset
   
    snprintf(line, sizeof(line), "f=%lu %ld", cfg.freq, cfg.offset);
    display.drawString(0, 1, line);

    // Ligne 2 : puissance, durée, trames
    
    snprintf(line, sizeof(line), "dbm%u mn%u nb%u", cfg.dbm, cfg.minute, cfg.nbFrame);
    display.drawString(0, 2, line);

    // Ligne 3 : GPS et baudrate
    
    snprintf(line, sizeof(line), "gps baud %lu", cfg.baud);
    display.drawString(0, 3, line);

    // Ligne 4 : locator
    
    snprintf(line, sizeof(line), "locator %.4s", cfg.locator);
    display.drawString(0, 4, line);

    // Ligne 5 : mode
    const char* modeStr[] = { "WSPR", "RTTY", "FT8", "HELL", "CW" };
    
    snprintf(line, sizeof(line), "mode %s %s", modeStr[cfg.mode],cfg.call);
    display.drawString(0, 5, line);

    // Ligne 6 : WPM
    
    snprintf(line, sizeof(line), "wpm %u nmea %s", cfg.wpm,cfg.nmeaEnabled ? "on" : "off");
    display.drawString(0, 6, line);
    
}

