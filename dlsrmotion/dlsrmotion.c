#include <avr/io.h>
#include <util/delay.h>

#define PIN_FOCUS 	(1<<0)
#define PIN_SHUTTER	(1<<1)
#define PIN_PIR		(1<<4)
#define PIN_LED		(1<<3)

void trigger_focus() {
	PORTB &= ~PIN_FOCUS;	
	_delay_ms(300);
	PORTB |= PIN_FOCUS;
	_delay_ms(300);
}

void trigger_shutter() {
	PORTB &= ~PIN_SHUTTER;
	_delay_ms(100);
	PORTB |= PIN_SHUTTER;
	_delay_ms(1);
}

int main(void) {
	// 0, 1, 3 Output
	DDRB = 0b00001011;
	PORTB = 0b00000011;

	int i;
	
	// der PIR braucht seine Zeit zum Kalibrieren, deshalb
	// startup delay, durch blinken signalisieren
	for(i=0;i<25;i++) {
		PORTB |= PIN_LED;
		_delay_ms(200);
		PORTB &= ~PIN_LED;
		_delay_ms(200);
	}
	

	for (;;) {
		// PIR meldet Bewegung
		if (PINB & PIN_PIR) {
			PORTB |= PIN_LED;
			// Kamera fokussieren
			trigger_focus();
			//5 Bilder machen, je 2 Sek eines
			for (i=0; i<5; i++) {
				PORTB &= ~PIN_LED;
				trigger_shutter();
				PORTB |= PIN_LED;
				_delay_ms(2000);
			}
			PORTB &= ~PIN_LED;
		} 
		PORTB &= ~PIN_LED;
		
	}
	
}
