#ifndef __DMS_EXTERNAL_H__
#define __DMS_EXTERNAL_H__
extern volatile uint32_t dmsCount;	//deci-milliseconds

#if (!defined(DMS_FRAC_UNUSED) || !DMS_FRAC_UNUSED)
extern volatile uint8_t dmsFrac;
#endif

//These WERE specific to externalUpdate, 
// however they can also be used for dmsFrac with timer0 interrupt
// (Along with dmsFrac, below)
// Also with nonAVR code, where us is read directly...
//   these defaults are changed in init functions
//   (defaults correspond to us-reading)
// a/o 1.12: Apparently these were a remnant of pre-xyt times...
//           Though, as stated above, they are usable by dmsFrac
//           So remove them if dmsFrac is disabled!
#if (!defined(DMS_FRAC_UNUSED) || !DMS_FRAC_UNUSED)
extern uint8_t dmsNumUpdates; 
extern uint8_t dmsIncrementSize;
#endif

//Fractional part of the last-read dmsTime...
// This'll likely be handled differently in different cases
// for now it's only implemented in NON-AVR code or AVR code based on
// timer interrupt 0 (untested)
//volatile uint8_t dmsFrac = 0;


//DMS timer is updated spurradically by an externally-defined source...
extern xyt_t dmsLine;

//__inline__ prefix might be unnecessary since the prototype includes it
__inline__ \
void init_dmsExternalUpdate(uint32_t numUpdates, uint32_t incrementSize)
{
#if (!defined(DMS_FRAC_UNUSED) || !DMS_FRAC_UNUSED)

//This is the result of 1.12-1:
// which was developed with FRAC_UNUSED...
// an easy option would be to step back to 1.12
// OR replace dmsNumUpdates and dmsIncrementSize with uint32_t's.
// Alternatively (memory limited?) do some divisions here...
#error "DMS_FRAC was unused a/o the creation of this version"
#error "numUpdates and/or incrementSize > UINT8_MAX will be destroyed!"
#error "It's an easy fix, see notes in this file"

	dmsIncrementSize = incrementSize;
	//one less than the actual count since it counts from 0...
	dmsNumUpdates = numUpdates;
#endif

//#warning "xyt requires incrementSize < numUpdates!"
	xyt_init(&dmsLine, incrementSize, numUpdates);
}

//__inline__ prefix might be unnecessary since the prototype includes it
__inline__ \
void dms_update(void)
{
//	static uint8_t counter = 0;
//	static dms4day_t lastTime;

//	lastTime = dmsCount;

	dmsCount=xyt_nextOutput(&dmsLine);	
  #if (!defined(DMS_FRAC_UNUSED) || !DMS_FRAC_UNUSED)
	dmsFrac =xyt_remainder(&dmsLine);
  #endif
//	if(dmsCount != lastTime)
//		dmsFrac=0;
	//???What happens with odd fractions...
	// e.g. 2/5, would give frac=1,2 in one case, and just 1 in the next
//	else
//		dmsFrac++;
/*
	//counter++;

	// '>' Just in case update is called before init...
	//@@@ Could use an xyT here?
	//    e.g. numUpdates=4, incSize=3
	//    would work better with dmsGetFrac...
	if(counter >= dmsNumUpdates)
	{
		counter = 0;
		dmsCount+=dmsIncrementSize;
	}
	else
		counter++;

	dmsFrac = counter;
*/
}

__inline__ \
dms4day_t dmsGetTime(void)
{
	return dmsCount;
}

#else //This has aleady been included...
 #warning "really?"
#endif

