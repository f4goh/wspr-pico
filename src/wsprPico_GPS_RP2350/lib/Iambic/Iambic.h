/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/file.h to edit this template
 */

/* 
 * File:   Iambic.h
 * Author: ale
 *
 * Created on 9 avril 2026, 18:42
 */

#ifndef IAMBIC_H
#define IAMBIC_H

#include <Arduino.h>
#include <Modulation.h> 
#include "Afficheur.h"
#include <Keyboard.h>


#define VBAND_LEFT '['
#define VBAND_RIGHT ']'


// Keyer define
#define SIDETONE_FREQ_HZ 600           // Frequency for the keyer sidetone
#define DEFAULT_SPEED_WPM 10           // 15 WPM default keyer speed
#define KEYER_MODE_IS_IAMBIC_A       // Remove the '//' at the beginning of the line if you prefer Iambic A, otherwise the code will use Iambic B. If you don't use squeeze keying it doesn't matter. 

#define LEFT_PADDLE_PIN 2    // Left paddle input pin
#define RIGHT_PADDLE_PIN 8  // Right paddle input pin
#define PIEZO_SPKR_PIN 25    // Piezo Speaker connection via 100 ohm resistor
#define LED_PIN 3
#define TX_SWITCH_PIN 26   // Radio T/R Switch - HIGH is Key down (TX)), LOW is key up (RX).

#define DIT_L 0x01     // Dit latch (Binary 0000 0001)
#define DAH_L 0x02     // Dah latch (Binary 0000 0010)
#define DIT_PROC 0x04  // Dit is being processed (Binary 0000 0100)
#define IAMBIC_B 0x10  // 0x00 for Iambic A, 0x10 for Iambic B (Binary 0001 0000)
#define IAMBIC_A 0

// Keyer State machine type - states for sending of Morse elements based on paddle input
enum KSTYPE {
  IDLE,
  CHK_DIT,
  CHK_DAH,
  KEYED_PREP,
  KEYED,
  INTER_ELEMENT,
  PRE_IDLE,
};


// Data type for storing Keyer settings in EEPROM
struct KeyerConfigType {
  uint32_t ms_per_dit;         // Speed
  uint8_t dit_paddle_pin;      // Dit paddle pin number
  uint8_t dah_paddle_pin;      // Dah Paddle pin number
  uint8_t iambic_keying_mode;  // 0 - Iambic-A, 1 Iambic-B
  uint8_t sidetone_is_muted;   //
};



class Iambic {
private:
    Modulation* mod;
    Afficheur* afficheur;

    uint32_t g_ditTime; // Number of milliseconds per dit
    uint8_t g_keyerControl; // 0x1 Dit latch, 0x2 Dah latch, 0x04 Dit being processed, 0x10 for Iambic A or 1 for B
    KSTYPE g_keyerState; // Keyer state global variable
    bool g_sidetone_muted; // If 'true' the Piezo speaker output is muted except for when in command mode


    // We declare these as variables so that DIT and DAH paddles can be swapped for both left and right hand users. Convention is DIT on thumb.
    uint8_t g_dit_paddle; // Current PIN number for Dit paddle
    uint8_t g_dah_paddle; // Current PIN number for Dah paddle

    // We use this variable to encode Morse elements (DIT = 0, DAH =1) to capture the current character sent via the paddles which is needed for user commands.
    // The encoding practice is to left pad the character with 1's. A zero start bit is to the left of the first encoded element.
    // We start with B11111110 and shift left with a 0 or 1 from the least significant bit, according the last Morse element received (DIT = 0, DAH =1).
    uint8_t g_current_morse_character;

    // Etat des touches
    bool left_down;
    bool right_down;

    // Anti-rebond
    unsigned long debounce_ms;
    unsigned long left_time;
    unsigned long right_time;



    char morse_char_decode(uint8_t code);
    void update_PaddleLatch();
    void tx_key_down();
    void tx_key_up();
    
public:
    // Constructeur
    Iambic(Modulation* m, Afficheur* a,int wpm,char _mode);

    // Destructeur
    ~Iambic();

    void scan();
    void vband();

};

#endif // IAMBIC_H