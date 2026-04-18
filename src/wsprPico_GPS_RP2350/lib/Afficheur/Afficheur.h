/* 
 * File:   Afficheur.cpp
 * Author: F4GOH
 * 
 * spécialisation de SSD1306Wire
 */

#ifndef AFFICHEUR_H
#define AFFICHEUR_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include "Menu.h"


#define TFT_BL   21
#define TFT_DC   16
#define TFT_CS   17
#define TFT_SCK  18
#define TFT_MOSI 19
#define TFT_RST  20

#define X_OFF 34
#define Y_OFF 0


#define MSGX 2
#define MSGY 2

#define TEXT_SIZE      4
#define CHAR_WIDTH     (6 * TEXT_SIZE)
#define CHAR_HEIGHT    (8 * TEXT_SIZE)

#define SCREEN_WIDTH   172
#define SCREEN_HEIGHT  320

#define MAX_COLS       13
#define MAX_LINES      5


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
    void printCharCW(char car);
    void printFrequency(uint32_t freq);

    
private:
    //Adafruit_SSD1306 display;
     //U8G2_SSD1306_128X64_NONAME_F_HW_I2C display; //gfx
     Adafruit_ST7789 *tft; 
     
     int cursorX = 0;
int cursorY = 0;

bool firstChar = true;

int currentCol = 0;
int currentLine = 0;

 static const uint16_t lineColors[8];


};

#endif /* AFFICHEUR_H */


