// Variables de la pantalla
// Pines que se usan
#define sda  7
#define scl  5
#define mrq  6
#define tid_delay 180

String texto = "Gracias Jaime";
int siguiente;
String subTexto;
int tipoScroll;

// Variables del Cassette

// Pines que se usan
#define swBottom    8  // Cable naranja,  pulsador de cassete en parte inferior
#define swInsert    9  // Cable amarillo, pulsador de insercion de cassette
#define swStandby  10  // Cable verde,    pulsador de color negro
#define swPlay     11  // Cable azul,     pulsador de color blanco
#define swCommon   12  // Cable morado,   comun a los pulsadores

#define swStep1    14  // Cable gris,     cable del "stepper motor"
#define swSetp2    15  // Cable blanco,   cable del "stepper motor"

#define swMotor1   16  // Cable negro,    desconocido
#define swMotor3   17  // Cable rojo,     positivo del motor del "transport disc"

int  casOrdenNueva     // Contiene la nueva orden recibida por RX-TX (pausa = 0, reproduciendo = 1, revovinando = 2, extraer = 3)
int  casOrdenAnterior  // Contiene la orden anteriormente recibida por RX-TX
bool casSwBottom       // Cable naranja,  pulsador de cassete en parte inferior
bool casSwInsert       // Cable amarillo, pulsador de insercion de cassette
bool casSwStandby      // Cable verde,    pulsador de color negro
bool casSwPlay         // Cable azul,     pulsador de color blanco
  
void setup () {

   delay(3000);

   // Configuracion de la pantalla
   pinMode(mrq, OUTPUT); 
   pinMode(sda, OUTPUT); 
   pinMode(scl, OUTPUT); 
   digitalWrite(mrq, HIGH);
   digitalWrite(sda, HIGH);
   digitalWrite(scl, HIGH);

   siguiente = 0;

   // Configuracion del Cassette
   tipoScroll = 1;        // 0 avance de 8 en 8; 1, avande caracter a caracter
   casOrdenAnterior = 0;  // Por defecto se enciende con el cassette en pausa
   casOdenNueva = 0;      // Por defecto se enciende con el cassette en pausa
}


void loop () {

  /*
   * Seccion de cosido encgargada de controlar el cassette
   * Estados del mismo:
   *                        Switch insercion       Switch bottom          Switch standby      Switch play  
   * Sin cinta              Close                  Open                   Open                Closed
   * Insercion Cinta        Open                   Closed                 Open                Open
   * Standby                Open                   Closed                 Open                Open
   * reproduciendose        Open                   Closed                 Closed              Closed
   */

   // Se comprueba que hay cinta, si no hay no se realiza ninguna operacion (no entra al if)
   if (swCasInsert == 0) {

     // Si la cinta se ha insertado pero no esta introducida completamente, se comienza el proceso de insercion
     if (swCasBottom == 0) {
        
        while (swCasBottom == 0) {
          // Gira el motor servo para introducir mientras no se haya introducido completamente
          casOrdenAnterior = 1; // Coloca el estado del cassette a reproducir
          casOrdenNueva = 1;
        }
        
     } else {

        if (casOrdenAnterior != casOrdenNueva) {

          casOrdenAnterior = casOrdenNueva; // Se pone la nueva orden como la anterior
        }
      
     }
      
   }
   
  /*
   * Seccion de codigo encargada del texto que se muestra en pantalla
   * */
  if (tipoScroll == 0) {

    subTexto = texto.substring(siguiente, siguiente+8);
    print(subTexto);
    
    if (subTexto.length() == 8) {
      siguiente = siguiente+8;
      
    } else {
      siguiente = 0;
    }
    delay(1500);
     
  } else {

    subTexto = texto.substring(siguiente, siguiente+8);
    print(subTexto);
    
    if (subTexto.length() == 8) {
      siguiente = siguiente+1;
      
    } else {
      siguiente = 0;
      delay(1000);
    }
    delay(150);
  }

  //print(texto);
  delay(500);
}
    

void tid_byte(byte data) {
    pinMode(scl, OUTPUT);
    for (uint8_t m = 0X80; m != 0; m >>= 1) { //2 lines from
    digitalWrite(sda, m & data);              //William Greiman's I2cMaster Library
    delayMicroseconds(10);
    digitalWrite(scl, HIGH);
    delayMicroseconds(tid_delay);
    digitalWrite(scl, LOW);
    delayMicroseconds(tid_delay);
    };
    delayMicroseconds(300);
    pinMode(sda,INPUT);     //3
    delayMicroseconds(tid_delay);
    pinMode(scl,INPUT);     //4
    delayMicroseconds(100);
    while(digitalRead(scl)==0);   //6
    delayMicroseconds(300);
    pinMode(scl,OUTPUT);
    digitalWrite(scl, LOW);   //7
    while(digitalRead(sda)==0);   //8
    pinMode(sda,OUTPUT);
}
    
void tid_data(byte data) {      //bytes of data
     byte val=data;            //this parity check code
     byte pari;                //comes from
     val = val ^ (val >> 4);   //Claas Anders "CaScAdE" Rathje http://www.mylifesucks.de/oss/c-tid/ 
     val = val ^ (val >> 2);   //from a post in the
     val = val ^ (val >> 1);   //mikrocontroller.net forum
     val &= 0x01;              //http://www.mikrocontroller.net/topic/19516
     pari = !val;              
     data = (data<<1)|pari;
     tid_byte(data);
}
     
     
void tid_address(byte address) {  //address completed with the extra 0 for write mode on the right
    tid_byte(address);
    digitalWrite(mrq, LOW); 
}


void start_tid() {
  
  int enWhile = 1;
  pinMode(sda, INPUT);
  
  digitalWrite(mrq, LOW);

  delayMicroseconds(150); //T1min = 100μs  T1máx = 15ms
  while(enWhile == 1) {
    enWhile = digitalRead(sda);
  } 
  
  delayMicroseconds(200);  //80
  digitalWrite(mrq, HIGH);

  //delayMicroseconds(100); //T3min = 100μs  T3max = 200μs
  while(enWhile == 0) { 
    enWhile = digitalRead(sda);
  }
  pinMode(sda,OUTPUT);
  
  delayMicroseconds(500); // T4min = 100μs T4max = 500μs
  digitalWrite(sda, LOW);
  delayMicroseconds(200);  //20
  digitalWrite(scl, LOW);
  delayMicroseconds(200);  //8
}
    
void stop_tid() {

  //Paso 9: Maestro establece SDA a Low  
  delayMicroseconds(250); // T1 = 100μs
  digitalWrite(sda, LOW);

  // Paso 10: Maestro establece MRQ a High  
  delayMicroseconds(800); // T2min = 100μs, T2max = 1ms
  digitalWrite(mrq, HIGH);

  // Paso 11: Maestro establece SCL a High
  delayMicroseconds(250); // T3 = 100μs
  digitalWrite(scl, HIGH);

  // Paso 12: Maestro establece SDA a High  
  delayMicroseconds(250); // T4 = 100μs   
  digitalWrite(sda, HIGH);
}

void print(String texto) {
  
  char c;
  
  start_tid();
      
  tid_address(0x94);
  tid_data(0);  //Primer simbolo
  tid_data(0);  //Segundo simbolo

  for (int i=0; i<(texto.length()); i++) {
  
     c = texto.charAt(i);
     tid_data(c);
     delayMicroseconds(100);
  }
  for (int i=0; i<(8-texto.length()); i++) {
    
    tid_data(' ');
    delayMicroseconds(100);
  }
  
  stop_tid(); 
}
