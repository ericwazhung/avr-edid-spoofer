//Auto-generated by makefile

#ifndef __PROJINFO_H__
#define __PROJINFO_H__
#include <inttypes.h>

#if (!defined(PROJINFO_SHORT) || !PROJINFO_SHORT)
 uint8_t __attribute__ ((progmem)) \
   header0[] = " /Users/meh/_avrProjects/LCDreIDer/51-lowerRefresh ";
 uint8_t __attribute__ ((progmem)) \
   header1[] = " Mon Dec 30 19:22:35 PST 2013 ";
 uint8_t __attribute__ ((progmem)) \
   headerOpt[] = " WDT_DIS=TRUE ";
#else //projInfo Shortened
 uint8_t __attribute__ ((progmem)) \
   header[] = "LCDreIDer51 2013-12-30 19:22:35";
#endif

//For internal use...
//Currently only usable in main.c
#define PROJ_VER 51
#define COMPILE_YEAR 2013

#endif

