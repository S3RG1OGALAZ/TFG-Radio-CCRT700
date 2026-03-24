#include "InputManager.h"

InputManager::InputManager() {
  // Pines 1 de los botones del CCRT700
  // Orden de índices: 0=1, 1=2, 2=3, 3=6, 4=7, 5=8
  _rowPins[0] = 52; // CCRT700 pin 1
  _rowPins[1] = 50; // CCRT700 pin 2
  _rowPins[2] = 48; // CCRT700 pin 3
  _rowPins[3] = 46; // CCRT700 pin 6
  _rowPins[4] = 44; // CCRT700 pin 7
  _rowPins[5] = 42; // CCRT700 pin 8

  // Pines 2 de los botones del CCRT700
  // Orden de índices: 0=4, 1=5, 2=12
  _colPins[0] = 53; // CCRT700 pin 4
  _colPins[1] = 51; // CCRT700 pin 5
  _colPins[2] = 49; // CCRT700 pin 12

  // Mapa interno de botones
  _config[BTN_TAPE]         = {3, 2}; // (6,12)
  _config[BTN_SOS]          = {4, 2}; // (7,12)
  _config[BTN_OI]           = {5, 2}; // (8,12)
  
  _config[BTN_ONSTAR]       = {0, 0}; // (1,4)
  _config[BTN_TUNER]        = {1, 0}; // (2,4)
  _config[BTN_EQUALIZADOR]  = {2, 0}; // (3,4)

  _config[BTN_CD]           = {0, 1}; // (1,5)
  _config[BTN_CLR]          = {1, 1}; // (2,5)

  for (int i = 0; i < BTN_TOTAL; i++) {
    _state[i] = HIGH;
    _lastReading[i] = HIGH;
    _pressEvent[i] = false;
    _lastDebounce[i] = 0;
  }
}

void InputManager::init() {
  // Asigna las columnas como entradas
  for (int i = 0; i < NUM_COLS; i++) {
    pinMode(_colPins[i], OUTPUT);
    digitalWrite(_colPins[i], HIGH); // columnas desactivadas
  }

// Asigna las filas como salidas
  for (int i = 0; i < NUM_ROWS; i++) {
    pinMode(_rowPins[i], INPUT_PULLUP); // filas con pull-up
  }
}

void InputManager::update() {
  unsigned long now = millis();

  // Bucle que activa cada columa individualmente
  for (int i = 0; i < NUM_COLS; i++) {
    digitalWrite(_colPins[i], LOW); // activa columna actual

    // Bucle que comprueba cada boton individualmente
    for (int j = 0; j < BTN_TOTAL; j++) {
      // Si el boton actual no tiene la columna activada actual, sale
      if (_config[j].col != i) {
        continue;
      }

      // Se lee el estado actual del boton
      byte rowIndex = _config[j].row;
      bool reading = digitalRead(_rowPins[rowIndex]);

      // Si cambia la lectura instantanea, reinicia debounce
      if (reading != _lastReading[j]) {
        _lastDebounce[j] = now;
        _lastReading[j] = reading;
      }

      // Si ya paso el tiempo de debounce, acepta el nuevo estado
      if ((now - _lastDebounce[j]) >= _keyChatterMillis) {
        if (_state[j] != reading) {
          _state[j] = reading;

          // Detectar flanco de pulsacion
          if (_state[j] == LOW) {
            _pressEvent[j] = true;
          }
        }
      }
    }

    digitalWrite(_colPins[i], HIGH); // desactiva columna actual
  }
}

bool InputManager::wasPressed(ButtonID btn) {
  if (_pressEvent[btn]) {
    _pressEvent[btn] = false;
    return true;
  }
  return false;
}