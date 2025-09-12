// Pines que se usan
#define swBottom    8  // Cable naranja,  pulsador de cassete en parte inferior
#define swInsert    9  // Cable amarillo, pulsador de insercion de cassette
#define swStandby  10  // Cable verde,    pulsador de color negro
#define swPlay     11  // Cable azul,     pulsador de color blanco

#define swStep1    14  // Cable gris
#define swStep2    15  // Cable blanco

#define swMotor1   16  // Cable negro,    desconocido
#define swMotor3   17  // Cable rojo,     positivo del motor del "transport disc"

int casOrden           // Contiene la nueva orden recibida por RX-TX (extraido = 0, pausa = 1, revovinandoA = 2, revovinandoB = 3, reproduciendoA = 4, reproduciendoB = 5, introducido = 6)
int casOrdenNueva      // Contiene la orden anteriormente recibida por RX-TX ()
int estadoActual       // Contiene el estado actual (extraido = 0, pausa = 1, revovinandoA = 2, revovinandoB = 3, reproduciendoA = 4, reproduciendoB = 5, introducido = 6)
boolean sentidoGiro    // Mantiene el sentido de giro cuando se pausa el cassette (true = cara A, false = cara B)

void setup () {

  Serial.begin(9600);
  
  // Secuencia de arranque de cassette

  pinMode(swBottom, INPUT_PULLUP);
  pinMode(swInsert, INPUT_PULLUP);
  pinMode(swStandby, INPUT_PULLUP);
  pinMode(swPlay, INPUT_PULLUP);

  pinMode(swStep1, OUTPUT);
  pinMode(swStep2, OUTPUT);
  digitalWrite(swStep2, LOW);

  // En el caso de que el cassette esté insertado, se coloca en pausa
  if (swInsert == 1 && swBottom == 0) { // Cassette insertado
    pausaCassette();
    casOrdenNueva = 0;
    casOrdenAnterior = 0;
  } else if (swBottom == 1 && swInsert == 1) { // Cassette a medias de la insercion
    insertaCassette();
    casOrdenNueva = 0;
    casOrdenAnterior = 0;
  }
  sentidoGiro = true; // Inicializa en la cara A
}

void loop () {

  Serial.print("\n Bottom: ");
  Serial.print(digitalRead(swBottom));

  Serial.print("\n Insert: ");
  Serial.print(digitalRead(swInsert));
  
  Serial.print("\n Standby: ");
  Serial.print(digitalRead(swStandby));
  
  Serial.print("\n Play: ");
  Serial.print(digitalRead(swPlay));

  
  if (compruebaEstadoCassette() ! casOrden) {
    
  }

  
  
  if (estado) {
    digitalWrite(swStep1, LOW);
    digitalWrite(swStep2, HIGH);
  } else {
    digitalWrite(swStep1, HIGH);
    digitalWrite(swStep2, LOW);
  }

  estado = !estado;
  delay(3000);
}

void compruebaEstadoCassette () {
    
  if (swInsert = 0) { 
    estadoActual = 0; // Coloca el estado actual a extraido
    
  } else if (swBottom == 1 && swInsert == 1) {
    estadoActual = 6; // Coloca el estado actual a introducido
    
  } else if (swStandby == 1 && swPlay == 1 && swInsert == 1) {
    estadoActual = 1; // Coloca el estado actual a pausa
    
  } else if (swStandby == 0 && swPlay == 1 && swInsert == 1) {
    if (sentidoGiro) {
      estadoActual = 2; // Coloca el estado actual a revovinando A
    } else {
      estadoActual = 3; // Coloca el estado actual a revovinando B
    }
  } else if (swStandby == 0 && swPlay == 0 && swInsert == 1) {
    if (sentidoGiro) {
      estadoActual = 4; // Coloca el estado actual a reproduciendo A
    } else {
      estadoActual = 5; // Coloca el estado actual a reproduciendo B
    }
  } 
}

void OrdenNueva(int ordenNueva) {

  // Orden de pausa/play
  if (ordenNueva == 1) { 

    // Si estaba en pausa se coloca en reproduciento
    if (casOrden == 1) { 
      if (sentidoGiro) {
        casOrden = 4; // Coloca el estado actual a reproduciendo A
      } else {
        casOrden = 5; // Coloca el estado actual a reproduciendo B
      }

    // Si esta revovinando o reproduciendo se coloca en pausa
    } else if (casOrden == 2 || casOrden == 3 || casOrden == 4 || casOrden == 5) { 
      casOrden == 1;
    }

  // Orden de cambio de sentido
  } else if (ordenNueva == 2) {
    
    if (casOrden == 4) {      // Si cara A, entonces cara B
      casOrden = 5;
      sentidoGiro = false;
    } else (casOrden == 5) {  // Si cara B, entonces cara A
      casOrden = 4
      sentidoGiro = true;
    }
    
    // Orden de avance rapido
  } else if (ordenNueva == 3) {
    
    if (sentidoGiro) {
      casOrden = 3;   // Si sentido A, entonces revovinando B (seria revovinar la otra cara por lo que es el equivalente a avanzar esta rapido)
    } else {
      casOrden = 2;   // Si sentido B, entonces revovinando A (seria revovinar la otra cara por lo que es el equivalente a avanzar esta rapido)
    }

  // Orden de retroceso rapido
  } else if (ordenNueva == 0) {

    if (sentidoGiro) {
      casOrden = 3;   // Si sentido A, entonces revovinando A
    } else {
      casOrden = 2;   // Si sentido B, entonces revovinando B
    }
    
  } else if (ordenNueva == "extrae") {
    casOrden = 6;
  }
}

void insertaCassette () {
    // Gira el "stepper motor" hasta que el cassette esta completamente insertado (bottom cambia a 0)
    while(swBottom == 1) {
      digitalWrite(swStep1, LOW);
      digitalWrite(swStep2, HIGH);
    }
    pausaCassette();
}

void pausaCassette () {
    // Gira la "transport disc" hasta la posicion de pausa (todos los switch abiertos menos el bottom)
    while(swStandby == 0 && swPlay == 0) {
      
    }
}
