#include <TID12.h>
#include "InputManager.h"
#include "201bt.h"

// Configuracioon de los pines de la pantalla
TID tid(11, 9, 10);   // Pines SDA, SCL, MRQ
InputManager input;   // Control del panel frotal
BT201 bt(Serial1);    // 

void setup() {
  Serial.begin(9600);                 // Inicia comunicacion serie para debug
  Serial1.begin(115200);              // Inicia comunicacion serie copn el Modulo BT-201
  Serial.println("SETUP iniciado");   // Mensaje para saber que arranca

  tid.space1_init(0, 7);              // Configura toda la pantalla
  input.init();                       // Configura el input de los botones pulsados
  bt.begin(115200);
  delay(2000);
  tid.display_message("  OPEL  ", 255, 1);
}

void loop() {
  input.update();
  bt.update();

  if (bt.hasNewEvent()) {
    tid.display_message(bt.consumeLastEvent(), 255, 1);
  }

  if (input.wasPressed(BTN_TAPE)) {
    bt.playPause();
  }

  if (input.wasPressed(BTN_SOS)) {
    Serial.println("SOS pulsado");
    tid.display_message("SOS     ", 255, 1);
  }

  if (input.wasPressed(BTN_CD)) {
    Serial.println("CD pulsado");
    tid.display_message("CD      ", 255, 1);
  }

  if (Serial.available()) {
    Serial1.write(Serial.read());
  }

  if (Serial1.available()) {
    Serial.write(Serial1.read());
  }
}