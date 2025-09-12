#include <TID12.h>
#include <SoftwareSerial.h>

String mensaje = "12345678";
int numeroSuma = 0;
byte velocidad = 100;
TID12 tid(7, 5, 6);

SoftwareSerial bt(10, 11); // RX, TX (usa pines que no sean 0 y 1)

void setup() {
  delay(1000);

  tid.space1_init(0, 2);
  tid.space2_init(3, 4);
  tid.space3_init(5, 8);

  // put your setup code here, to run once:
  tid.display_message(mensaje, velocidad, 1);

  Serial.begin(115200);   // Para depurar en el Monitor Serie
  bt.begin(115200);       // Comunicación con el BT201
}

void loop() {
  // put your main code here, to run repeatedly:

  //String mensajeCompleto = mensaje + numeroSuma;
  
  tid.display_message(mensaje, velocidad, 1);
  bt.println("AT+CB");

  delay(1000);

  numeroSuma++;

  if (bt.available()) {  // Si hay datos del BT201
    char c = bt.read();
    Serial.print(c);  // Muestra en la consola del PC
    tid.display_message(String(c), velocidad, 3);
  }
}
