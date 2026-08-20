#ifndef BT201_MODULE_H
#define BT201_MODULE_H

#include <Arduino.h>

/*
  Clase encargada de gestionar el modulo Bluetooth BT201.

  Esta clase encapsula:
  - La comunicacion serie con el modulo.
  - El envio de comandos AT.
  - La lectura y procesado de respuestas/eventos.
  - El estado actual del Bluetooth.

  Nota importante:
  En este proyecto no se consulta el estado con AT+TS de forma automatica.
  Se ha comprobado que, en este modulo/firmware, enviar AT+TS mientras el
  modulo esta en TS+00 puede provocar reinicios o re-inicializaciones.

  En su lugar, la clase guarda el ultimo estado recibido automaticamente
  desde el modulo cuando llega una linea TS+xx.
*/
class BT201 {
public:

  /*
    Estados posibles del modulo Bluetooth.

    Los valores coinciden con los codigos enviados por el modulo:
    - TS+00: esperando emparejamiento
    - TS+01: conectado
    - TS+02: reproduciendo audio
    - TS+03: llamada activa
  */
  enum EstadoBt {
    BT_DESCONOCIDO = -1,
    BT_ESPERANDO_EMPAREJAMIENTO = 0,   // TS+00
    BT_CONECTADO = 1,                  // TS+01
    BT_REPRODUCIENDO = 2,              // TS+02
    BT_EN_LLAMADA = 3                  // TS+03
  };

  /*
    Constructor de la clase.

    Recibe una referencia al puerto serie hardware que se utilizara
    para comunicarse con el modulo BT201.
  */
  explicit BT201(HardwareSerial& serial);

  /*
    Inicializa la comunicacion serie con el modulo Bluetooth.

    Por defecto usa 115200 baudios, que suele ser la velocidad habitual
    de comunicacion del BT201.
  */
  void iniciar(unsigned long baudios = 115200);

  /*
    Debe llamarse de forma periodica desde el loop principal.

    Se encarga de leer los datos recibidos por el puerto serie,
    formar lineas completas y procesarlas cuando corresponde.

    Este metodo no envia consultas automaticas al modulo. Solo lee lo que
    el BT201 envie por si mismo.
  */
  void actualizar();

  // Accion

  /*
    Envia al modulo el comando para alternar entre reproduccion y pausa.
  */
  void reproducirPausar();

  // Eventos

  /*
    Indica si se ha recibido un evento nuevo desde el modulo.
    Devuelve true si hay un evento pendiente de consumir.
  */
  bool hayNuevoEvento() const;

  /*
    Devuelve el ultimo evento recibido y lo marca como consumido.
    Una vez llamado este metodo, hayNuevoEvento() volvera a devolver false
    hasta que se reciba un nuevo evento.
  */
  String consumirUltimoEvento();

  // Estado

  /*
    Devuelve el ultimo estado conocido del modulo Bluetooth.

    No pregunta nada al modulo.
    Solo devuelve el ultimo estado guardado al recibir una linea TS+xx.
  */
  EstadoBt obtenerEstado() const;

  /*
    Indica si ya se ha recibido al menos un estado real del modulo.

    Sirve para diferenciar entre:
    - BT_DESCONOCIDO porque aun no se ha recibido ningun TS+xx.
    - Un estado real recibido anteriormente.
  */
  bool estadoRecibido() const;

  /*
    Devuelve el ultimo estado conocido como texto.

    No pregunta nada al modulo.
    Solo convierte la variable interna _estado a un texto legible.
  */
  String obtenerTextoEstado() const;

  /*
    Devuelve el texto de estado preparado para mostrar en pantalla.

    No pregunta nada al modulo BT201.
    Solo usa el ultimo estado Bluetooth guardado anteriormente.
  */
  String obtenerTextoPantallaEstado() const;

private:

  /*
    Referencia al puerto serie usado para comunicarse con el BT201.
    Se mantiene como referencia para no copiar el objeto HardwareSerial.
  */
  HardwareSerial& _serial;

  /*
    Estado actual conocido del modulo Bluetooth.

    Se actualiza solo cuando llega una respuesta TS+xx desde el modulo.
  */
  EstadoBt _estado;

  /*
    Indica si ya se ha recibido al menos un estado TS+xx.

    Al arrancar es false porque todavia no se sabe en que estado real
    esta el modulo.
  */
  bool _estadoRecibido;

  /*
    Buffer temporal donde se va almacenando la linea recibida por serie
    hasta encontrar un salto de linea o fin de mensaje.
  */
  String _lineaRecibida;

  /*
    Ultimo evento recibido desde el modulo Bluetooth.

    Se usa para mostrar informacion corta en pantalla.
  */
  String _ultimoEvento;

  /*
    Indica si existe un evento nuevo pendiente de ser leido.
  */
  bool _nuevoEvento;

  /*
    Envia un comando al modulo Bluetooth a traves del puerto serie.
  */
  void enviarComando(const String& comando);

  /*
    Procesa una linea completa recibida desde el modulo.

    Aqui se interpreta el contenido recibido, se actualiza el estado
    y se registran eventos cuando proceda.
  */
  void procesarLinea(String linea);

  /*
    Guarda un estado Bluetooth recibido desde una linea TS+xx.

    Tambien genera un evento para pantalla si el estado es nuevo o ha cambiado.
  */
  void guardarEstado(EstadoBt nuevoEstado, const String& eventoPantalla);

  /*
    Normaliza un texto para poder mostrarlo correctamente en pantalla.

    Puede usarse para limpiar caracteres no deseados, adaptar el texto
    o evitar problemas al mostrar informacion recibida desde el modulo.
  */
  String normalizarPantalla(String texto) const;
};

#endif
