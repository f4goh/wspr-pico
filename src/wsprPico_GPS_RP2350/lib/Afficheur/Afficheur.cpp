/* 
 * File:   Afficheur.cpp
 * Author: F4GOH
 * 
 */

#include "Afficheur.h"

// Constructeur de Afficheur

Afficheur::Afficheur()
{
    tft = new Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
}

Afficheur::~Afficheur() {
    // Rien à nettoyer
}

void Afficheur::begin() {
    // Initialisation I2C avec adresse 0x3C
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    SPI.setSCK(TFT_SCK);
    SPI.setTX(TFT_MOSI);
    tft->init(172, 320);
    tft->setRotation(1);
    tft->setAddrWindow(34, 0, 172, 320);
    tft->fillScreen(ST77XX_BLACK);
tft->setTextWrap(false);
    
}

const uint16_t Afficheur::lineColors[8] = {
    ST77XX_YELLOW,
    ST77XX_CYAN,
    ST77XX_GREEN,
    ST77XX_MAGENTA,
    ST77XX_ORANGE,
    ST77XX_WHITE,
    ST77XX_RED,
    ST77XX_BLUE
};

void Afficheur::timeDisplay(char* txt) {
    tft->setTextSize(4);
    tft->setTextColor(ST77XX_CYAN, ST77XX_BLACK);

    const int width = 172;
    const int height = 22;

    // Efface uniquement la zone du haut
    tft->fillRect(0, 0, width, height, ST77XX_BLACK);

    // Petit padding
    tft->setCursor(46, 0);
    tft->print(txt);
}

void Afficheur::modeDisplay(const char* txt) {
    int16_t x, y;
    uint16_t w, h;

    tft->setTextSize(4);
    tft->setTextColor(ST77XX_YELLOW, ST77XX_BLACK);

    tft->getTextBounds(txt, 0, 0, &x, &y, &w, &h);

    const int width = 320;
    const int height = 172;

    // Centrage parfait en portrait
    int cx = (width - w) / 2;
    int cy = (height - h) / 2;

    // Efface uniquement la zone centrale (évite de flinguer le reste)
    int clearY = cy - 4;
    int clearH = h + 8;

    tft->fillRect(0, clearY, width, clearH, ST77XX_BLACK);

    tft->setCursor(cx, cy);
    tft->print(txt);
}

void Afficheur::modeEfface() {
    const int width = 172;
    // Zone centrale approximative (même logique que modeDisplay)
    tft->fillRect(0, 120, width, 80, ST77XX_BLACK);
}

void Afficheur::clearDisplay() {
    tft->fillScreen(ST77XX_BLACK);
}

void Afficheur::configDisplay(config& cfg) {
    char line[40];

    // Efface uniquement la fenêtre définie
    tft->fillScreen(ST77XX_BLACK);

    tft->setTextSize(2);

    const int x = 0;        // relatif à la fenêtre
    const int lineHeight = 22;

    int y = 10;

    const char * bandStr[7] = {"160m", "80m", "60m", "40m", "30m", "20m", "free frequency"};
    // Ligne 1 : fréquence et offset
    tft->setTextColor(lineColors[0], ST77XX_BLACK);
    snprintf(line, sizeof(line), " f=%lu %ld band %s", cfg.freq, cfg.offset,bandStr[cfg.band]);
    tft->setCursor(x, y);
    tft->print(line);
    y += lineHeight;

    // Ligne 2
    tft->setTextColor(lineColors[1], ST77XX_BLACK);
    snprintf(line, sizeof(line), "dbm%u every %u minutes", cfg.dbm, cfg.minute);
    tft->setCursor(x, y);
    tft->print(line);
    y += lineHeight;

    // Ligne 3
    tft->setTextColor(lineColors[2], ST77XX_BLACK);
    snprintf(line, sizeof(line), "gps baud %lu", cfg.baud);
    tft->setCursor(x, y);
    tft->print(line);
    y += lineHeight;

    // Ligne 4
    tft->setTextColor(lineColors[3], ST77XX_BLACK);
    snprintf(line, sizeof(line), "loc %.4s", cfg.locator);
    tft->setCursor(x, y);
    tft->print(line);
    y += lineHeight;

    // Ligne 5
    const char* modeStr[] = { "WSPR", "RTTY", "FT8", "HELL", "CW","IAMBIC" };
    tft->setTextColor(lineColors[4], ST77XX_BLACK);
    snprintf(line, sizeof(line), "mode %s %s", modeStr[cfg.mode], cfg.call);
    tft->setCursor(x, y);
    tft->print(line);
    y += lineHeight;

    // Ligne 6
    tft->setTextColor(lineColors[5], ST77XX_BLACK);
    snprintf(line, sizeof(line), "wpm %u nmea %s",
             cfg.wpm,
             cfg.nmeaEnabled ? "on" : "off");
    tft->setCursor(x, y);
    tft->print(line);
    y += lineHeight;
    
    // Ligne 7
    tft->setTextColor(lineColors[6], ST77XX_BLACK);
    snprintf(line, sizeof(line), "Iambic type %c follow %s",
             cfg.bicmd,
             cfg.follow ? "on" : "off");
    tft->setCursor(x, y);
    tft->print(line);
    
}

void Afficheur::printCharCW(char car) {

    if (firstChar) {
       // tft->fillScreen(ST77XX_BLACK);
         tft->fillRect(
        0,
        CHAR_HEIGHT * 1,                         // début à la ligne 1
        SCREEN_HEIGHT,
        CHAR_HEIGHT * (MAX_LINES - 1),           // hauteur des lignes 1 à 4
        ST77XX_BLACK
    );
         
        tft->setTextSize(TEXT_SIZE);
        tft->setTextWrap(false);

        currentCol = 0;
        currentLine = 1;
        firstChar = false;
    }

    // retour ligne AVANT écriture
    if (currentCol >= MAX_COLS) {
        currentCol = 0;
        currentLine++;
    }

    // écran plein
    if (currentLine >= MAX_LINES) {
        //tft->fillScreen(ST77XX_BLACK);
        
    tft->fillRect(
        0,
        CHAR_HEIGHT * 1,                         // début à la ligne 1
        SCREEN_HEIGHT,
        CHAR_HEIGHT * (MAX_LINES - 1),           // hauteur des lignes 1 à 4
        ST77XX_BLACK
    );
         
        currentLine = 1;
    }

    int x = currentCol * CHAR_WIDTH;
    int y = currentLine * CHAR_HEIGHT;

    // 🔥 couleur selon la ligne
    uint16_t color = lineColors[currentLine % (sizeof(lineColors) / sizeof(lineColors[0]))];
    tft->setTextColor(color, ST77XX_BLACK);

    tft->setCursor(x, y);
    tft->write(car);

    currentCol++;
}


void Afficheur::printFrequency(uint32_t freq) {

    char buf[16];
    snprintf(buf, sizeof(buf), "%lu", freq);

    uint16_t color = lineColors[0];

    tft->setTextSize(TEXT_SIZE);
    tft->setTextWrap(false);
    tft->setTextColor(color, ST77XX_BLACK);

    // position ligne 0
    int x = 20;
    int y = 0;

    tft->setCursor(x, y);
    tft->print(buf);
}