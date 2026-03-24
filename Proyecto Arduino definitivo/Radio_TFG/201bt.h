#ifndef BT201_MODULE_H
#define BT201_MODULE_H

#include <Arduino.h>

class BT201 {
public:
  enum BtState {
    BT_UNKNOWN = -1,
    BT_WAIT_PAIR = 0,   // TS+00
    BT_CONNECTED = 1,   // TS+01
    BT_PLAYING = 2,     // TS+02
    BT_IN_CALL = 3      // TS+03
  };

  explicit BT201(HardwareSerial& serial);

  void begin(unsigned long baud = 115200);
  void update();

  // Acción
  void playPause();

  // Eventos
  bool hasNewEvent() const;
  String consumeLastEvent();

  // Estado
  BtState getState() const;

private:
  HardwareSerial& _serial;

  BtState _state;
  String _rxLine;
  String _lastEvent;
  bool _newEvent;

  void sendCommand(const String& cmd);
  void handleLine(String line);
  String normalizeDisplay(String text) const;
};

#endif