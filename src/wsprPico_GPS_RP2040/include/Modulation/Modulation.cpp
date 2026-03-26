/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Modulation.cpp
 * Author: ale
 * 
 * Created on 28 août 2025, 07:55
 */

#include "Modulation.h"

Modulation::Modulation(config* _cfg) : cfg(_cfg) {
    setFreqWspr(cfg->freq+cfg->offset);
    setFreqRtty(cfg->freq+cfg->offset+500L);  //rtty 700Hz sous la fréquence wspr (plus simple pour écouter)
    setFreqFt8(7075000L);  //provisoire non implanté dans le menu
    anchor = this;
    pinMode(TEST_PIN,OUTPUT);
}

Modulation::Modulation(const Modulation& orig) {
}

Modulation::~Modulation() {
}

void Modulation::marshall() {
      // Clear the alarm irq
    hw_clear_bits(&timer_hw->intr, 1u << ALARM_NUM);
    // Reset the alarm register
    timer_hw->alarm[ALARM_NUM] = timer_hw->timerawl +  period_us ;
    anchor->timerCallback();  
}


void Modulation::startTimerIRQ(uint32_t us) {
    period_us = us;
     // Enable the interrupt for the alarm (we're using Alarm 0)
    hw_set_bits(&timer_hw->inte, 1u << ALARM_NUM) ;
    // Associate an interrupt handler with the ALARM_IRQ
    irq_set_exclusive_handler(ALARM_IRQ, marshall) ;
    // Enable the alarm interrupt
    irq_set_enabled(ALARM_IRQ, true) ;
    // Write the lower 32 bits of the target time to the alarm register, arming it.
    timer_hw->alarm[ALARM_NUM] = timer_hw->timerawl + period_us ;    
    irq_set_priority(ALARM_IRQ, 0); // Priorité haute

    Serial.println("init irq");
}

void Modulation::stopTimerIRQ() {
    irq_set_enabled(ALARM_IRQ, false);
    hardware_alarm_unclaim(ALARM_NUM);
}

void Modulation::timerCallback() {
    gpio_xor_mask(1u << TEST_PIN); // Toggle rapide
    flagIrq = true;
    //Serial.print("irq");
}

void Modulation::waitIrq(){
   while (!flagIrq){}
   flagIrq=false;  
}


void Modulation::sendWspr(char *locator) {
    
    memset(txBuffer, 0, WSPR_SYMBOL_COUNT);
    jtencode.wspr_encode(cfg->call, locator, cfg->dbm, txBuffer);
    startTimerIRQ(WSPR_DELAY*1000);
    flagIrq=false;
    for (uint8_t i = 0; i < WSPR_SYMBOL_COUNT; i++) {
        periods = wsfr[txBuffer[i]];    //modulation fsk sur 4 niveaux de fréquences avec un shift de 1.48 Hz le contenu de buffer est 0 ou 1 ou 2 ou 3
        //delay(WSPR_DELAY);
        waitIrq();
    }
    periods = 200 << 24; //dds off
    stopTimerIRQ();
}



void Modulation::sendRtty(char* stringRtty) {
const static int TableRtty[59] PROGMEM = { 4, 22, 17, 5, 18, 0, 11, 26, 30, 9, 0, 0, 6, 24, 7, 23, 13, 29, 25, 16, 10, 1, 21, 28, 12, 3, 14, 15, 0, 0, 0, 19, 0, 24, 19, 14, 18, 16, 22, 11, 5, 12, 26, 30, 9, 7, 6, 3, 13, 29, 10, 20, 1, 28, 15, 25, 23, 21, 17 };
                                          //tableau dans l'ordre ascii qui démarre à partir de l'espace 32
                                          //les valeurs représentent le code baudot
  int signlett = 1;  // RTTY Baudot signs/letters tables toggle
  char c;
    startTimerIRQ(RTTY_DELAY*1000);
  c = *stringRtty++;
  for (int n = 1; n < 20; n++) {  //sync on envoie une série de letters table code
    rttyTxByte(27);
  }

  while (c != '\0') {
    //Serial.print(c);
    c = toupper(int(c));  // Uppercase
    if (c == 10)          // Line Feed
    {
      rttyTxByte(8);
    } else if (c == 13)  // Carriage Return
    {
      rttyTxByte(2);
    } else if (c == 32)  // Space
    {
      rttyTxByte(4);
    } else if (c > 32 && c < 91) {
      c = c - 32;
      if (c < 33) {
        if (signlett == 1) {
          signlett = 0;          // toggle form signs to letters table
          rttyTxByte(27);  //
        }
      } else if (signlett == 0) {
        signlett = 1;          // toggle form letters to signs table
        rttyTxByte(31);  //
      }
      rttyTxByte(int(pgm_read_word(&TableRtty[int(c)])));  // Send the 5 bits word
    }
    c = *stringRtty++;  // Next character in string
  }
  periods = 200 << 24;  //dds off
  stopTimerIRQ();
}


void Modulation::rttyTxByte(char c) {
  int val;
  c = (c << 2) + 3;  //ajoute les deux bits de stop
  for (int b = 7; b >= 0; b--)  // MSB first
  {
    val = bitRead(c, b);  // Read 1 bit
    if (val == 0) periods = rttyfr[0];  //modulation fsk
    else periods = rttyfr[1]; 
    //delay(RTTY_DELAY);
     waitIrq();
  }
}

void Modulation::sendFt8(char *call, char *locator) {
    char buf[20];
    sprintf(buf, "CQ %s %s", call, locator);
    //Serial.println(buf);
    memset(txBuffer, 0, FT8_SYMBOL_COUNT);
    jtencode.ft8_encode(buf, txBuffer);

    //debug
    /*
    int n, lf;
    lf = 0;
    for (n = 0; n < FT8_SYMBOL_COUNT; n++) { //print symbols on serial monitor
        if (lf % 16 == 0) {
            Serial.println();
            Serial.print(n);
            Serial.print(": ");
        }
        lf++;
        Serial.print(txBuffer[n]);
        Serial.print(',');
    }
    Serial.println();
    */
    startTimerIRQ(FT8_DELAY*1000);
    flagIrq=false;
    for (uint8_t i = 0; i < FT8_SYMBOL_COUNT; i++) {
        periods = ft8fr[txBuffer[i]]; //modulation fsk sur 4 niveaux de fréquences avec un shift de 1.48 Hz le contenu de buffer est 0 ou 1 ou 2 ou 3
        //delay(FT8_DELAY);
        waitIrq();
        
    }
    periods = 200 << 24; //dds off
    stopTimerIRQ();
}



/***********************************************************************************
 * Hellschreiber 122.5 bauds => 8,163ms
 * http://brainwagon.org/2012/01/11/hellduino-sending-hellschreiber-from-an-arduino
 ************************************************************************************/

void Modulation::sendHell(char * stringHell) {
    static word GlyphTab[59][8] PROGMEM = {
        {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
        {0x1f9c, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
        {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
        {0x0330, 0x0ffc, 0x0330, 0x0ffc, 0x0330, 0x0000, 0x0000},
        {0x078c, 0x0ccc, 0x1ffe, 0x0ccc, 0x0c78, 0x0000, 0x0000},
        {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
        {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
        {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
        {0x01e0, 0x0738, 0x1c0e, 0x0000, 0x0000, 0x0000, 0x0000},
        {0x1c0e, 0x0738, 0x01e0, 0x0000, 0x0000, 0x0000, 0x0000},
        {0x018c, 0x0198, 0x0ff0, 0x0198, 0x018c, 0x0000, 0x0000},
        {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
        {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
        {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
        {0x001c, 0x001c, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
        {0x001c, 0x0070, 0x01c0, 0x0700, 0x1c00, 0x0000, 0x0000},
        {0x07f8, 0x0c0c, 0x0c0c, 0x0c0c, 0x07f8, 0x0000, 0x0000},
        {0x0300, 0x0600, 0x0ffc, 0x0000, 0x0000, 0x0000, 0x0000},
        {0x061c, 0x0c3c, 0x0ccc, 0x078c, 0x000c, 0x0000, 0x0000},
        {0x0006, 0x1806, 0x198c, 0x1f98, 0x00f0, 0x0000, 0x0000},
        {0x1fe0, 0x0060, 0x0060, 0x0ffc, 0x0060, 0x0000, 0x0000},
        {0x000c, 0x000c, 0x1f8c, 0x1998, 0x18f0, 0x0000, 0x0000},
        {0x07fc, 0x0c66, 0x18c6, 0x00c6, 0x007c, 0x0000, 0x0000},
        {0x181c, 0x1870, 0x19c0, 0x1f00, 0x1c00, 0x0000, 0x0000},
        {0x0f3c, 0x19e6, 0x18c6, 0x19e6, 0x0f3c, 0x0000, 0x0000},
        {0x0f80, 0x18c6, 0x18cc, 0x18cc, 0x0ff0, 0x0000, 0x0000},
        {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
        {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
        {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
        {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
        {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
        {0x1800, 0x1800, 0x19ce, 0x1f00, 0x0000, 0x0000, 0x0000},
        {0x01f0, 0x0208, 0x04e4, 0x0514, 0x0514, 0x03e0, 0x0000},
        {0x07fc, 0x0e60, 0x0c60, 0x0e60, 0x07fc, 0x0000, 0x0000},
        {0x0c0c, 0x0ffc, 0x0ccc, 0x0ccc, 0x0738, 0x0000, 0x0000},
        {0x0ffc, 0x0c0c, 0x0c0c, 0x0c0c, 0x0c0c, 0x0000, 0x0000},
        {0x0c0c, 0x0ffc, 0x0c0c, 0x0c0c, 0x07f8, 0x0000, 0x0000},
        {0x0ffc, 0x0ccc, 0x0ccc, 0x0c0c, 0x0c0c, 0x0000, 0x0000},
        {0x0ffc, 0x0cc0, 0x0cc0, 0x0c00, 0x0c00, 0x0000, 0x0000},
        {0x0ffc, 0x0c0c, 0x0c0c, 0x0ccc, 0x0cfc, 0x0000, 0x0000},
        {0x0ffc, 0x00c0, 0x00c0, 0x00c0, 0x0ffc, 0x0000, 0x0000},
        {0x0c0c, 0x0c0c, 0x0ffc, 0x0c0c, 0x0c0c, 0x0000, 0x0000},
        {0x003c, 0x000c, 0x000c, 0x000c, 0x0ffc, 0x0000, 0x0000},
        {0x0ffc, 0x00c0, 0x00e0, 0x0330, 0x0e1c, 0x0000, 0x0000},
        {0x0ffc, 0x000c, 0x000c, 0x000c, 0x000c, 0x0000, 0x0000},
        {0x0ffc, 0x0600, 0x0300, 0x0600, 0x0ffc, 0x0000, 0x0000},
        {0x0ffc, 0x0700, 0x01c0, 0x0070, 0x0ffc, 0x0000, 0x0000},
        {0x0ffc, 0x0c0c, 0x0c0c, 0x0c0c, 0x0ffc, 0x0000, 0x0000},
        {0x0c0c, 0x0ffc, 0x0ccc, 0x0cc0, 0x0780, 0x0000, 0x0000},
        {0x0ffc, 0x0c0c, 0x0c3c, 0x0ffc, 0x000f, 0x0000, 0x0000},
        {0x0ffc, 0x0cc0, 0x0cc0, 0x0cf0, 0x079c, 0x0000, 0x0000},
        {0x078c, 0x0ccc, 0x0ccc, 0x0ccc, 0x0c78, 0x0000, 0x0000},
        {0x0c00, 0x0c00, 0x0ffc, 0x0c00, 0x0c00, 0x0000, 0x0000},
        {0x0ff8, 0x000c, 0x000c, 0x000c, 0x0ff8, 0x0000, 0x0000},
        {0x0ffc, 0x0038, 0x00e0, 0x0380, 0x0e00, 0x0000, 0x0000},
        {0x0ff8, 0x000c, 0x00f8, 0x000c, 0x0ff8, 0x0000, 0x0000},
        {0x0e1c, 0x0330, 0x01e0, 0x0330, 0x0e1c, 0x0000, 0x0000},
        {0x0e00, 0x0380, 0x00fc, 0x0380, 0x0e00, 0x0000, 0x0000},
        {0x0c1c, 0x0c7c, 0x0ccc, 0x0f8c, 0x0e0c, 0x0000, 0x0000}
    };
    int val;
    char ch;
    word fbits;
    startTimerIRQ(HELL_DELAY);
    ch = *stringHell++;
    while (ch != '\0') {
        ch = toupper(int(ch)); // Uppercase
        if (ch >= 32 && ch <= 90) // Character is in the range of ASCII space to Z
        {
            ch -= 32; // Character number starting at 0
            for (int i = 0; i < 7; i++) // Scanning each 7 columns of glyph
            {
                fbits = int(pgm_read_word(&GlyphTab[int(ch)][i])); // Get each column of glyph
                for (int b = 0; b < 14; b++) // Scanning each 14 rows
                {
                    val = bitRead(fbits, b); // Get binary state of pixel
                    //DDS.setfreq(freqHell * val, 0); // Let's transmit (or not if pixel is clear)
                    if (val){
                        periods = rttyfr[0];  //utiliser la fréquence du rtty pour essais
                    }
                    else{
                       periods = 200 << 24;  //dds off 
                    }
                    waitIrq();
                    //delayMicroseconds(HELL_DELAY); //8,163ms/2
                }
            }
        }
        ch = *stringHell++; // Next character in string
    }      
    periods = 200 << 24;  //dds off
    stopTimerIRQ();
}

/***************************************************************************
 * CW
 ***************************************************************************/
void Modulation::sendCw(char * stringCw, int cwWpm) {
    static int morseVaricode[2][59] PROGMEM = {
        {0, 212, 72, 0, 144, 0, 128, 120, 176, 180, 0, 80, 204, 132, 84, 144, 248, 120, 56, 24, 8, 0, 128, 192, 224, 240, 224, 168, 0, 136, 0, 48, 104, 64, 128, 160, 128, 0, 32, 192, 0, 0, 112, 160, 64, 192, 128, 224, 96, 208, 64, 0, 128, 32, 16, 96, 144, 176, 192},
        {7, 6, 5, 0, 4, 0, 4, 6, 5, 6, 0, 5, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 0, 5, 0, 6, 6, 2, 4, 4, 3, 1, 4, 3, 4, 2, 4, 3, 4, 2, 2, 3, 4, 4, 3, 3, 1, 3, 4, 3, 4, 4, 4}
    };

    int tempo = 1200 / cwWpm; // Duration of 1 dot
    byte nb_bits, val;
    int d;
    int c = *stringCw++;
    while (c != '\0') {
        c = toupper(c); // Uppercase
        if (c == 32) { // Space character between words in string
            periods = 200 << 24; //dds off 7 dot length spacing
            delay(tempo * 7); // between words
        } else if (c > 32 && c < 91) {
            c = c - 32;
            d = int(pgm_read_word(&morseVaricode[0][c])); // Get CW varicode    
            nb_bits = int(pgm_read_word(&morseVaricode[1][c])); // Get CW varicode length
            if (nb_bits != 0) { // Number of bits = 0 -> invalid character #%<>
                for (int b = 7; b > 7 - nb_bits; b--) { // Send CW character, each bit represents a symbol (0 for dot, 1 for dash) MSB first 
                    val = bitRead(d, b); //look varicode

                    periods = rttyfr[0]; //utiliser la fréquence du rtty pour essais
                    delay(tempo + 2 * tempo * val); // A dot length or a dash length (3 times the dot)

                    periods = 200 << 24; //dds off 1 dot length spacing
                    delay(tempo); // between symbols in a character
                }
            }
            periods = 200 << 24; //dds off 3 dots length spacing
            delay(tempo * 3); // between characters in a word
        }
        c = *stringCw++; // Next caracter in string
    }
    periods = 200 << 24; //dds off 
}

/*
 prototype de transmission CW manchester
 */

void Modulation::setManchesterTempo(duration _tempo) {
  tempo = _tempo;
}

void Modulation::setManchesterTxMode(mode _txMode) {
  txMode = _txMode;
}



void Modulation::sendManchesterBit(bool bit) {
  if (bit) {
    periods = rttyfr[0];        //high emprunt de la freq rtty
    delay(tempo);
    periods = 200 << 24;   //low
    delay(tempo);
  }
  else {
    periods = 200 << 24;   //low
    delay(tempo);
    periods = rttyfr[0];  //high
    delay(tempo);
  }
}

void Modulation::sendManchesterByte(byte val)
{
  byte n, b;
  Serial.print(val,HEX);
  Serial.print(',');
  for (int i = 7; i >= 0; i--) {
    b = (val >> i) & 0x01;
    sendManchesterBit(b);
  }
}

void Modulation::sendManchesterFloat(float angle, byte *crc) {
  byte *ptr = (byte*) &angle;
  int i;
  for (i = 0; i < sizeof(float); i++) {
    sendManchesterByte(ptr[i]);
    *crc ^= ptr[i];
  }
}

void Modulation::sendManchesterVaricode(char c) {
  const static int PskVaricode[2][128] PROGMEM = {
    { 683, 731, 749, 887, 747, 863, 751, 765, 767, 239, 29, 879, 733, 31, 885, 939, 759, 757, 941, 943, 859, 875, 877,
      855, 891, 893, 951, 853, 861, 955, 763, 895, 1, 511, 351, 501, 475, 725, 699, 383, 251, 247, 367, 479, 117, 53,
      87, 431, 183, 189, 237, 255, 375, 347, 363, 429, 427, 439, 245, 445, 493, 85, 471, 687, 701, 125, 235, 173, 181,
      119, 219, 253, 341, 127, 509, 381, 215, 187, 221, 171, 213, 477, 175, 111, 109, 343, 437, 349, 373, 379, 685, 503,
      495, 507, 703, 365, 735, 11, 95, 47, 45, 3, 61, 91, 43, 13, 491, 191, 27, 59, 15, 7, 63, 447, 21, 23, 5, 55, 123, 107,
      223, 93, 469, 695, 443, 693, 727, 949
    },
    { 10, 10, 10, 10, 10, 10, 10, 10, 10, 8, 5, 10, 10, 5, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
      10, 1, 9, 9, 9, 9, 10, 10, 9, 8, 8, 9, 9, 7, 6, 7, 9, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 8, 9, 9, 7, 9, 10, 10, 7, 8, 8, 8, 7, 8, 8, 9, 7,
      9, 9, 8, 8, 8, 8, 8, 9, 8, 7, 7, 9, 9, 9, 9, 9, 10, 9, 9, 9, 10, 9, 10, 4, 7, 6, 6, 2, 6, 7, 6, 4, 9, 8, 5, 6, 4, 3, 6, 9, 5, 5, 3, 6,
      7, 7, 8, 7, 9, 10, 9, 10, 10, 10
    }
  };

  int d;
  byte nb_bits, val;
  d = int(pgm_read_word(&PskVaricode[0][c]));    // Get PSK varicode
  nb_bits = int(pgm_read_word(&PskVaricode[1][c])); // Get PSK varicode length
  d <<= 2; //add 00 on lsb for spacing between caracters
  for (int b = nb_bits + 2; b >= 0; b--) //send car in manchester
  {
    val = bitRead(d, b); //look varicode
    sendManchesterBit((bool) val);
  }
}

void Modulation::sendManchester(const char *txt) {
  sendManchesterByte(PREAMBULE);
  sendManchesterByte(txMode);
  byte crc = 0;
  while (*txt != '\0') {
    Serial.println(*txt);
    switch (txMode) {
      case BYTES_NO_CRC: sendManchesterByte((byte)*txt);
        break;
      case BYTES_WITH_CRC: sendManchesterByte((byte)*txt);
        crc ^= (byte) * txt;
        break;
      case VARICODE: sendManchesterVaricode((byte)*txt);
        break;
    }
    txt++;
  }
  if (txMode == BYTES_WITH_CRC) {
    sendManchesterByte(crc);
  }
   periods = 200 << 24;
}

void Modulation::sendManchesterLoc(const char *call, byte ssid, char symbol, float latitude, float longitude) {
  sendManchesterByte(PREAMBULE);
  sendManchesterByte(GEOLOC);  //id
  byte crc = 0;
  while (*call != '\0') {   //call
    //Serial.println(*call);
    sendManchesterByte((byte)*call);
    crc ^= (byte) *call;
    call++;
  }
  sendManchesterByte(ssid);       //ssid  de la place sur les 4 bits MSB
  crc ^= ssid;
  sendManchesterByte((byte)symbol);// symbole APRS
  crc ^= (byte)symbol;
  sendManchesterFloat(latitude, &crc);  //position
  sendManchesterFloat(longitude, &crc);
  sendManchesterByte(crc);              //crc
   periods = 200 << 24;
}


Modulation* Modulation::anchor = NULL;
uint32_t Modulation::period_us;

