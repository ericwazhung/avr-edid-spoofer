# Modified for reallyCommon2.mk via LCDdirectLVDS-61

# This gets modified for compilation to _BUILD/
# if needed after common.mk is included Reference ORIGINALTARGET...
TARGET = LCDreIDer

MCU = attiny45

FCPU = 8000000UL

MY_SRC = main.c

# Use the short projInfo to save code-space... 
CFLAGS += -D'PROJINFO_SHORT=TRUE'

# Override the default optimization level:
#OPT = 3

#Necessary for printing floats via sprintf... (gotoRamped, speed)
#LDFLAGS += -Wl,-u,vfprintf -lprintf_flt -lm
# Add lm if used... (also necessary for floats?)
#LDFLAGS += -lm
#endif


# Probably best not to enable the WDT while testing/debugging!
WDT_DISABLE = TRUE
	PROJ_OPT_HDR += WDT_DIS=$(WDT_DISABLE)


# I have configured OSCCAL for this chip...
# When using another chip I might have trouble
# This will warn whenever burning fuses...
#OSCCAL_SET = TRUE





#Location of the _common directory, relative to here...
# this should NOT be an absolute path...
# COMREL is used for compiling common-code locally...
COMREL = ../../..
COMDIR = $(COMREL)/_commonCode



################# SHOULD NOT CHANGE THIS BLOCK... FROM HERE ############## 
#                                                                        #
# This stuff has to be done early-on (e.g. before other makefiles are    #
#   included..                                                           #
#                                                                        #
#                                                                        #
# If this is defined, we can use 'make copyCommon'                       #
#   to copy all used commonCode to this subdirectory                     #
#   We can also use 'make LOCAL=TRUE ...' to build from that code,       #
#     rather than that in _commonCode                                    #
LOCAL_COM_DIR = _commonCode_localized
#                                                                        #
#                                                                        #
# If use_LocalCommonCode.mk exists and contains "LOCAL=1"                #
# then code will be compiled from the LOCAL_COM_DIR                      #
# This could be slightly more sophisticated, but I want it to be         #
#  recognizeable in the main directory...                                #
# ONLY ONE of these two files (or neither) will exist, unless fiddled with 
SHARED_MK = __use_Shared_CommonCode.mk
LOCAL_MK = __use_Local_CommonCode.mk
#                                                                        #
-include $(SHARED_MK)
-include $(LOCAL_MK)
#                                                                        #
#                                                                        #
#                                                                        #
#Location of the _common directory, relative to here...                  #
# this should NOT be an absolute path...                                 #
# COMREL is used for compiling common-code into _BUILD...                #
# These are overriden if we're using the local copy                      #
# OVERRIDE the main one...                                               #
ifeq ($(LOCAL), 1)
COMREL = ./
COMDIR = $(LOCAL_COM_DIR)
endif
#                                                                        #
################# TO HERE ################################################




# Common "Libraries" to be included
#  haven't yet figured out how to make this less-ugly...
#DON'T FORGET to change #includes...
# (should no longer be necessary... e.g. "#include _HEARTBEAT_HEADER_")


VER_SINETABLE = 0.99
# This is now default when SINE_TYPE=8
CFLAGS += -D'SINE_RAW8=TRUE'
SINETABLE_LIB = $(COMDIR)/sineTable/$(VER_SINETABLE)/sineTable
# This only disables sineRaw(), not sineRaw8()
# This is now default when SINE_TYPE=8
CFLAGS += -D'SINE_DISABLE_SINERAW=TRUE'
CFLAGS += -D'SINE_DISABLEDOUBLESCALE=TRUE'
SINE_TYPE = 8
#16
#CFLAGS += -D'SINE_TABLE_ONLY=TRUE'
include $(SINETABLE_LIB).mk

# There're lots of options like this for code-size
# Also INLINE_ONLY's see the specific commonCode...
#CFLAGS += -D'TIMER_INIT_UNUSED=TRUE'


VER_HFMODULATION = 1.00
HFMODULATION_LIB = $(COMDIR)/hfModulation/$(VER_HFMODULATION)/hfModulation
include #(HFMODULATION_LIB).mk


VER_HEARTBEAT = 1.21
HEARTBEAT_LIB = $(COMDIR)/heartbeat/$(VER_HEARTBEAT)/heartbeat
# Don't include HEART source-code, for size tests...
#HEART_REMOVED = TRUE
#HFMODULATION_REMOVED = TRUE
HEART_DMS = TRUE
DMS_EXTERNALUPDATE = FALSE
#override heartBeat's preferred 4s choice...
CFLAGS += -D'_WDTO_USER_=WDTO_1S'
#CFLAGS += -D'HEART_INPUTPOLLING_UNUSED=TRUE'
#CFLAGS += -D'HEART_GETRATE_UNUSED=TRUE'
CFLAGS += -D'HEARTPIN_HARDCODED=TRUE'
# Heartbeat is on MISO:
#  since the ATtiny can drive up to 40mA it shouldn't affect programming
CFLAGS += -D'HEART_PINNUM=PB1'
#CFLAGS += -D'HEART_PINNUM=PB3'
CFLAGS += -D'HEART_PINPORT=PORTB'
CFLAGS += -D'HEART_LEDCONNECTION=LED_TIED_HIGH'
include $(HEARTBEAT_LIB).mk



# HEADERS... these are LIBRARY HEADERS which do NOT HAVE SOURCE CODE
# These are added to COM_HEADERS after...
# These are necessary only for make tarball... 
#   would be nice to remove this...
# NOTE: These CAN BE MULTIPLY-DEFINED! Because newer headers almost always 
#    include older headers' definitions, as well as new ones
#   i.e. bithandling...
#  the only way to track all this, for sure, is to hunt 'em all down
#  (or try make tarball and see what's missing after the compilation)
VER_BITHANDLING = 0.94
BITHANDLING_HDR = $(COMDIR)/bithandling/$(VER_BITHANDLING)/
COM_HEADERS += $(BITHANDLING_HDR)

# This is only so #include _BITHANDLING_HEADER_ can be used in .c and .h files...
CFLAGS += -D'_BITHANDLING_HEADER_="$(BITHANDLING_HDR)/bithandling.h"'

VER_ERRORHANDLING = 0.99
ERRORHANDLING_HDR = $(COMDIR)/errorHandling/$(VER_ERRORHANDLING)/
COM_HEADERS += $(ERRORHANDLING_HDR)

include $(COMDIR)/_make/reallyCommon2.mk


