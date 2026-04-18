#include "Encodeur.h"

Encodeur::Encodeur(PIO pio) {
    _pio = pio;
}

void Encodeur::begin() {
    pinMode(QUADRATURE_A_PIN, INPUT_PULLUP);
    pinMode(QUADRATURE_B_PIN, INPUT_PULLUP);
    pinMode(PUSH_PIN, INPUT_PULLUP);

    // Charger programmes PIO
    _offsetA = pio_add_program(_pio, &quadratureA_program);
    _smA = pio_claim_unused_sm(_pio, true);

    _offsetB = pio_add_program(_pio, &quadratureB_program);
    _smB = pio_claim_unused_sm(_pio, true);

    // Init PIO
    quadratureA_program_init(_pio, _smA, _offsetA, QUADRATURE_A_PIN, QUADRATURE_B_PIN);
    quadratureB_program_init(_pio, _smB, _offsetB, QUADRATURE_A_PIN, QUADRATURE_B_PIN);
}

int32_t Encodeur::getCount() {
    pio_sm_exec(_pio, _smA, pio_encode_mov(pio_isr, pio_x));
    pio_sm_exec(_pio, _smA, pio_encode_push(false, false));
    int32_t a = pio_sm_get_blocking(_pio, _smA);

    pio_sm_exec(_pio, _smB, pio_encode_mov(pio_isr, pio_x));
    pio_sm_exec(_pio, _smB, pio_encode_push(false, false));
    int32_t b = pio_sm_get_blocking(_pio, _smB);

    return a + b;
}

void Encodeur::reset() {
    _lastCount = getCount();
}

void Encodeur::updateButton() {
    _lastBtnState = _btnState;
    _btnState = !digitalRead(PUSH_PIN);

    _pressedEvent = (!_lastBtnState && _btnState);
    _releasedEvent = (_lastBtnState && !_btnState);
}

bool Encodeur::isPressed() {
    return _btnState;
}

bool Encodeur::wasPressed() {
    return _pressedEvent;
}

bool Encodeur::wasReleased() {
    return _releasedEvent;
}

int8_t Encodeur::tick() {
    static int32_t last = 0;

    int32_t current = getCount(); // ton PIO actuel

    int32_t diff = current - last;
    last = current;

    if (diff > 0) return +1;
    if (diff < 0) return -1;

    return 0;
}
