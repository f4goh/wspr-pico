/* 
 * File:   Menu.h
 * 
 * Created on 22 avril 2022, 17:11
 */

#ifndef MENU_H
#define MENU_H
#include <Arduino.h>
#include <console.h>
#include "hardware/watchdog.h"
#include <EEPROM.h>
#include <Wire.h>


#define MENU_PIN 6

typedef enum{
    WSPR,RTTY,FT8,HELL,CW     
}txModes;

typedef struct {
  uint32_t freq;
  char call[10];
  uint8_t dbm;
  char mail[31];
  int32_t offset;
  uint8_t minute;    // Durée en minutes
  uint8_t nbFrame;   // Nombre de trames à envoyer
  uint32_t baud;
  char locator[5];
  txModes mode; 
  bool nmeaEnabled;
  uint8_t wpm;
} config;



class Menu {
    
public:
    Menu();
    Menu(const Menu& orig);
    virtual ~Menu();
    
    void run();
    void setup();    

 private:
    // Méthodes associées aux commandes
    static void _call_(ArgList& L, Stream& S);
    static void _freq_(ArgList& L, Stream& S);
    static void _minute_(ArgList& L, Stream& S);
    static void _offset_(ArgList& L, Stream& S);
    static void _gpsbaud_(ArgList& L, Stream& S);
    static void _dbm_(ArgList& L, Stream& S);
    static void _nbframe_(ArgList& L, Stream& S);
    static void _mail_(ArgList& L, Stream& S);
    static void _settemp_(ArgList& L, Stream& S);
    static void _mode_(ArgList& L, Stream& S);
    static void _show_(ArgList& L, Stream& S);
    static void _raz_(ArgList& L, Stream& S);
    static void _save_(ArgList& L, Stream& S);
    static void _restart_(ArgList& L, Stream& S);
    static void _exit_(ArgList& L, Stream& S);
    static void _help_(ArgList& L, Stream& S);
    static void _unknown(String& L, Stream& S);
    static void _scan_(ArgList& L, Stream& S);
    static void _loc_(ArgList& L, Stream& S);
    static void _nmea_(ArgList& L, Stream& S);
    static void _wpm_(ArgList& L, Stream& S);
    
    //void displayRtc();
    
    bool exitFlag;
    Console *con;
    config cfg;
    static Menu* anchor;        
    bool acceptCmd(String cmd, int longMin, int longMax);
    void scanI2C();
    static const char* modeStr[5];       
    
};

#endif /* MENU_H */

