#include "peripheral.h"

/*
	On private functions:
		- essentially any function we put in this file is effectively private

	There are 2 approaches to private variables (aka data)
		- if we assume we only ever have one instance of a peripheral, 
			we can use static variables in this file as "private"
		- we could have another header file "Peripheral-private.h", which defines private data 
			and can only be included here.  If we do that, we'd need to attach it to the "instance" somehow
*/

// private function that we expose in the "constructor" below
static char* _get_name() {
	return "Peripheral";
}

static char* _get_part_number() {
	return "partnum";
}

// this function is exposed in peripheral.h and is therefore public
int Peripheral_init(Peripheral* _perf) {
	//set the function pointers to our private functions
	_perf->Peripheral_getName = &_get_name;
	_perf->Peripheral_getPartNumber = &_get_part_number;
	return 0;
}

