#ifndef __MAIN_H__
#define __MAIN_H__

#include <avr/io.h>
#include <inttypes.h>
//#include <stdlib.h>

/* Interrupt Handling */
//#include <avr/interrupt.h>

//#include "../../_common/bithandling/0.94/bithandling.h"
#include _BITHANDLING_HEADER_
//#include _PWM_HEADER_
#include _HEARTBEAT_HEADER_
//#include _ADC_HEADER_


//The EDID Product ID will be reported as <ProductNum><ProjectVersion>
#define MY_EDID_PRODUCT_NUM 1

// No longer relevent... see makefile...
//#define HEARTBEAT    	PA3 
//#define HEARTBEATPIN 	PINA
//#define HEARTCONNECTION	LED_TIED_HIGH

//#include _SYSEX_DEBUG_HEADER_

#endif

