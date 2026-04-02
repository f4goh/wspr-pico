/* 
* PicoWSPR 40m/30m
* https://www.elektronik-labor.de/Raspberry/Pico38.html
* 
* TX	GP1 (RX)
* RX	GP0 (TX)
* https://github.com/earlephilhower/arduino-pico
* one wire  GP2
* LED GP4
* BP MENU GP6 actif au NL0 (detection à la mise sous tension)
 * 
 * installer wsjt-x
 * https://wsjt.sourceforge.io/wsjtx.html
 * wsjtx_2.7.0_amd64.deb
 * sudo dpkg -i wsjtx_2.7.0_amd64.deb
 * sudo apt-get install -f
 * >show
Current configuration :
  freq           : 14097100 Hz
  offset         : -350
  call           : F4GOH
  locator        : JN07
  dbm            : 10 dBm
  mode           : WSPR
  wpm            : 12
  mail           : none@example.com
  minute         : 2 min
  nbframe        : 1
  Gps baud rate  : 9600
  Nmea debug  is : OFF
  Follow  is     : ON
>
 */


#include <Arduino.h>
#include <Menu.h> 
#include <TinyGPS++.h>

#include <Modulation.h> 
#include <Wire.h>
#include "Afficheur.h"
#include "LedCouleur.h"
#include "Locator.h"


#define DS3231_ADDRESS 0x68
#define PTT_PIN 7
#define MINUTES 10

TinyGPSPlus gps;


Menu *leMenu;
Modulation* mod;
Afficheur *afficheur;
LedCouleur led;
Locator loc;

void checkGps();
void computeLocator(int nbChar);
void initTimeTable(void);
txModes recherche(int minutes, int seconde);
void txing(txModes md);


void rtty();
void wspr();
void ft8();
void hell();
void cw();
void manchester();

typedef struct {
  float latitude;   // Latitude en degrés
  float longitude;  // Longitude en degrés
  float speedKmph;  // Vitesse en km/h
  float courseDeg;  // Cap en degrés
  char locator[9];  //= "JN07";  
} GpsData;

typedef struct {
    txModes mode;
    uint8_t minute;
    uint8_t seconde;
}timeTable;

timeTable timt[(60/MINUTES)*4];



int secondPrec = 0;

GpsData position;
config cfg;

volatile bool stateObjMod=false;

bool firstTxStart=false;


void setup(void) {
    set_sys_clock_khz(CLK_KHZ, true);
    Serial.begin(115200);
  
    Wire.setSDA(4);
    Wire.setSCL(5);
    
    Wire.begin();
    
    
    pinMode(PTT_PIN,OUTPUT);
    digitalWrite(PTT_PIN,LOW);

    leMenu = new Menu(); // Menu de configuration
    leMenu->setup();

    delete(leMenu);

     
    EEPROM.begin(sizeof (config));
    EEPROM.get(0, cfg);

    Serial1.setTX(0); // GP0 = TX vers GPS RX
    Serial1.setRX(1); // GP1 = RX depuis GPS TX
    Serial1.begin(cfg.baud); // Démarre UART1
    

    mod = new Modulation(&cfg);
    stateObjMod = true; //il faut que l'objet mod ait le temps de s'initialiser avant de lancer le dds dans le core 1
    afficheur = new Afficheur();
    afficheur->begin();
    
    Wire.setClock(400000);
    
    initTimeTable();
    led.begin();
    led.jaune();
    afficheur->configDisplay(cfg);
    delay(10000);
    
    
}

void loop() {
    checkGps();
}

void checkGps() {
    char car;
    char buffer[12];
    txModes md;
    while (Serial1.available()) {
        car = Serial1.read();
        gps.encode(car);
        if (cfg.nmeaEnabled) {
            Serial.print(car);
        }
    }



    //séquenceur
    if (gps.time.isValid() && gps.location.isUpdated()) {
        led.vert();
        int hour = gps.time.hour();
        int minute = gps.time.minute();
        int second = gps.time.second();
        //int day = gps.date.day();  //inutile
        //int month = gps.date.month();
        //int year = gps.date.year();

        if (second != secondPrec) {
            if (!cfg.nmeaEnabled) {
                snprintf(buffer, sizeof (buffer), "%02d:%02d:%02d", hour, minute, second);
                Serial.printf("Heure GPS: %s\n\r", buffer);
                afficheur->timeDisplay(buffer);
            }
            secondPrec = second;
            if (cfg.follow) {
                md=recherche(minute,second);
                if (md!=NOTX){
                    if (!firstTxStart) {
                        afficheur->clearDisplay();
                        firstTxStart = true;
                    }
                    txing(md);
                }

            } else {
                if (minute % cfg.minute == 0 && second == 0) { //peut etre une tramsission à faire
                    if (!firstTxStart) {
                        afficheur->clearDisplay();
                        firstTxStart = true;
                    }
                    txing(cfg.mode);
                }
            }
        }
    } else {
        //Serial.println("Heure GPS non disponible.");
        led.jaune();
    }
}
//returne un bool et change de couleur
void computeLocator(int nbChar) {
  uint8_t dbm;  
  if (gps.location.isUpdated()) {
    position.latitude = gps.location.lat();
    position.longitude = gps.location.lng();
    position.speedKmph = gps.speed.knots();
    position.courseDeg = gps.course.deg();
    loc.getLocator(position.latitude, position.longitude, nbChar, position.locator,&dbm);
    mod->setDbm(dbm);
    Serial.printf("Lat: %.6f, Lon: %.6f\n\r", position.latitude, position.longitude);
    Serial.printf("Speed: %.2f km/h, Course: %.2f°\n\r", position.speedKmph, position.courseDeg);
    Serial.printf("Locator: %s dbm %d\n\n\r", position.locator,dbm);
  } 
}

void txing(txModes md) {
    switch (md) { //manque la gestion de la répétition avec cfg.nbFrame
        case WSPR: wspr();
            break;
        case RTTY: rtty();
            break;
        case FT8: ft8();
            break;
        case HELL: hell();
            break;
        case CW: cw();
            break;
    }
}


void ft8() {
  led.rouge();
  Serial.println("Appel de la fonction ft8");
  computeLocator(8);
  afficheur->modeDisplay("FT8");
  mod->sendFt8(cfg.call,position.locator);
  afficheur->modeEfface();
  led.bleu();
}

void wspr() {
  led.rouge();
  Serial.println("Appel de la fonction WSPR");
  computeLocator(4);
  
  afficheur->modeDisplay("WSPR");
  digitalWrite(PTT_PIN,HIGH);
  mod->sendWspr(position.locator);
  digitalWrite(PTT_PIN,LOW);
  afficheur->modeEfface();
  led.bleu();
}

void rtty() {
  char buffer[300];
  led.rouge();
  Serial.println("Appel de la fonction RTTY");
  computeLocator(8);  
  sprintf(buffer, " %s BEACON LOC %s LAT:%.6f LON:%.6f SPD:%.1f KTS HDG:%.0f PSE REPORT %s ........\n\r", cfg.call, position.locator, position.latitude, position.longitude, position.speedKmph, position.courseDeg,cfg.mail);
  Serial.println(buffer);
  afficheur->modeDisplay("RTTY");
  mod->sendRtty(buffer);
  afficheur->modeEfface();
 led.bleu();
}

void hell() {
  char buffer[300];
  led.rouge();
  Serial.println("Appel de la fonction HELL");
  computeLocator(8);
  sprintf(buffer, "..... %s BEACON LOC %s ....\n\r", cfg.call, position.locator);
  Serial.println(buffer);
  afficheur->modeDisplay("HELL");
  mod->sendHell(buffer);
  afficheur->modeEfface();
  led.bleu();
}

void cw() {
  char buffer[300];
  led.rouge();
  Serial.println("Appel de la fonction CW");
  computeLocator(8);
  sprintf(buffer, " %s %s BEACON %s \n\r", cfg.call,cfg.call, position.locator);
  Serial.println(buffer);
  afficheur->modeDisplay(" CW");
  mod->sendCw(buffer,cfg.wpm);
  afficheur->modeEfface();
  led.bleu();
}

void initTimeTable(void)
{
    uint8_t idx = 0;

    for (uint8_t base = 0; base < 60; base += MINUTES)
    {
        // 2 WSPR
        timt[idx].mode = WSPR;
        timt[idx].minute = base + 0;
        timt[idx].seconde = 0;
        idx++;

        timt[idx].mode = WSPR;
        timt[idx].minute = base + 2;
        timt[idx].seconde = 0;
        idx++;

        // 2 FT8
        timt[idx].mode = FT8;
        timt[idx].minute = base + 4;
        timt[idx].seconde = 0;
        idx++;

        timt[idx].mode = FT8;
        timt[idx].minute = base + 4;
        timt[idx].seconde = 30;
        idx++;
    }
}

txModes recherche(int minutes, int seconde)
{
    uint8_t size = (60 / MINUTES) * 4;

    for (uint8_t i = 0; i < size; i++)
    {
        if (timt[i].minute == minutes && timt[i].seconde == seconde)
        {
            return timt[i].mode;
        }
    }

    return NOTX;
}

void setup1(void){
    while (!stateObjMod){}
    delay(1); //nécessaire pour que le dds démarre
    mod->coreUnSetup();
}

