/* 
 * File:   Menu.cpp
 * 
 * Created on 22 avril 2022, 17:11
 */

#include "Menu.h"

const char* Menu::modeStr[6] = {"WSPR", "RTTY", "FT8", "HELL", "CW", "IAMBIC"};

const char* Menu::bandStr[7] = {"160m", "80m", "60m", "40m", "30m", "20m", "free frequency"};
 
Menu::Menu() :
exitFlag(false),
con(new Console())      
{
    anchor = this;
    pinMode(MENU_PIN, INPUT_PULLUP); // GP6 avec résistance interne activée
    tabfreq[0] = {1838100, -50};
    tabfreq[1] = {3570100, -150};
    tabfreq[2] = {5288700, -200};
    tabfreq[3] = {7040100, -300};
    tabfreq[4] = {10140200, -400};
    tabfreq[5] = {14097100, -550};
}

Menu::Menu(const Menu& orig) {
}

Menu::~Menu() {
    anchor = NULL;
}

void Menu::run() {
    exitFlag = false;
    if (digitalRead(MENU_PIN) == 0) {
        EEPROM.begin(sizeof (config));
        EEPROM.get(0, cfg);
        Serial.println(F("help command for info"));
        while (exitFlag == false) {
            con->run();
        }
    }
    Serial.println(F("\nConsole exit"));
}

void Menu::setup() {
 
    con->onCmd("call", _call_);
    con->onCmd("freq", _freq_);
    con->onCmd("minute",_minute_);
    con->onCmd("offset", _offset_);
    con->onCmd("gpsbaud", _gpsbaud_);
    con->onCmd("dbm", _dbm_);
    con->onCmd("mail", _mail_);
    con->onCmd("show", _show_);
    con->onCmd("raz", _raz_);
    con->onCmd("restart", _restart_);
    con->onCmd("save", _save_);
    con->onCmd("exit", _exit_);
    con->onCmd("help", _help_);
    con->onCmd("scan", _scan_);
    con->onCmd("loc", _loc_);
    con->onCmd("mode", _mode_);
    con->onCmd("band", _band_);
    con->onCmd("iambic", _iambic_);    
    con->onCmd("nmea", _nmea_);
    con->onCmd("wpm", _wpm_);
    con->onCmd("follow", _follow_);
 
    con->onUnknown(_unknown);
    con->start();
    this->run();
}

void Menu::_exit_(ArgList& L, Stream& S) {
    anchor->exitFlag = true;

}

void Menu::_help_(ArgList& L, Stream& S) {
    S.println(F("Available commands"));
    S.println(F("Set transmission frequency            : freq 7040100"));
    S.println(F("Set offset value                      : offset -200"));
    S.println(F("Set callsign                          : call F4XYZ"));
    S.println(F("Set locator                           : loc JN07"));
    S.println(F("Set mode  wspr rtty ft8 hell cw       : mode wspr"));
    S.println(F("Set band 160,80m,60m,40m,30m,20m,freq : band 40m"));    
    S.println(F("Set modulo in minutes                 : minute 10"));
    S.println(F("Set follow mode                       : follow 1"));
    S.println(F("Set cw words per minute               : wpm 15"));
    S.println(F("Set iambic mode                       : iambic B"));
    S.println(F("Set transmission power (dBm)          : dbm 10"));
    S.println(F("Set email address                     : mail f4xyz at example.com"));
    S.println(F("Set gps baud rate                     : gpsbaud 9600"));
    S.println(F("Scan I2C bus                          : scan"));
    S.println(F("Show nmea frame                       : nmea 1"));
    S.println(F("Save current configuration to EEPROM  : save"));
    S.println(F("Display current configuration         : show"));
    S.println(F("Reset all parameters to default       : raz"));
    S.println(F("Restart RPI pico                      : restart"));
    S.println(F("Show this help message                : help"));
    S.println(F("Exit menu                             : exit"));
}


bool Menu::acceptCmd(String cmd, int longMin, int longMax) {

    if (cmd.length() >= longMin && cmd.length() <= longMax) {
        return true;
    } else {
        Serial.println("Erreur");
        return false;
    }
}

void Menu::_unknown(String& L, Stream& S) {
    S.print(L);
    S.println(" : command not found");
}

void Menu::_call_(ArgList& L, Stream& S) {
    String p;
    bool ret;
    p = L.getNextArg();
    ret = anchor->acceptCmd(p, 4, 9);
    if (ret == true) {
        p.toUpperCase();
        size_t len = strlen(p.c_str());

        if (len < sizeof (cfg.call)) {
            //memcpy(anchor->cfg.call, p.c_str(), len);
            //anchor->cfg.call[len] = '\0'; // Ajout manuel du terminateur
            strcpy(anchor->cfg.call, p.c_str());
            S.printf("Callsign set to: %s\n\r", anchor->cfg.call);
        } else {
            S.println(F("Callsign too long. Max 9 characters."));
        }
    } else {
        S.println(F("Invalid callsign format."));
    }

}

void Menu::_loc_(ArgList& L, Stream& S) {
    String p;
    bool ret;
    p = L.getNextArg();
    ret = anchor->acceptCmd(p, 4, 4);
    if (ret == true) {
        p.toUpperCase();
            strcpy(anchor->cfg.locator, p.c_str());
            S.printf("Locator set to: %s\n\r", anchor->cfg.locator);
    } else {
        S.println(F("Invalid locator format ex JN07"));
    }
}


void Menu::_mode_(ArgList& L, Stream& S) {
    String p = L.getNextArg();
    p.toLowerCase();  // Normalise la casse pour comparaison
    if (p == "wspr") {
        anchor->cfg.mode = WSPR;
    } else if (p == "rtty") {
        anchor->cfg.mode = RTTY;
    } else if (p == "ft8") {
        anchor->cfg.mode = FT8;
    } else if (p == "hell") {
        anchor->cfg.mode = HELL;
    } else if (p == "cw") {
        anchor->cfg.mode = CW;
    } else if (p == "iambic") {
        anchor->cfg.mode = IAMBIC;
    } else {
        S.println(F("Invalid mode syntax. Example: mode wspr"));
        return;
    }
    // Affiche le mode sélectionné    
    S.printf("Mode set to: %s\n\r", modeStr[anchor->cfg.mode]);
}

void Menu::_band_(ArgList& L, Stream& S) {
    String p = L.getNextArg();
    p.toLowerCase(); // Normalise la casse pour comparaison
    if (p == "160m") {
        anchor->cfg.band = M160;
        anchor->cfg.freq = anchor->tabfreq[0].freq;
        anchor->cfg.offset = anchor->tabfreq[0].offset;
    } else if (p == "80m") {
        anchor->cfg.band = M80;
        anchor->cfg.freq = anchor->tabfreq[1].freq;
        anchor->cfg.offset = anchor->tabfreq[1].offset;
    } else if (p == "60m") {
        anchor->cfg.band = M60;
        anchor->cfg.freq = anchor->tabfreq[2].freq;
        anchor->cfg.offset = anchor->tabfreq[2].offset;
    } else if (p == "40m") {
        anchor->cfg.band = M40;
        anchor->cfg.freq = anchor->tabfreq[3].freq;
        anchor->cfg.offset = anchor->tabfreq[3].offset;
    } else if (p == "30m") {
        anchor->cfg.band = M30;
        anchor->cfg.freq = anchor->tabfreq[4].freq;
        anchor->cfg.offset = anchor->tabfreq[4].offset;
    } else if (p == "20m") {
        anchor->cfg.band = M20;
        anchor->cfg.freq = anchor->tabfreq[5].freq;
        anchor->cfg.offset = anchor->tabfreq[5].offset;
    } else if (p == "freq") {
        anchor->cfg.band = FREQ;
    } else {
        S.println(F("Invalid mode syntax. Example: band 40m"));
        return;
    }
    // Affiche le mode sélectionné    
    S.printf("Band set to: %s\n\r", bandStr[anchor->cfg.band]);
}



//manque le test de la fréquence max supportée par le RPI
void Menu::_freq_(ArgList& L, Stream& S) {
    String p;
    bool ret;
    p = L.getNextArg();
    ret = anchor->acceptCmd(p, 6, 8);
    if (ret == true) {
        uint32_t freqValue = p.toInt(); // Conversion de la chaîne en entier
        anchor->cfg.freq = freqValue; // Sauvegarde dans la structure
        S.printf("Frequency set to: %lu Hz\n\r", anchor->cfg.freq);
    }
}


void Menu::_minute_(ArgList& L, Stream& S) {
    String p;
    bool ret;
    p = L.getNextArg();
    ret = anchor->acceptCmd(p, 1, 2);
    if (ret == true) {
        uint8_t minuteValue = (uint8_t) p.toInt(); // Conversion en entier 8 bits
        anchor->cfg.minute = minuteValue; // Sauvegarde dans la structure
        S.printf("Minute modulo set to: %u\n\r", anchor->cfg.minute);
    }
}

void Menu::_wpm_(ArgList& L, Stream& S) {
    String p;
    bool ret;
    p = L.getNextArg();
    ret = anchor->acceptCmd(p, 1, 2);
    if (ret == true) {
        uint8_t wpm = (uint8_t) p.toInt(); // Conversion en entier 8 bits
        anchor->cfg.wpm = wpm; // Sauvegarde dans la structure
        S.printf("WPM set to: %u\n\r", anchor->cfg.wpm);
    }
}


//facteur de correction
void Menu::_offset_(ArgList& L, Stream& S) {
    String p;
    bool ret;
    p = L.getNextArg();
    ret = anchor->acceptCmd(p, 1, 5);
    if (ret == true) {
        int32_t offsetValue = p.toInt(); // Conversion en entier signé
        anchor->cfg.offset = offsetValue; // Sauvegarde dans la structure
        S.printf("Offset set to: %ld\n\r", anchor->cfg.offset);
    }
}

void Menu::_gpsbaud_(ArgList& L, Stream& S) {
    String p;
    bool ret;
    p = L.getNextArg();
    ret = anchor->acceptCmd(p, 4, 5);
    if (ret == true) {
        uint32_t baudValue = p.toInt(); // Conversion en entier non signé
        anchor->cfg.baud = baudValue; // Sauvegarde dans la structure
        S.printf("GPS baudrate set to: %lu\n\r", anchor->cfg.baud);
    }
}



//les valeurs de dbm sont normalisées la table pour vérification
void Menu::_dbm_(ArgList& L, Stream& S) {
    String p = L.getNextArg();
    bool ret = anchor->acceptCmd(p, 1, 2);

    if (ret == true) {
        int inputValue = p.toInt();

        // Tableau des puissances disponibles
        const uint8_t dbmTable[] = {
            0, 3, 7, 10, 13, 17, 20, 23, 27, 30,
            33, 37, 40, 43, 47, 50, 53, 57, 60
        };
        const size_t tableSize = sizeof(dbmTable) / sizeof(dbmTable[0]);

        // Recherche de la valeur la plus proche
        uint8_t closest = dbmTable[0];
        int minDiff = abs(inputValue - dbmTable[0]);

        for (size_t i = 1; i < tableSize; ++i) {
            int diff = abs(inputValue - dbmTable[i]);
            if (diff < minDiff) {
                minDiff = diff;
                closest = dbmTable[i];
            }
        }
        // Mise à jour de la configuration
        anchor->cfg.dbm = closest;
        S.printf("Transmission power set to: %u dBm (closest to %d)\n\r", closest, inputValue);
    }
}



void Menu::_mail_(ArgList& L, Stream& S) {
    String arg;
    bool ret;
    String p = "";
    while (!(arg = L.getNextArg()).isEmpty()) {
        p = p + arg + " ";
    }
    ret = anchor->acceptCmd(p, 3, 30);
    p.toUpperCase();
    size_t len = strlen(p.c_str());
    if (len < sizeof (anchor->cfg.mail)) {
        //memcpy(anchor->cfg.mail, p.c_str(), len);
        //anchor->cfg.mail[len] = '\0'; // Ajout manuel du terminateur
        strcpy(anchor->cfg.mail, p.c_str());
        S.printf("Email set to: %s\n\r", anchor->cfg.mail);
    } else {
        S.println(F("Email too long. Max 30 characters."));
    }
}



void Menu::_nmea_(ArgList& L, Stream& S) {
    String p;
    bool ret;
    p = L.getNextArg();
    ret = anchor->acceptCmd(p, 1, 1);
    if (ret == true) {
        int val = p.toInt(); // Conversion en entier
        if (val == 0 || val == 1) {
            anchor->cfg.nmeaEnabled = (val == 1); // Affectation booléenne

            S.printf("Nmea frame debug %s\n\r", anchor->cfg.nmeaEnabled ? "enabled" : "disabled");
        } else {
            S.println(F("Invalid value. Use 0 to disable or 1 to enable nmea debug."));
        }
    }
}

void Menu::_iambic_(ArgList& L, Stream& S) {
    String p;
    bool ret;
    p = L.getNextArg();
    ret = anchor->acceptCmd(p, 1, 1);
    if (ret == true) {
        char val = toupper(p[0]); // Conversion en char
        if (val == 'A' || val == 'B') {
            anchor->cfg.bicmd = val; // Affectation booléenne

            S.printf("Iambic type %c\n\r", anchor->cfg.bicmd);
        } else {
            S.println(F("Invalid value. Use A or B type for iambic paddle."));
        }
    }
}



void Menu::_follow_(ArgList& L, Stream& S) {
    String p;
    bool ret;
    p = L.getNextArg();
    ret = anchor->acceptCmd(p, 1, 1);
    if (ret == true) {
        int val = p.toInt(); // Conversion en entier
        if (val == 0 || val == 1) {
            anchor->cfg.follow = (val == 1); // Affectation booléenne

            S.printf("Follow is %s\n\r", anchor->cfg.follow ? "enabled" : "disabled");
        } else {
            S.println(F("Invalid value. Use 0 to disable or 1 to enable follow mode."));
        }
    }
}

void Menu::_show_(ArgList& L, Stream& S) {
    S.println("Current configuration :");
    S.printf("  freq           : %lu Hz\n\r", anchor->cfg.freq);
    S.printf("  offset         : %ld\n\r", anchor->cfg.offset);
    S.printf("  call           : %s\n\r", anchor->cfg.call);
    S.printf("  locator        : %s\n\r", anchor->cfg.locator);
    S.printf("  dbm            : %u dBm\n\r", anchor->cfg.dbm);
    S.printf("  mode           : %s\n\r", modeStr[anchor->cfg.mode]);
    S.printf("  band           : %s\n\r", bandStr[anchor->cfg.band]);
    S.printf("  wpm            : %u\n\r", anchor->cfg.wpm);
    S.printf("  iambic mode    : %c\n\r", anchor->cfg.bicmd);    
    S.printf("  mail           : %s\n\r", anchor->cfg.mail);
    S.printf("  minute         : %u min\n\r", anchor->cfg.minute);
    S.printf("  Gps baud rate  : %u\n\r", anchor->cfg.baud);
    S.printf("  Nmea debug  is : %s\n\r", anchor->cfg.nmeaEnabled ? "ON" : "OFF");
    S.printf("  Follow  is     : %s\n\r", anchor->cfg.follow ? "ON" : "OFF");
}

void Menu::_save_(ArgList& L, Stream& S) {
  EEPROM.put(0, anchor->cfg);
  EEPROM.commit();
  Serial.println("Configuration saved.");
}

void Menu::_raz_(ArgList& L, Stream& S) {

    anchor->cfg.freq = 7040100; // Fréquence par défaut
    anchor->cfg.offset = -200; // Fréquence par défaut
    strcpy(anchor->cfg.call, "NOCALL"); // Indicatif par défaut
    anchor->cfg.dbm = 10; // Puissance par défaut
    strcpy(anchor->cfg.mail, "none@example.com"); // Email par défaut
    strcpy(anchor->cfg.locator, "JN07"); // Locator par défaut
    anchor->cfg.minute = 6; // Durée par défaut
    anchor->cfg.baud=9600;  //9600 bauds
    anchor->cfg.mode=WSPR;
    anchor->cfg.nmeaEnabled=false;
    anchor->cfg.wpm=12;
    anchor->cfg.follow=0;
    anchor->cfg.band = FREQ;
    Serial.println("Configuration has been reset to default values.");    
}

void Menu::_restart_(ArgList& L, Stream& S) {
    watchdog_reboot(0, 0, 0);  // Redémarre immédiatement    
}


void Menu::_scan_(ArgList& L, Stream& S) {
    anchor->scanI2C();
}

void Menu::scanI2C() {
    Serial.println("    00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f");
    for (uint8_t row = 0; row < 8; row++) {
        Serial.printf("%02x: ", row << 4);
        for (uint8_t col = 0; col < 16; col++) {
            uint8_t address = (row << 4) | col;
            Wire.beginTransmission(address);
            uint8_t error = Wire.endTransmission();

            if (error == 0) {
                Serial.printf("%02x ", address);
            } else if (error == 4) {
                Serial.print("UU "); // Bus error
            } else {
                Serial.print("-- ");
            }
        }
        Serial.println();
    }
}



Menu* Menu::anchor = NULL;

