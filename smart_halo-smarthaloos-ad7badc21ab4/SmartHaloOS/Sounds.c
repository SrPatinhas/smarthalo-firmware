#include "Sounds.h"

static Peripheral_Piezo_API* _myPiezo;

int Sounds_Register_Piezo_Peripheral(Peripheral_Piezo_API* piezo) {
	_myPiezo = piezo;
	return 1;
}

int Sound_play_alarm_sound() {
	/*
		In this case, the alarm sound is actually held in the operating system.  Whether
			or not we want this to ACTUALLY be the case in the final product is not
			yet decided, the intent of this example is to show how we might take
			commonly-used "app" functionality and put it into the OS for convenience/portability.
	*/
	for(int x=0; x<10; x++) {
		_myPiezo->play(100000,1,100);
		_myPiezo->play(50000,1,100);
	}

	return 1;
}