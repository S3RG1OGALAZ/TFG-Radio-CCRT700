#ifndef GESTOR_ENTRADAS_H
#define GESTOR_ENTRADAS_H

#include <Arduino.h>

/*
  Identificadores de los botones disponibles.

  Cada valor representa un boton fisico del CCRT700.
  Estos identificadores se usan para consultar si un boton concreto
  ha sido pulsado mediante fuePulsado().
*/
enum IdBoton {
  BOTON_CINTA,
  BOTON_SOS,
  BOTON_OI,
  BOTON_ONSTAR,
  BOTON_SINTONIZADOR,
  BOTON_EQUALIZADOR,
  BOTON_CD,
  BOTON_BORRAR
};

/*
  Estructura que relaciona un boton con su posicion dentro
  de la matriz de botones.

  Cada boton se identifica mediante:
  - Una fila.
  - Una columna.
*/
struct ConfiguracionBoton {
  byte fila;
  byte columna;
};

/*
  Clase encargada de gestionar la lectura de los botones.

  Esta clase se encarga de:
  - Configurar los pines de entrada y salida.
  - Escanear la matriz de botones.
  - Aplicar antirrebote.
  - Detectar pulsaciones una sola vez por cada pulsacion real.
*/
class GestorEntradas {
public:

  /*
    Constructor del gestor de entradas.

    Inicializa la configuracion interna de los botones y sus estados.
  */
  GestorEntradas();

  /*
    Inicializa los pines necesarios para leer la matriz de botones.
  */
  void iniciar();

  /*
    Escanea los botones y actualiza su estado interno.

    Debe llamarse de forma continua desde el loop principal.
  */
  void actualizar();

  /*
    Indica si un boton ha sido pulsado desde la ultima consulta.

    Devuelve true una sola vez por cada pulsacion detectada.
    Despues de devolver true, el evento queda consumido.
  */
  bool fuePulsado(IdBoton boton);

private:

  /* Numero de filas de la matriz de botones. */
  static const byte NUM_FILAS = 6;

  /* Numero de columnas de la matriz de botones. */
  static const byte NUM_COLUMNAS = 3;

  /* Numero total de botones gestionados. */
  static const byte TOTAL_BOTONES = 8;

  /*
    Pines del Arduino Mega conectados a las filas de la matriz.
  */
  byte _pinesFila[NUM_FILAS];

  /*
    Pines del Arduino Mega conectados a las columnas de la matriz.
  */
  byte _pinesColumna[NUM_COLUMNAS];

  /*
    Mapa interno de botones.

    Relaciona cada boton logico con su fila y columna reales.
  */
  ConfiguracionBoton _configuracion[TOTAL_BOTONES];

  /*
    Estado estable actual de cada boton.

    Con INPUT_PULLUP:
    - HIGH significa no pulsado.
    - LOW significa pulsado.
  */
  bool _estado[TOTAL_BOTONES];

  /*
    Ultima lectura instantanea de cada boton.

    Se usa para detectar cambios rapidos antes de aplicar antirrebote.
  */
  bool _ultimaLectura[TOTAL_BOTONES];

  /*
    Indica si cada boton tiene una pulsacion pendiente de consumir.

    Cuando fuePulsado() devuelve true, este valor se vuelve a poner a false.
  */
  bool _eventoPulsacion[TOTAL_BOTONES];

  /*
    Guarda el instante del ultimo cambio detectado en cada boton.

    Se usa para calcular si ya ha pasado el tiempo de antirrebote.
  */
  unsigned long _ultimoRebote[TOTAL_BOTONES];

  /*
    Tiempo minimo que debe mantenerse una lectura para considerarla valida.

    Sirve para evitar dobles pulsaciones o lecturas falsas provocadas
    por el rebote mecanico de los botones.
  */
  const unsigned long _milisegundosAntirrebote = 50;
};

#endif