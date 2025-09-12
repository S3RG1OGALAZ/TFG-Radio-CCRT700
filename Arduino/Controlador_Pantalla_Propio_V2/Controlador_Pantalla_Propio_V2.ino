/*
Most of the i2c commands here presented were copied and then modified
the original source is the  Arduino I2cMaster Library
written by William Greiman

the parity check code is by Claas Anders "CaScAdE" Rathje 

pins for sda,scl,mrq are connected directly to their correspondant
on the TID connector, no extra circuitry needed

*/

// pin numbers
#define sda  7
#define scl  5
#define mrq  6
#define tid_delay 25

int numIntentosEnvioByte;

void setup () {

 Serial.begin(115200);
  
 delay(3000);
 
 digitalWrite(mrq, LOW);
 digitalWrite(sda, LOW);
 digitalWrite(scl, LOW);
 pinMode(mrq, OUTPUT); 
 pinMode(sda, OUTPUT); 
 pinMode(scl, OUTPUT);
 delayMicroseconds(700); // T2min = 500μs  T2max = 1ms
 digitalWrite(mrq, HIGH);
 digitalWrite(sda, HIGH);
 digitalWrite(scl, HIGH);
 delayMicroseconds(700); // T2min = 500μs  T2max = 1ms
 digitalWrite(sda, LOW);
 delayMicroseconds(700); // T2min = 500μs  T2max = 1ms
 digitalWrite(sda, HIGH);
 delayMicroseconds(700); // T2min = 500μs  T2max = 1ms
 digitalWrite(scl, LOW);
 delayMicroseconds(700); // T2min = 500μs  T2max = 1ms
 digitalWrite(scl, HIGH);
 delayMicroseconds(700); // T2min = 500μs  T2max = 1ms
 digitalWrite(mrq, LOW);
 delayMicroseconds(700); // T2min = 500μs  T2max = 1ms
 digitalWrite(mrq, HIGH);
 delayMicroseconds(2000); // T2min = 500μs  T2max = 1ms



  numIntentosEnvioByte = 1;
  char c;
  start_tid();

  String texto = ("Te AMO  ");
  
  tid_address();
  
  //tid_data(0);  //first symbols byte
  //tid_data(0);  //second  symbols byte
  
  if (numIntentosEnvioByte == 1) {  // byte 1
    tid_data(0);
    delayMicroseconds(100);
  } else {
    stop_tid();
  }
  if (numIntentosEnvioByte == 1) {  // Byte 2
    tid_data(0);
    delayMicroseconds(100);
  } else {
    stop_tid();
  }

  for (int i=0; i<8; i++) {         // Texto
    c = texto.charAt(i);
    if (numIntentosEnvioByte == 1) {
      tid_data(c);
      delayMicroseconds(100);
    } else {
      stop_tid();
    }
  }
  
  //tid_data('@');  
  //tid_data('$');
  //tid_data('?');
  //tid_data('i');
  //tid_data('o');
  //tid_data('S');
  //tid_data('U');    //8th letter
  
  stop_tid();

  Serial.print("Termino");
  
  delay(100);
}

void loop () {

}

void tid_byte(byte data) {
  
  //pinMode(scl, OUTPUT);
  
  for (uint8_t m = 0X80; m != 0; m >>= 1) { //2 lines from
    if((m & data) != 0) {
      digitalWrite(sda, HIGH);
    } else {
      digitalWrite(sda, LOW);
    }
    //digitalWrite(sda, m & data);              //William Greiman's I2cMaster Library
    
    delayMicroseconds(100);
    digitalWrite(scl, HIGH);
    delayMicroseconds(500);
    digitalWrite(scl, LOW);
    delayMicroseconds(50);
  };
  
  delayMicroseconds(tid_delay/2);
  pinMode(sda,INPUT);     //3
  delayMicroseconds(tid_delay/2);
  pinMode(scl,INPUT);     //4
  delayMicroseconds(tid_delay/2);
  while(digitalRead(scl)==0);   //6
  delayMicroseconds(tid_delay/2);
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

void tid_address() {  
  tid_byte(0x94);
  digitalWrite(mrq, LOW); 
  delayMicroseconds(tid_delay*5);
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
  //delayMicroseconds(tid_delay * 15);  //28

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
  
/*void stop_tid() {
  digitalWrite(sda, LOW);
  delayMicroseconds(500);  //40
  digitalWrite(mrq, HIGH);
  delayMicroseconds(250);  //20
  digitalWrite(scl, HIGH);
  delayMicroseconds(100);  //8
  digitalWrite(sda, HIGH);
  delayMicroseconds(100);  //8
}*/

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
