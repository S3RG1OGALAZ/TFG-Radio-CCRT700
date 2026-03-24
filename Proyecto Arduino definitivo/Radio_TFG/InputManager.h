#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <Arduino.h>

// Identificadores de botones
enum ButtonID {
  BTN_TAPE,
  BTN_SOS,
  BTN_OI,
  BTN_ONSTAR,
  BTN_TUNER,
  BTN_EQUALIZADOR,
  BTN_CD,
  BTN_CLR
};

struct ButtonConfig {
  byte row;
  byte col;
};

class InputManager {
public:
  InputManager();

  void init();                  // Inicializa pines
  void update();                // Escanea botones
  bool wasPressed(ButtonID btn); // Devuelve true una sola vez por pulsacion

private:
  static const byte NUM_ROWS  = 6;
  static const byte NUM_COLS  = 3;
  static const byte BTN_TOTAL = 8;

  byte _rowPins[NUM_ROWS];              // Pines del Mega para filas
  byte _colPins[NUM_COLS];              // Pines del Mega para columnas
  ButtonConfig _config[BTN_TOTAL];      // Mapa interno de botones

  bool _state[BTN_TOTAL];                  // Estado estable del boton
  bool _lastReading[BTN_TOTAL];              // Ultima lectura instantánea
  bool _pressEvent[BTN_TOTAL];             // Si el boton ha sido pulsado
  unsigned long _lastDebounce[BTN_TOTAL];

  const unsigned long _keyChatterMillis = 50; // Valor para evitar dobles pulsaciones
};

#endif