#include "TID10.h"


TID10::TID10(byte sda, byte scl, byte mrq) {
	
	_sda = sda;
	_mrq = mrq;
	_scl = scl;
	_timestamp = millis();
	_incr = 0;
	_ad = 0;
	_SYMBOLS1 = 0;
	_SYMBOLS2 = 0;
	for (int i=0; i<8; i++) {
		_display[i] = ' ';
	};


	
	pinMode(_mrq, OUTPUT); 
	pinMode(_sda, OUTPUT); 
	pinMode(_scl, OUTPUT); 
	digitalWrite(_mrq, HIGH);
	digitalWrite(_sda, HIGH);
	digitalWrite(_scl, HIGH);
}


void TID10::tid_byte(byte data) {
  pinMode(_scl, OUTPUT);
    for (uint8_t m = 0X80; m != 0; m >>= 1) { //2 lines from
    digitalWrite(_sda, m & data);              //William Greiman's I2cMaster Library
    delayMicroseconds(10);
    digitalWrite(_scl, HIGH);
    delayMicroseconds(tid_delay);
    digitalWrite(_scl, LOW);
    delayMicroseconds(tid_delay);
    };
  delayMicroseconds(300);
  pinMode(_sda,INPUT);     //3
  delayMicroseconds(tid_delay);
  pinMode(_scl,INPUT);     //4
  delayMicroseconds(100);
  while(digitalRead(_scl)==0);   //6
  delayMicroseconds(300);
  pinMode(_scl,OUTPUT);
  digitalWrite(_scl, LOW);   //7
  while(digitalRead(_sda)==0);   //8
  pinMode(_sda,OUTPUT);
  }

void TID10::tid_data(byte data) {      //bytes of data
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

void TID10::tid_address() {  
  tid_byte(0x4a);		//the address for the 8 char display is embedded here, for the 10 char you have to use 0x9A
  digitalWrite(_mrq, LOW); 
  }

void TID10::start_tid() {
  digitalWrite(_mrq, LOW);
  delayMicroseconds(2000);
  digitalWrite(_mrq, HIGH);
  delayMicroseconds(700);
  pinMode(_sda,OUTPUT);
  pinMode(_scl,OUTPUT);
  digitalWrite(_sda, LOW);
  delayMicroseconds(500);
  digitalWrite(_scl, LOW);
  delayMicroseconds(200);
  }

void TID10::stop_tid() {
  digitalWrite(_sda, LOW);
  delayMicroseconds(1000);
  digitalWrite(_mrq, HIGH);
  delayMicroseconds(500);
  digitalWrite(_scl, HIGH);
  delayMicroseconds(200);
  digitalWrite(_sda, HIGH);
  delayMicroseconds(200);
  }

void TID10::display_message(String message, byte roll_speed) {
	

	int upd_delay = 1000 / roll_speed;
	char display[8];

	for (int i=0; i<8; i++) {
		_display[i]=message.charAt(i);
	};

	if ((millis() - _timestamp) > upd_delay) {
		if (message.length()>8) {
			
			for (int i=0; i<8; i++) {
				_ad = i + _incr;
				if (_ad >= message.length()) {
					_ad = _ad-message.length();
				};
				_display[i] = message.charAt(_ad);
			};
			_incr++;
			if (_incr >= message.length()) {
				_incr=0;
			};
		};



		start_tid();
		tid_address();
		tid_data(_SYMBOLS1);
		tid_data(_SYMBOLS2);
		for (int i=0; i<8; i++) {
			tid_data(_display[i]);
		};
		stop_tid();
		_timestamp = millis();
	};

}

void TID10::clear_message() {

	start_tid();
	tid_address();
	tid_data(_SYMBOLS1);
	tid_data(_SYMBOLS2);
	
	for (int i=0; i<8; i++) {
		tid_data(0);
	};
	stop_tid();
}

void TID10::display_symbol(byte symbo) {

	if ((symbo == 1) && !bitRead(_SYMBOLS1,0)) bitSet(_SYMBOLS1,0);
	if ((symbo == 2) && !bitRead(_SYMBOLS1,1)) bitSet(_SYMBOLS1,1);
	if ((symbo == 3) && !bitRead(_SYMBOLS1,3)) bitSet(_SYMBOLS1,3);
	if ((symbo == 4) && !bitRead(_SYMBOLS1,4)) bitSet(_SYMBOLS1,4);
	if ((symbo == 5) && !bitRead(_SYMBOLS1,5)) bitSet(_SYMBOLS1,5);
	if ((symbo == 6) && !bitRead(_SYMBOLS1,6)) bitSet(_SYMBOLS1,6);

	if ((symbo == 7) && !bitRead(_SYMBOLS2,2)) bitSet(_SYMBOLS2,2);
	if ((symbo == 8) && !bitRead(_SYMBOLS2,3)) bitSet(_SYMBOLS2,3);
	if ((symbo == 9) && !bitRead(_SYMBOLS2,4)) bitSet(_SYMBOLS2,4);
	if ((symbo == 10) && !bitRead(_SYMBOLS2,5)) bitSet(_SYMBOLS2,5);
	if ((symbo == 11) && !bitRead(_SYMBOLS2,6)) bitSet(_SYMBOLS2,6);

	start_tid();
	tid_address();
	tid_data(_SYMBOLS1);
	tid_data(_SYMBOLS2);
	for (int i=0; i<8; i++) {
		tid_data(_display[i]);
	};
	stop_tid();

}

void TID10::clear_symbol(byte symbo) {

	if ((symbo == 1) && bitRead(_SYMBOLS1,0)) bitClear(_SYMBOLS1,0);
	if ((symbo == 2) && bitRead(_SYMBOLS1,1)) bitClear(_SYMBOLS1,1);
	if ((symbo == 3) && bitRead(_SYMBOLS1,3)) bitClear(_SYMBOLS1,3);
	if ((symbo == 4) && bitRead(_SYMBOLS1,4)) bitClear(_SYMBOLS1,4);
	if ((symbo == 5) && bitRead(_SYMBOLS1,5)) bitClear(_SYMBOLS1,5);
	if ((symbo == 6) && bitRead(_SYMBOLS1,6)) bitClear(_SYMBOLS1,6);

	if ((symbo == 7) && bitRead(_SYMBOLS2,2)) bitClear(_SYMBOLS2,2);
	if ((symbo == 8) && bitRead(_SYMBOLS2,3)) bitClear(_SYMBOLS2,3);
	if ((symbo == 9) && bitRead(_SYMBOLS2,4)) bitClear(_SYMBOLS2,4);
	if ((symbo == 10) && bitRead(_SYMBOLS2,5)) bitClear(_SYMBOLS2,5);
	if ((symbo == 11) && bitRead(_SYMBOLS2,6)) bitClear(_SYMBOLS2,6);
	start_tid();
	tid_address();
	tid_data(_SYMBOLS1);
	tid_data(_SYMBOLS2);
	for (int i=0; i<8; i++) {
		tid_data(_display[i]);
	};
	stop_tid();

}

void TID10::clear_all_symbols() {
	start_tid();
	tid_address();
	tid_data(0);
	tid_data(0);
	for (int i=0; i<8; i++) {
		tid_data(_display[i]);
	};
	stop_tid();
}



