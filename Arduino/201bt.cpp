#include "201bt.h"

/*
  Constructor de la clase BT201.

  Inicializa:
  - El puerto serie recibido por referencia.
  - El estado inicial como desconocido.
  - El indicador de estado recibido a false.
  - Los buffers de texto vacios.
  - El indicador de evento nuevo a false.
*/
BT201::BT201(HardwareSerial& serial)
  : _serial(serial),
    _estado(BT_DESCONOCIDO),
    _estadoRecibido(false),
    _lineaRecibida(""),
    _ultimoEvento(""),
    _nuevoEvento(false) {
}

/*
  Inicializa el puerto serie del modulo Bluetooth.

  El valor de baudios se recibe desde fuera, aunque en el .h tiene
  un valor por defecto de 115200.
*/
void BT201::iniciar(unsigned long baudios) {
  _serial.begin(baudios);
}

/*
  Actualiza la comunicacion con el modulo BT201.

  Este metodo solo lee las respuestas que envia el modulo.
  No envia consultas automaticamente para evitar saturar o desestabilizar
  el BT201.

  Debe llamarse continuamente desde el loop principal.
*/
void BT201::actualizar() {
  // Primero se leen todos los caracteres disponibles en el puerto serie.
  // El modulo envia respuestas terminadas en salto de linea.
  // Se van acumulando caracteres en _lineaRecibida hasta recibir '\n'.

  while (_serial.available()) {
    char caracter = (char)_serial.read();

    // Ignora el retorno de carro para quedarse solo con el salto de linea.
    if (caracter == '\r') {
      continue;
    }

    // Si llega un salto de linea, se considera que la linea esta completa.
    if (caracter == '\n') {
      if (_lineaRecibida.length() > 0) {
        procesarLinea(_lineaRecibida);
        _lineaRecibida = "";
      }
    } else {
      // Si no es fin de linea, se anade el caracter al buffer temporal.
      _lineaRecibida += caracter;
    }
  }
}

/*
  Envia el comando de reproducir/pausar al modulo BT201.

  El comando AT+CB alterna entre reproduccion y pausa.
*/
void BT201::reproducirPausar() {
  enviarComando("AT+CB");
}

/*
  Indica si hay un evento nuevo pendiente de consumir.

  Devuelve true si se ha recibido un nuevo estado o mensaje del modulo.
*/
bool BT201::hayNuevoEvento() const {
  return _nuevoEvento;
}

/*
  Devuelve el ultimo evento recibido y lo marca como consumido.

  Despues de llamar a este metodo, hayNuevoEvento() devolvera false
  hasta que se reciba otro evento nuevo.
*/
String BT201::consumirUltimoEvento() {
  _nuevoEvento = false;
  return _ultimoEvento;
}

/*
  Devuelve el ultimo estado conocido del modulo Bluetooth.
*/
BT201::EstadoBt BT201::obtenerEstado() const {
  return _estado;
}

/*
  Indica si ya se ha recibido al menos un estado real del modulo.

  Sirve para saber si obtenerEstado() contiene informacion real del modulo
  o si todavia sigue en el valor inicial BT_DESCONOCIDO.
*/
bool BT201::estadoRecibido() const {
  return _estadoRecibido;
}

/*
  Devuelve el ultimo estado conocido como texto.

*/
String BT201::obtenerTextoEstado() const {
  switch (_estado) {
    case BT_ESPERANDO_EMPAREJAMIENTO:
      return "ESPERANDO";

    case BT_CONECTADO:
      return "CONECTADO";

    case BT_REPRODUCIENDO:
      return "REPRODUCIENDO";

    case BT_EN_LLAMADA:
      return "EN LLAMADA";

    case BT_DESCONOCIDO:
    default:
      return "DESCONOCIDO";
  }
}

/*
  Envia un comando al modulo Bluetooth.

  Todos los comandos se terminan con retorno de carro y salto de linea,
  que es el formato habitual esperado por comandos AT.
*/
void BT201::enviarComando(const String& comando) {
  _serial.print(comando);
  _serial.print("\r\n");
}

/*
  Procesa una linea completa recibida desde el modulo BT201.

  Aqui se interpretan las respuestas del modulo:
  - Estados Bluetooth TS+xx.
  - Confirmaciones OK.
  - Errores ER+xx.
*/
void BT201::procesarLinea(String linea) {
  // Elimina espacios, saltos de linea o caracteres sobrantes al inicio y final.
  linea.trim();

  // Si la linea esta vacia, no hay nada que procesar.
  if (linea.length() == 0) {
    return;
  }

  // Muestra por el monitor serie lo recibido desde el modulo BT201.
  Serial.print("[BT201] ");
  Serial.println(linea);

  // TS+00: Bluetooth no conectado. Modulo esta esperando emparejamiento o conexion.
  if (linea == "TS+00") {
    guardarEstado(BT_ESPERANDO_EMPAREJAMIENTO, "PAIR    ");
    return;
  }

  // TS+01 indica que el modulo esta conectado pero sin reproducir.
  if (linea == "TS+01") {
    guardarEstado(BT_CONECTADO, "PAUSE   ");
    return;
  }

  // TS+02 indica que el modulo esta reproduciendo audio.
  if (linea == "TS+02") {
    guardarEstado(BT_REPRODUCIENDO, "PLAY    ");
    return;
  }

  // TS+03: Bluetooth en llamada.
  if (linea == "TS+03") {
    guardarEstado(BT_EN_LLAMADA, "CALL    ");
    return;
  }

  // OK indica que el modulo ha aceptado correctamente un comando.
  // Importante: OK no modifica el estado Bluetooth guardado.
  if (linea == "OK") {
    _ultimoEvento = "OK      ";
    _nuevoEvento = true;
    return;
  }

  // ER+ indica un error o mensaje especial del modulo.
  if (linea.startsWith("ER+")) {
    _ultimoEvento = normalizarPantalla(linea);
    _nuevoEvento = true;
    return;
  }
}

/*
  Guarda el nuevo estado Bluetooth recibido.

  Este metodo actualiza la variable _estado cada vez que llega un TS+xx.

  Solo genera un nuevo evento para pantalla si:
  - Es el primer estado recibido.
  - El estado ha cambiado respecto al anterior.

  Asi se evita refrescar la pantalla continuamente con el mismo estado.
*/
void BT201::guardarEstado(EstadoBt nuevoEstado, const String& eventoPantalla) {
  bool haCambiado = (!_estadoRecibido || _estado != nuevoEstado);

  _estado = nuevoEstado;
  _estadoRecibido = true;

  if (haCambiado) {
    _ultimoEvento = eventoPantalla;
    _nuevoEvento = true;
  }
}

/*
  Devuelve el texto del estado Bluetooth preparado para una pantalla de 8 caracteres.

  No envia ningun comando al modulo.
  Solo convierte el ultimo estado recibido en un texto corto para mostrarlo.

  Textos usados:
  - BT_DESCONOCIDO:                "BT ---- "
  - BT_ESPERANDO_EMPAREJAMIENTO:   "PAIR    "
  - BT_CONECTADO:                  "PAUSE   "
  - BT_REPRODUCIENDO:              "PLAY    "
  - BT_EN_LLAMADA:                 "LLAMADA "
*/
String BT201::obtenerTextoPantallaEstado() const {
  if (!_estadoRecibido) {
    return "PAIR    ";
  }

  switch (_estado) {
    case BT_ESPERANDO_EMPAREJAMIENTO:
      return "PAIR    ";

    case BT_CONECTADO:
      return "PAUSE   ";

    case BT_REPRODUCIENDO:
      return "PLAY    ";

    case BT_EN_LLAMADA:
      return "CALL    ";

    case BT_DESCONOCIDO:
    default:
      return "PAIR    ";
  }
}

/*
  Ajusta un texto para mostrarlo en una pantalla de 8 caracteres.

  Si el texto es mas largo de 8 caracteres, lo recorta.
  Si es mas corto, rellena con espacios hasta llegar a 8 caracteres.
*/
String BT201::normalizarPantalla(String texto) const {
  if (texto.length() > 8) {
    texto = texto.substring(0, 8);
  }

  while (texto.length() < 8) {
    texto += ' ';
  }

  return texto;
}
