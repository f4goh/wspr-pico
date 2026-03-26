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
 * 
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

TinyGPSPlus gps;


Menu *leMenu;
Modulation* mod;
Afficheur *afficheur;
LedCouleur led;
Locator loc;

void checkGps();
void computeLocator(int nbChar);
//void readfiletest();

void rtty();
void wspr();
void ft8();
void hell();
void cw();
void manchester();

char locator[9];  //= "JN07";  //local provisoire

typedef struct {
  float latitude;   // Latitude en degrés
  float longitude;  // Longitude en degrés
  float speedKmph;  // Vitesse en km/h
  float courseDeg;  // Cap en degrés
} GpsData;


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
    while (Serial1.available()) {
        car = Serial1.read();
        gps.encode(car);
        if (cfg.nmeaEnabled) {
            Serial.print(car);
        }
    }



    //séquenceur en fonction des 2 tableaux précalculés
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
                snprintf(buffer, sizeof(buffer),"%02d:%02d:%02d", hour, minute, second);
                Serial.printf("Heure GPS: %s\n\r",buffer);
                //Serial.printf("Heure GPS: %02d:%02d:%02d\n\r", hour, minute, second);
                afficheur->timeDisplay(buffer);
                
            }
            secondPrec = second;
            if (minute % cfg.minute == 0 && second == 0) { //peut etre une tramsission à faire
                if (!firstTxStart) {                        //efface l'écran dès la 1ere transmission
                    afficheur->clearDisplay();
                    firstTxStart=true;
                }
                
                switch (cfg.mode) { //manque la gestion de la répétition avec cfg.nbFrame
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
                
               // manchester();
            }
        }
    } else {
        //Serial.println("Heure GPS non disponible.");
        led.jaune();
    }
}
//returne un bool et change de couleur
void computeLocator(int nbChar) {
  if (gps.location.isUpdated()) {
    position.latitude = gps.location.lat();
    position.longitude = gps.location.lng();
    position.speedKmph = gps.speed.knots();
    position.courseDeg = gps.course.deg();
    loc.getLocator(position.latitude, position.longitude, nbChar, locator);
    Serial.printf("Lat: %.6f, Lon: %.6f\n\r", position.latitude, position.longitude);
    Serial.printf("Speed: %.2f km/h, Course: %.2f°\n\r", position.speedKmph, position.courseDeg);
    Serial.printf("Locator: %s\n\n\r", locator);
  } 
}


void ft8() {
  led.rouge();
  Serial.println("Appel de la fonction ft8");
  computeLocator(4);
  afficheur->modeDisplay("FT8");
  mod->sendFt8(cfg.call,locator);
  afficheur->modeEfface();
  led.bleu();
}

void wspr() {
  led.rouge();
  Serial.println("Appel de la fonction WSPR");
  computeLocator(4);
  afficheur->modeDisplay("WSPR");
  digitalWrite(PTT_PIN,HIGH);
  mod->sendWspr(locator);
  digitalWrite(PTT_PIN,LOW);
  afficheur->modeEfface();
  led.bleu();
}

void rtty() {
  char buffer[300];
  led.rouge();
  Serial.println("Appel de la fonction RTTY");
  computeLocator(8);  
  sprintf(buffer, " %s BEACON LOC %s LAT:%.6f LON:%.6f SPD:%.1f KTS HDG:%.0f PSE REPORT %s ........\n\r", cfg.call, locator, position.latitude, position.longitude, position.speedKmph, position.courseDeg,cfg.mail);
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
  sprintf(buffer, "..... %s BEACON LOC %s ....\n\r", cfg.call, locator);
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
  sprintf(buffer, " %s %s BEACON %s \n\r", cfg.call,cfg.call, locator);
  Serial.println(buffer);
  afficheur->modeDisplay(" CW");
  mod->sendCw(buffer,cfg.wpm);
  afficheur->modeEfface();
  led.bleu();
}



void setup1(void){
    while (!stateObjMod){}
    delay(1); //nécessaire pour que le dds démarre
    mod->coreUnSetup();
}

