
#include "Iambic.h"


// Constructeur

Iambic::Iambic(Modulation* m, Afficheur* a,int wpm,char _mode)
: mod(m), afficheur(a) {
  
    Keyboard.begin();  

  pinMode(LED_PIN, OUTPUT);
  pinMode(PIEZO_SPKR_PIN, OUTPUT);
  pinMode(TX_SWITCH_PIN, OUTPUT);

  // Setup input pins
  pinMode(LEFT_PADDLE_PIN, INPUT_PULLUP);   // sets Left Paddle digital pin as input
  pinMode(RIGHT_PADDLE_PIN, INPUT_PULLUP);  // sets Right Paddle digital pin as input
 
  digitalWrite(LED_PIN, LOW);        // turn the LED off
  digitalWrite(TX_SWITCH_PIN, LOW);  // Transmitter off

    g_current_morse_character = 0b11111110;
    g_keyerState = IDLE;

    if (_mode == 'A') {
        g_keyerControl = IAMBIC_A;
    } else {
        g_keyerControl = IAMBIC_B;
    }
    g_ditTime = 1200 / wpm;
        
    // Setup for Righthanded paddle by default
    g_dit_paddle = LEFT_PADDLE_PIN; // Dits on right hand thumb
    g_dah_paddle = RIGHT_PADDLE_PIN; // Dahs on right hand index finger

    g_sidetone_muted = true; // Default to no sidetone

    //vband
    // Etat des touches
    left_down = false;
    right_down = false;

    // Anti-rebond
    debounce_ms = 10;
    left_time = 0;
    right_time = 0;
}

// Destructeur
Iambic::~Iambic() {
    // Pas de delete ici car agrégation (pas propriétaire)
}

void Iambic::scan() {
    static long ktimer;

    // This state machine translates paddle input into DITS and DAHs and keys the transmitter.
    switch (g_keyerState) {
        case IDLE:
            // Wait for direct or latched paddle press
            if ((digitalRead(g_dit_paddle) == LOW) || (digitalRead(g_dah_paddle) == LOW) || (g_keyerControl & 0x03)) {
                update_PaddleLatch();
                g_current_morse_character = 0b11111110; // Sarting point for accumlating a character input via paddles
                g_keyerState = CHK_DIT;
            }
            break;

        case CHK_DIT:
            // See if the dit paddle was pressed
            if (g_keyerControl & DIT_L) {
                g_keyerControl |= DIT_PROC;
                ktimer = g_ditTime;
                g_current_morse_character = (g_current_morse_character << 1); // Shift a DIT (0) into the bit #0 position.
                g_keyerState = KEYED_PREP;
            } else {
                g_keyerState = CHK_DAH;
            }
            break;

        case CHK_DAH:
            // See if dah paddle was pressed
            if (g_keyerControl & DAH_L) {
                ktimer = g_ditTime * 3;
                g_current_morse_character = ((g_current_morse_character << 1) | 1); // Shift left one position and make bit #0 a DAH (1)
                g_keyerState = KEYED_PREP;
            } else {
                ktimer = millis() + (g_ditTime * 2); // Character space, is g_ditTime x 2 because already have a trailing intercharacter space
                g_keyerState = PRE_IDLE;
            }
            break;

        case KEYED_PREP:
            // Assert key down, start timing, state shared for dit or dah
            tx_key_down();

            ktimer += millis(); // set ktimer to interval end time
            g_keyerControl &= ~(DIT_L + DAH_L); // clear both paddle latch bits
            g_keyerState = KEYED; // next state
            break;

        case KEYED:
            // Wait for timer to expire
            if (millis() > ktimer) { // are we at end of key down ?
                tx_key_up();

                ktimer = millis() + g_ditTime; // inter-element time
                g_keyerState = INTER_ELEMENT; // next state

            } else if (g_keyerControl & IAMBIC_B) {
                update_PaddleLatch(); // early paddle latch check in Iambic B mode
            }
            break;

        case INTER_ELEMENT:
            // Insert time between dits/dahs
            update_PaddleLatch(); // latch paddle state
            if (millis() > ktimer) { // are we at end of inter-space ?
                if (g_keyerControl & DIT_PROC) { // was it a dit or dah ?
                    g_keyerControl &= ~(DIT_L + DIT_PROC); // clear two bits
                    g_keyerState = CHK_DAH; // dit done, check for dah
                } else {
                    g_keyerControl &= ~(DAH_L); // clear dah latch
                    ktimer = millis() + (g_ditTime * 2); // Character space, is g_ditTime x 2 because already have a trailing intercharacter space
                    g_keyerState = PRE_IDLE; // go idle
                }
            }
            break;

        case PRE_IDLE: // Wait for an intercharacter space

            // Check for direct or latched paddle press
            if ((digitalRead(g_dit_paddle) == LOW) || (digitalRead(g_dah_paddle) == LOW) || (g_keyerControl & 0x03)) {
                update_PaddleLatch();
                g_keyerState = CHK_DIT;
            } else { // Check for intercharacter space
                if (millis() > ktimer) { // We have a character
                    //Serial.println(g_current_morse_character);
                    afficheur->printCharCW(morse_char_decode(g_current_morse_character));
                    //Serial.print(morse_char_decode(g_current_morse_character));
                    g_keyerState = IDLE; // go idle
                }
            }
            break;
    }
}


///////////////////////////////////////////////////////////////////////////////
//
//    Latch dit and/or dah press
//
///////////////////////////////////////////////////////////////////////////////

void Iambic::update_PaddleLatch() {
  if (digitalRead(g_dit_paddle) == LOW) {
    g_keyerControl |= DIT_L;
  }
  if (digitalRead(g_dah_paddle) == LOW) {
    g_keyerControl |= DAH_L;
  }
}

///////////////////////////////////////////////////////////////////////////////
//       Key the Transmitter
///////////////////////////////////////////////////////////////////////////////
void Iambic::tx_key_down() {

    digitalWrite(LED_PIN, HIGH);             // turn the LED on
    digitalWrite(TX_SWITCH_PIN, HIGH);       // Turn the transmitter on
    mod->sendCarriage(true);
    if (!g_sidetone_muted) {
      tone(PIEZO_SPKR_PIN, SIDETONE_FREQ_HZ); // Sidetone on if not muted
    }
}


///////////////////////////////////////////////////////////////////////////////
//    Un-key the transmitter
///////////////////////////////////////////////////////////////////////////////
void Iambic::tx_key_up() {

  digitalWrite(LED_PIN, LOW);  // turn the LED off

    digitalWrite(TX_SWITCH_PIN, LOW);  // Turn the transmitter off
    mod->sendCarriage(false);
    if (!g_sidetone_muted) {
      noTone(PIEZO_SPKR_PIN);
    }
}



char Iambic::morse_char_decode(uint8_t code) {

  switch (code) {
    case 0b11111001: return 'A';
    case 0b11101000: return 'B';
    case 0b11101010: return 'C';
    case 0b11110100: return 'D';
    case 0b11111100: return 'E';
    case 0b11100010: return 'F';
    case 0b11110110: return 'G';
    case 0b11100000: return 'H';
    case 0b11111000: return 'I';
    case 0b11100111: return 'J';
    case 0b11110101: return 'K';
    case 0b11100100: return 'L';
    case 0b11111011: return 'M';
    case 0b11111010: return 'N';
    case 0b11110111: return 'O';
    case 0b11100110: return 'P';
    case 0b11101101: return 'Q';
    case 0b11110010: return 'R';
    case 0b11110000: return 'S';
    case 0b11111101: return 'T';
    case 0b11110001: return 'U';
    case 0b11100001: return 'V';
    case 0b11110011: return 'W';
    case 0b11101001: return 'X';
    case 0b11101011: return 'Y';
    case 0b11101100: return 'Z';

    case 0b11011111: return '0';
    case 0b11001111: return '1';
    case 0b11000111: return '2';
    case 0b11000011: return '3';
    case 0b11000001: return '4';
    case 0b11000000: return '5';
    case 0b11010000: return '6';
    case 0b11011000: return '7';
    case 0b11011100: return '8';
    case 0b11011110: return '9';

    case 0b11010010: return '/';
    case 0b10001100: return '?';
    case 0b11001010: return '*';   // AR
    case 0b00000000: return '#';   // ERROR

    case 0b11101111: return ' ';   // espace

    default: return '?'; // inconnu
  }
}

void Iambic::vband() {

 bool left_current = digitalRead(LEFT_PADDLE_PIN) == LOW;
  bool right_current = digitalRead(RIGHT_PADDLE_PIN) == LOW;
  unsigned long now = millis();

  // ----- LEFT (DIT) -----
  if (left_current != left_down && (now - left_time) > debounce_ms) {
    left_time = now;
    left_down = left_current;

    if (left_current) {
      Keyboard.press(VBAND_LEFT);
    } else {
      Keyboard.release(VBAND_LEFT);
    }
  }

  // ----- RIGHT (DAH) -----
  if (right_current != right_down && (now - right_time) > debounce_ms) {
    right_time = now;
    right_down = right_current;

    if (right_current) {
      Keyboard.press(VBAND_RIGHT);
    } else {
      Keyboard.release(VBAND_RIGHT);
    }
  }
 }