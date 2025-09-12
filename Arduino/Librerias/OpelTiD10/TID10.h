#ifndef TID10_H
#define TID10_H
 
#include <Arduino.h> 

#define tid_delay 180

class TID10 {
public:
        TID10(byte sda, byte scl, byte mrq);

		void display_message(String message, byte roll_speed); // roll_speed = 1 -> min roll_speed = 255 -> max

		void clear_message();
		void display_symbol(byte symbo);
		void clear_symbol(byte symbo);
		void clear_all_symbols();


		/*
		message is shown and updated according to roll_speed
		if message it's longer than 8 char will roll
		
		symbols and their correspondant value:

		[]		symbo = 1
		AS		symbo = 2
		stereo	symbo = 3
		TP		symbo = 4
		RDS		symbo = 5
		.		symbo = 6
		CPS		symbo = 7
		Cr		symbo = 8
		B		symbo = 9
		C		symbo = 10
		CD		symbo = 11


		*/

		
		
		
private:
		void tid_byte(byte data);
		void tid_data(byte data);
		void tid_address();
		void start_tid();
		void stop_tid();
//private:
		byte _sda;
		byte _scl;
		byte _mrq;
		unsigned long _timestamp;
		byte _incr;
		byte _ad;
		byte _SYMBOLS1;
		byte _SYMBOLS2;
		char _display[9];
};
#endif
