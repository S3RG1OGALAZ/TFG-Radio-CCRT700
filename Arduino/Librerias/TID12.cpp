#include "TID12.h"


TID::TID(byte sda, byte scl, byte mrq) {
	
	_sda = sda;
	_mrq = mrq;
	_scl = scl;

	_space1start = 0;
	_space1stop = 0;
	_space1timestamp = millis();
	_incr1 = 0;
	_ad1 = 0;

	_space2start = 0;
	_space2stop = 0;
	_space2timestamp = millis();
	_incr2 = 0;
	_ad2 = 0;

	_space3start = 0;
	_space3stop = 0;
	_space3timestamp = millis();
	_incr3 = 0;
	_ad3 = 0;

	
		
	_SYMBOLS1 = 0;
	_SYMBOLS2 = 0;
	_old_bars = 0;
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

void TID::space1_init(byte start, byte stop) {
	_space1start = start;
	_space1stop = stop;
}

void TID::space2_init(byte start, byte stop) {
	_space2start = start;
	_space2stop = stop;
}

void TID::space3_init(byte start, byte stop) {
	_space3start = start;
	_space3stop = stop;
}
void TID::tid_byte(byte data) {
  pinMode(_scl, OUTPUT);
   for (uint8_t m = 0X80; m != 0; m >>= 1) { //2 lines from
    digitalWrite(_sda, m & data);              //William Greiman's I2cMaster Library
   delayMicroseconds(tid_delay * 12);
    digitalWrite(_scl, HIGH);
    delayMicroseconds(tid_delay * 2);
    digitalWrite(_scl, LOW);
    delayMicroseconds(tid_delay * 2);
    };
  delayMicroseconds(tid_delay/2);
  pinMode(_sda,INPUT);     //3
  delayMicroseconds(tid_delay/2);
  pinMode(_scl,INPUT);     //4
  delayMicroseconds(tid_delay/2);
  while(digitalRead(_scl)==0);   //6
  delayMicroseconds(tid_delay/2);
  pinMode(_scl,OUTPUT);
  digitalWrite(_scl, LOW);   //7
  while(digitalRead(_sda)==0);   //8
  pinMode(_sda,OUTPUT);
  }

void TID::tid_data(byte data) {      //bytes of data
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

void TID::tid_address() {  
  tid_byte(0x94);		// <--- ADDRESS
  digitalWrite(_mrq, LOW); 
  delayMicroseconds(tid_delay/2);
  }


void TID::start_tid() {
  digitalWrite(_mrq, LOW);
  delayMicroseconds(tid_delay * 10);  //80
  digitalWrite(_mrq, HIGH);
  delayMicroseconds(tid_delay * 15);  //28
  pinMode(_sda,OUTPUT);
  pinMode(_scl,OUTPUT);
  digitalWrite(_sda, LOW);
  delayMicroseconds(tid_delay * 4);  //20
  digitalWrite(_scl, LOW);
  delayMicroseconds(tid_delay * 4);  //8
  }

void TID::stop_tid() {
  digitalWrite(_sda, LOW);
  delayMicroseconds(tid_delay * 20);  //40
  digitalWrite(_mrq, HIGH);
  delayMicroseconds(tid_delay * 10);  //20
  digitalWrite(_scl, HIGH);
  delayMicroseconds(tid_delay * 4);  //8
  digitalWrite(_sda, HIGH);
  delayMicroseconds(tid_delay * 4);  //8
  }




void TID::reset_pos(byte spaceid) {
	switch (spaceid) {
		case 1:
			_incr1 = 0;
			_ad1 = 0;
			break;
		case 2:
			_incr2 = 0;
			_ad2 = 0;
			break;
		case 3:
			_incr3 = 0;
			_ad3 = 0;
			break;
	}
	
}

void TID::clear_space(byte spaceid) {
	byte start;
	byte stop;
	switch (spaceid) {
		case 1:
			start = _space1start;
			stop = _space1stop;
			break;
		case 2:
			start = _space2start;
			stop = _space2stop;
			break;
		case 3:
			start = _space3start;
			stop = _space3stop;
			break;
	}
	for (int i = start; i<=stop; i++) _display[i] = ' ';
	cycle();
}


	

void TID::symbol_assign (byte symbo, byte RW) {
	if (symbo == 9) bitWrite(_SYMBOLS1,0,RW);
	if (symbo == 3) bitWrite(_SYMBOLS1,1,RW);
	if (symbo == 5) bitWrite(_SYMBOLS1,3,RW);
	if (symbo == 10) bitWrite(_SYMBOLS1,4,RW);
	if (symbo == 7) bitWrite(_SYMBOLS1,5,RW);
	if (symbo == 11) bitWrite(_SYMBOLS1,6,RW);

	if (symbo == 8) bitWrite(_SYMBOLS2,2,RW);
	if (symbo == 4) bitWrite(_SYMBOLS2,3,RW);
	if (symbo == 2) bitWrite(_SYMBOLS2,4,RW);
	if (symbo == 1) bitWrite(_SYMBOLS2,5,RW);
	if (symbo == 6) bitWrite(_SYMBOLS2,6,RW);
}
	


void TID::display_symbol(byte symbo) {
	symbol_assign(symbo,1);
	cycle();
}

void TID::clear_symbol(byte symbo) {
	symbol_assign(symbo,0);
	cycle();
}

void TID::clear_all_symbols() {
	for (int i=1; i <= 11; i++) {
		symbol_assign(i,0);
	};
	cycle();
}




void TID::display_message(String message, byte speed, byte spaceid) {

	byte start;
	byte stop;
	byte incr;
	byte ad;

	int upd_delay = 1000 / speed;
	unsigned long timestamp;
	switch (spaceid) {
		case 1:
			timestamp = _space1timestamp;
			start = _space1start;
			stop = _space1stop;
			incr = _incr1;
			ad = _ad1;
			break;
		case 2:
			timestamp = _space2timestamp;
			start = _space2start;
			stop = _space2stop;
			incr = _incr2;
			ad = _ad2;
			break;
		case 3:
			timestamp = _space3timestamp;
			start = _space3start;
			stop = _space3stop;
			incr = _incr3;
			ad = _ad3;
			break;
	}
	if ((millis() - timestamp) >= upd_delay) {

		for (int i=start; i<=min(stop,(start-1+message.length())) ; i++) {
			ad = i + (incr * (message.length()>(stop-start+1))) - start;
			if (ad >= message.length()) ad = ad-message.length();
			_display[i] = message.charAt(ad);
			}
		if (message.length()>(stop-start+1)) incr++;
		if (incr >= message.length()) incr=0;
		cycle();
		switch (spaceid) {
			case 1:
				_space1timestamp = millis();
				_incr1 = incr;
				_ad1 = ad;
				break;
			case 2:
				_space2timestamp = millis();
				_incr2 = incr;
				_ad2 = ad;
				break;
			case 3:
				_space3timestamp = millis();
				_incr3 = incr;
				_ad3 = ad;
				break;
		}
	}
}




	
	



void TID::cycle(){
	start_tid();
	tid_address();
	tid_data(_SYMBOLS1);
	tid_data(_SYMBOLS2);
	for (int i=0; i<8; i++) {
		tid_data(_display[i]);
	};
	stop_tid();
}

void TID::bargraph(byte level) {
	int bars = int(level/25.5)+1;
	if (bars != _old_bars) {
		
		if (bars==11) bars = 10;
		for (int i=1; i <= 10; i++) {
			symbol_assign(i,(i <= bars));
		};
		cycle();
		_old_bars = bars;
	};
}





