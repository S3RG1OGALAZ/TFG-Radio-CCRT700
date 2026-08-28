#include <TID12.h>
#include "GestorEntradas.h"
#include "201bt.h"

// Configuracioon de los pines de la pantalla
TID pantalla(11, 9, 10);            // Pines SDA, SCL, MRQ de conexion con la pantalla
GestorEntradas gestorEntradas;      // Control del panel frotal del CCRT700
BT201 moduloBt(Serial1);            //

/*
  Modos de audio contemplados por la prueba de concepto.

  El modo radio representa por ahora el cambio de modo solicitado por el
  usuario. La recepcion FM real se incorporara al integrar el RDA5807.
*/
enum ModoAudio {
  MODO_BLUETOOTH,
  MODO_RADIO
};

ModoAudio modoAudioActual = MODO_BLUETOOTH;

String ultimoTextoPantalla = "";              // Texto que se esta mostrando actualmente en pantalla.
bool estadoConexionAnteriorRecibido = false;  // Indica la referencia anterior del estado de conexion
bool estabaConectadoAnteriormente = false;    // Guarda si en la ultima comprobacion el Bluetooth estaba conectado
String mensajeTemporalPantalla = "";          // Texto temporal
unsigned long inicioMensajeTemporal = 0;             // Momento en el que empezo a mostrarse el mensaje temporal
const unsigned long duracionMensajeTemporal = 1500;   // Tiempo que se mostrara el mensaje temporal antes de volver al estado normal

// Declaraciones de las funciones utilizadas por la aplicacion principal.
void cambiarModoAudio();
void mostrarMensajeTemporal(const String& mensaje);
bool bluetoothEstaConectado(BT201::EstadoBt estado);
bool bluetoothPermiteControlMusica();
void actualizarPantalla();

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
  actualizarPantalla();

  if (gestorEntradas.fuePulsado(BOTON_CINTA)) {
    if (bluetoothPermiteControlMusica()) {
      moduloBt.reproducirPausar();
    }
  }

  if (gestorEntradas.fuePulsado(BOTON_SINTONIZADOR)) {
    if (bluetoothPermiteControlMusica()) {
      moduloBt.pistaSiguiente();
      mostrarMensajeTemporal("NEXT    ");
    }
  }

  if (gestorEntradas.fuePulsado(BOTON_EQUALIZADOR)) {
    if (bluetoothPermiteControlMusica()) {
      moduloBt.pistaAnterior();
      mostrarMensajeTemporal("PREV    ");
    }
  }

  if (gestorEntradas.fuePulsado(BOTON_SOS)) {
    Serial.println("SOS pulsado");
  }

  if (gestorEntradas.fuePulsado(BOTON_CD)) {
    cambiarModoAudio();
  }
}

/*
  Cambia entre los modos contemplados por la prueba de concepto.

  El cambio a radio genera el estado y la respuesta visual necesarios para
  validar RF_V1. No activa todavia un receptor FM fisico.
*/
void cambiarModoAudio() {
  if (modoAudioActual == MODO_BLUETOOTH) {
    modoAudioActual = MODO_RADIO;
    mostrarMensajeTemporal("RADIO   ");
  } else {
    modoAudioActual = MODO_BLUETOOTH;
    mostrarMensajeTemporal("BT      ");
  }
}

/*
  Muestra un texto durante un tiempo limitado sin bloquear el loop.
*/
void mostrarMensajeTemporal(const String& mensaje) {
  mensajeTemporalPantalla = mensaje;
  inicioMensajeTemporal = millis();

  pantalla.display_message(mensajeTemporalPantalla, 255, 1);
  ultimoTextoPantalla = mensajeTemporalPantalla;
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
  Indica si se pueden enviar ordenes de reproduccion al BT201.

  Se evita mostrar NEXT o PREV cuando el modulo no esta conectado, esta en
  emparejamiento o se encuentra gestionando una llamada.
*/
bool bluetoothPermiteControlMusica() {
  if (modoAudioActual != MODO_BLUETOOTH || !moduloBt.estadoRecibido()) {
    return false;
  }

  BT201::EstadoBt estado = moduloBt.obtenerEstado();
  return estado == BT201::BT_CONECTADO ||
         estado == BT201::BT_REPRODUCIENDO;
}

/*
  Actualiza la pantalla segun el estado Bluetooth.

  Funcionamiento:
  1. Lee el ultimo estado guardado en el modulo BT201.
  2. Detecta si ha cambiado de desconectado a conectado o al reves.
  3. Si hay cambio, muestra un mensaje temporal.
  4. Cuando termina el mensaje temporal, vuelve a mostrar el estado normal.
*/
void actualizarPantalla() {
  /*
    Los mensajes temporales tienen prioridad independientemente del modo.
  */
  if (mensajeTemporalPantalla.length() > 0) {
    if ((millis() - inicioMensajeTemporal) < duracionMensajeTemporal) {
      return;
    }

    mensajeTemporalPantalla = "";
  }

  /*
    El receptor FM aun no esta integrado. Se mantiene visible el modo logico
    para validar el cambio de modo sin mostrar un estado Bluetooth incorrecto.
  */
  if (modoAudioActual == MODO_RADIO) {
    String textoModo = "RADIO   ";

    if (textoModo != ultimoTextoPantalla) {
      pantalla.display_message(textoModo, 255, 1);
      ultimoTextoPantalla = textoModo;
    }

    return;
  }

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

    mostrarMensajeTemporal(mensajeTemporalPantalla);

    return;
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
