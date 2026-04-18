
#pragma once

#include <Arduino.h>
#include "hardware/pio.h"
#include "quadrature.pio.h"

// Pins
#define QUADRATURE_A_PIN 28
#define QUADRATURE_B_PIN 29
#define PUSH_PIN         27

class Encodeur {
public:
    Encodeur(PIO pio = pio1);

    void begin();

    // Lecture rotation
    int32_t getCount();
    void reset();

    // Bouton
    void updateButton();
    bool isPressed();       // état actuel
    bool wasPressed();      // front montant
    bool wasReleased();     // front descendant
    int8_t tick(); 

private:
    PIO _pio;
    uint _smA, _smB;
    uint _offsetA, _offsetB;

    int32_t _lastCount = 0;

    bool _btnState = false;
    bool _lastBtnState = false;
    bool _pressedEvent = false;
    bool _releasedEvent = false;
};
