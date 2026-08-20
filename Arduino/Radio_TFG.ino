#include <TID12.h>
#include "GestorEntradas.h"
#include "201bt.h"

// Configuracioon de los pines de la pantalla
TID pantalla(11, 9, 10);            // Pines SDA, SCL, MRQ de conexion con la pantalla
GestorEntradas gestorEntradas;      // Control del panel frotal del CCRT700
BT201 moduloBt(Serial1);            //

String ultimoTextoPantalla = "";              // Texto que se esta mostrando actualmente en pantalla.
bool estadoConexionAnteriorRecibido = false;  // Indica la referencia anterior del estado de conexion
bool estabaConectadoAnteriormente = false;    // Guarda si en la ultima comprobacion el Bluetooth estaba conectado
String mensajeTemporalPantalla = "";          // Texto temporal
unsigned long inicioMensajeTemporal = 0;             // Momento en el que empezo a mostrarse el mensaje temporal
const unsigned long duracionMensajeTemporal = 1500;   // Tiempo que se mostrara el mensaje temporal antes de volver al estado normal

void setup() {
  Serial.begin(9600);                       // Inicia comunicacion serie para debug
  
  moduloBt.iniciar(115200);                 // Indica la velocidad de comunicacion del serial con el modulo BT
  gestorEntradas.iniciar();                 // Configura el gestor de entradas de los botones pulsados

  pantalla.space1_init(0, 7);               // Configura toda la pantalla
  delay(100);
  pantalla.display_message("   <>   ", 255, 1);  // pone un primer mensaje nada mas encender
  delay(200);
  pantalla.display_message("  <  >  ", 255, 1);  // pone un primer mensaje nada mas encender
  delay(200);
  pantalla.display_message(" <    > ", 255, 1);  // pone un primer mensaje nada mas encender
  delay(200);
  pantalla.display_message("<      >", 255, 1);  // pone un primer mensaje nada mas encender
  delay(200);
  pantalla.display_message("TFG  SER", 255, 1);  // pone un primer mensaje nada mas encender
  delay(1000);
}

void loop() {
  gestorEntradas.actualizar();
  moduloBt.actualizar();

  /*
    Actualiza la pantalla en funcion del ultimo estado Bluetooth conocido.

    No consulta AT+TS.
    Solo usa el estado que ya haya recibido el modulo.
  */
  actualizarPantallaBluetooth();

  if (gestorEntradas.fuePulsado(BOTON_CINTA)) {
    moduloBt.reproducirPausar();
  }

  if (gestorEntradas.fuePulsado(BOTON_SOS)) {
    Serial.println("SOS pulsado");
  }

  if (gestorEntradas.fuePulsado(BOTON_CD)) {
    Serial.println("CD pulsado");
  }
}

/*
  Indica si el estado Bluetooth se considera conectado.

  Se considera conectado cuando:
  - Esta conectado pero en pausa.
  - Esta reproduciendo.
  - Esta en llamada.

  No se considera conectado cuando:
  - Esta esperando emparejamiento.
  - Aun no se conoce el estado.
*/
bool bluetoothEstaConectado(BT201::EstadoBt estado) {
  return estado == BT201::BT_CONECTADO ||
         estado == BT201::BT_REPRODUCIENDO ||
         estado == BT201::BT_EN_LLAMADA;
}

/*
  Actualiza la pantalla segun el estado Bluetooth.

  Funcionamiento:
  1. Lee el ultimo estado guardado en el modulo BT201.
  2. Detecta si ha cambiado de desconectado a conectado o al reves.
  3. Si hay cambio, muestra un mensaje temporal.
  4. Cuando termina el mensaje temporal, vuelve a mostrar el estado normal.
*/
void actualizarPantallaBluetooth() {
  /*
    Primero se comprueba si ya se ha recibido algun estado real del BT201.

    Si aun no se ha recibido ningun TS+xx, se muestra un texto de espera.
  */
  if (!moduloBt.estadoRecibido()) {
    String textoEstado = moduloBt.obtenerTextoPantallaEstado();

    if (textoEstado != ultimoTextoPantalla) {
      pantalla.display_message(textoEstado, 255, 1);
      ultimoTextoPantalla = textoEstado;
    }

    return;
  }

  /*
    Obtiene el ultimo estado Bluetooth guardado.

    No envia ningun comando al modulo.
    Solo lee la variable interna actualizada previamente al recibir TS+xx.
  */
  BT201::EstadoBt estadoActual = moduloBt.obtenerEstado();
  bool conectadoAhora = bluetoothEstaConectado(estadoActual);

  /*
    Detecta cambios entre conectado y desconectado.

    La primera vez solo guarda el estado, pero no muestra mensaje temporal.
    Asi se evita que al arrancar aparezca "DISCNCTD" sin que haya ocurrido
    realmente una desconexion.
  */
  if (!estadoConexionAnteriorRecibido) {
    estadoConexionAnteriorRecibido = true;
    estabaConectadoAnteriormente = conectadoAhora;
  } else if (conectadoAhora != estabaConectadoAnteriormente) {
    estabaConectadoAnteriormente = conectadoAhora;

    /*
      Si pasa de desconectado a conectado, muestra CONECTED.
      Si pasa de conectado a desconectado, muestra DISCNCTD.

      Ambos textos tienen 8 caracteres para encajar en la pantalla.
    */
    if (conectadoAhora) {
      mensajeTemporalPantalla = "CONECTED";
    } else {
      mensajeTemporalPantalla = "DISCNCTD";
    }

    inicioMensajeTemporal = millis();

    pantalla.display_message(mensajeTemporalPantalla, 255, 1);
    ultimoTextoPantalla = mensajeTemporalPantalla;

    return;
  }

  /*
    Si hay un mensaje temporal activo, se mantiene hasta que pase
    duracionMensajeTemporal.
  */
  if (mensajeTemporalPantalla.length() > 0) {
    if ((millis() - inicioMensajeTemporal) < duracionMensajeTemporal) {
      return;
    }

    /*
      Cuando termina el tiempo del mensaje temporal, se limpia para volver
      al estado normal.
    */
    mensajeTemporalPantalla = "";
  }

  /*
    Texto normal de estado:
    - PAIR
    - PAUSE
    - PLAY
    - CALL
  */
  String textoEstado = moduloBt.obtenerTextoPantallaEstado();

  /*
    Solo se refresca la pantalla si el texto ha cambiado.
  */
  if (textoEstado != ultimoTextoPantalla) {
    pantalla.display_message(textoEstado, 255, 1);
    ultimoTextoPantalla = textoEstado;
  }
}
