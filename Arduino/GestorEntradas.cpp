#include "GestorEntradas.h"

/*
  Constructor del gestor de entradas.

  Aqui se configuran:
  - Los pines fisicos conectados al CCRT700.
  - La relacion interna entre cada boton y su fila/columna.
  - El estado inicial de cada boton.
*/
GestorEntradas::GestorEntradas() {
  // Pines de los botones del CCRT700.
  // Orden de indices: 0=1, 1=2, 2=3, 3=6, 4=7, 5=8.
  _pinesFila[0] = 52; // CCRT700 pin 1
  _pinesFila[1] = 50; // CCRT700 pin 2
  _pinesFila[2] = 48; // CCRT700 pin 3
  _pinesFila[3] = 46; // CCRT700 pin 6
  _pinesFila[4] = 44; // CCRT700 pin 7
  _pinesFila[5] = 42; // CCRT700 pin 8

  // Pines de columna de los botones del CCRT700.
  // Orden de indices: 0=4, 1=5, 2=12.
  _pinesColumna[0] = 53; // CCRT700 pin 4
  _pinesColumna[1] = 51; // CCRT700 pin 5
  _pinesColumna[2] = 49; // CCRT700 pin 12

  /*
    Mapa interno de botones.

    Cada boton se asocia a una fila y una columna.
    La pulsacion se detecta activando una columna y leyendo la fila
    correspondiente.
  */
  _configuracion[BOTON_CINTA]         = {3, 2}; // Pines reales: (6,12)
  _configuracion[BOTON_SOS]           = {4, 2}; // Pines reales: (7,12)
  _configuracion[BOTON_OI]            = {5, 2}; // Pines reales: (8,12)

  _configuracion[BOTON_ONSTAR]        = {0, 0}; // Pines reales: (1,4)
  _configuracion[BOTON_SINTONIZADOR]  = {1, 0}; // Pines reales: (2,4)
  _configuracion[BOTON_EQUALIZADOR]   = {2, 0}; // Pines reales: (3,4)

  _configuracion[BOTON_CD]            = {0, 1}; // Pines reales: (1,5)
  _configuracion[BOTON_BORRAR]        = {1, 1}; // Pines reales: (2,5)

  /*
    Inicializa el estado interno de todos los botones.

    HIGH significa que el boton no esta pulsado, ya que se usan entradas
    con resistencia pull-up.
  */
  for (int indice = 0; indice < TOTAL_BOTONES; indice++) {
    _estado[indice] = HIGH;
    _ultimaLectura[indice] = HIGH;
    _eventoPulsacion[indice] = false;
    _ultimoRebote[indice] = 0;
  }
}

/*
  Inicializa los pines usados para leer la matriz de botones.

  Las columnas se configuran como salidas.
  Las filas se configuran como entradas con pull-up interno.
*/
void GestorEntradas::iniciar() {
  // Asigna las columnas como salidas y las deja desactivadas.
  for (int indiceColumna = 0; indiceColumna < NUM_COLUMNAS; indiceColumna++) {
    pinMode(_pinesColumna[indiceColumna], OUTPUT);
    digitalWrite(_pinesColumna[indiceColumna], HIGH); // Columna desactivada.
  }

  // Asigna las filas como entradas con resistencia pull-up interna.
  for (int indiceFila = 0; indiceFila < NUM_FILAS; indiceFila++) {
    pinMode(_pinesFila[indiceFila], INPUT_PULLUP); // Fila con pull-up.
  }
}

/*
  Actualiza el estado de los botones.

  Este metodo debe llamarse de forma continua desde el loop principal.
  Recorre las columnas una a una, activa cada columna y lee las filas
  asociadas a los botones de esa columna.
*/
void GestorEntradas::actualizar() {
  unsigned long ahora = millis();

  // Bucle que activa cada columna individualmente.
  for (int indiceColumna = 0; indiceColumna < NUM_COLUMNAS; indiceColumna++) {
    digitalWrite(_pinesColumna[indiceColumna], LOW); // Activa la columna actual.

    // Bucle que comprueba cada boton individualmente.
    for (int indiceBoton = 0; indiceBoton < TOTAL_BOTONES; indiceBoton++) {
      
      // Si el boton actual no pertenece a la columna activada,
      // no se comprueba en esta pasada.
      if (_configuracion[indiceBoton].columna != indiceColumna) {
        continue;
      }

      // Lee la fila asociada al boton actual.
      byte indiceFila = _configuracion[indiceBoton].fila;
      bool lectura = digitalRead(_pinesFila[indiceFila]);

      // Si cambia la lectura instantanea, se reinicia el temporizador
      // de antirrebote.
      if (lectura != _ultimaLectura[indiceBoton]) {
        _ultimoRebote[indiceBoton] = ahora;
        _ultimaLectura[indiceBoton] = lectura;
      }

      // Si ya ha pasado el tiempo de antirrebote, se acepta la lectura
      // como estado estable del boton.
      if ((ahora - _ultimoRebote[indiceBoton]) >= _milisegundosAntirrebote) {
        if (_estado[indiceBoton] != lectura) {
          _estado[indiceBoton] = lectura;

          // Detecta el flanco de pulsacion.
          // Con INPUT_PULLUP, LOW significa boton pulsado.
          // Solo se marca evento cuando el boton pasa a pulsado.
          if (_estado[indiceBoton] == LOW) {
            _eventoPulsacion[indiceBoton] = true;
          }
        }
      }
    }

    digitalWrite(_pinesColumna[indiceColumna], HIGH); // Desactiva la columna actual.
  }
}

/*
  Indica si un boton ha sido pulsado desde la ultima consulta.

  Si habia una pulsacion pendiente, la consume y devuelve true.
  Si no habia pulsacion pendiente, devuelve false.
*/
bool GestorEntradas::fuePulsado(IdBoton boton) {
  if (_eventoPulsacion[boton]) {
    _eventoPulsacion[boton] = false;
    return true;
  }

  return false;
}