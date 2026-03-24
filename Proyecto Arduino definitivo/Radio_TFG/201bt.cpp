#include "201bt.h"

BT201::BT201(HardwareSerial& serial)
  : _serial(serial),
    _state(BT_UNKNOWN),
    _rxLine(""),
    _lastEvent(""),
    _newEvent(false) {
}

void BT201::begin(unsigned long baud) {
  _serial.begin(baud);
}

void BT201::update() {
  while (_serial.available()) {
    char c = (char)_serial.read();

    if (c == '\r') {
      continue;
    }

    if (c == '\n') {
      if (_rxLine.length() > 0) {
        handleLine(_rxLine);
        _rxLine = "";
      }
    } else {
      _rxLine += c;
    }
  }
}

void BT201::playPause() {
  sendCommand("AT+CB");
}

bool BT201::hasNewEvent() const {
  return _newEvent;
}

String BT201::consumeLastEvent() {
  _newEvent = false;
  return _lastEvent;
}

BT201::BtState BT201::getState() const {
  return _state;
}

void BT201::sendCommand(const String& cmd) {
  _serial.print(cmd);
  _serial.print("\r\n");
}

void BT201::handleLine(String line) {
  line.trim();

  if (line.length() == 0) {
    return;
  }

  Serial.print("[BT201] ");
  Serial.println(line);

  if (line == "TS+01") {
    _state = BT_CONNECTED;
    _lastEvent = "PAUSE   ";
    _newEvent = true;
    return;
  }

  if (line == "TS+02") {
    _state = BT_PLAYING;
    _lastEvent = "PLAY    ";
    _newEvent = true;
    return;
  }

if (line == "OK") {
    _state = BT_PLAYING;
    _lastEvent = "OK      ";
    _newEvent = true;
    return;
  }

  if (line.startsWith("ER+")) {
    _lastEvent = normalizeDisplay(line);
    _newEvent = true;
    return;
  }
}

String BT201::normalizeDisplay(String text) const {
  if (text.length() > 8) {
    text = text.substring(0, 8);
  }

  while (text.length() < 8) {
    text += ' ';
  }

  return text;
}