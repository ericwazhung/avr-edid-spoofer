# dmsTimer depends on:
#  timer0 library if DMS_EXTERNALUPDATE is false AND this is an AVR
 

# The following variables are expected to be defined in the calling makefile(s)
# DMSTIMER_LIB	- Path to dmsTimer.mk/c including filename, excluding extension
# COMDIR
# DMS_EXTERNALUPDATE	(TRUE or FALSE...)
#   FALSE when Timer0 can be used exclusively for dmsTimer
#   TRUE when Timer0 is used for other purposes... 
#        in which case, call dms_update manually (T0 overflow can work)...


# optional definitions (in gcc -D listing) TRUE or FALSE
# __TODO_WARN__ - warn about TODOs
# __UNKN_WARN__ - warn about unknowns/untesteds
# __OPTIM_WARN__ - warn of potential optimizations
# __ERR_WARN__   - warn of potential errors (divide-by-zero, etc...)


#only include dms_timer once...
ifneq ($(DMSTIMER_INCLUDED),true)

DMSTIMER_INCLUDED = true
CFLAGS += -D'_DMSTIMER_INCLUDED_=$(DMSTIMER_INCLUDED)' \
			 -D'_DMSTIMER_HEADER_="$(DMSTIMER_LIB).h"'


#Add this library's source code to SRC
SRC += $(DMSTIMER_LIB).c

#Wouldn't it make more sense to set this CFLAG either way? probably have to use #ifeq instead of #ifdef...
ifeq ($(DMS_EXTERNALUPDATE),TRUE)
CFLAGS += -D'_DMS_EXTERNALUPDATE_=$(DMS_EXTERNALUPDATE)'

ifndef XYTRACKER_LIB
VER_XYTRACKER = 3.05
XYTRACKER_LIB = $(COMDIR)/xyTracker/$(VER_XYTRACKER)/xyTracker
include $(XYTRACKER_LIB).mk
endif

else
  #Only set it if it's false or true, 
  #if it's undefined, let dmsTimer.h give a warning
  ifeq ($(DMS_EXTERNALUPDATE),FALSE)
    CFLAGS += -D'_DMS_EXTERNALUPDATE_=$(DMS_EXTERNALUPDATE)'
  endif

  # THERE MUST be a better definition to look for...
  # Maybe "AVR" somewhere?
  ifdef MCU

    # The following variables are defined here for called makefile(s)
    ifndef TIMERCOMMON_LIB
     VER_TIMERCOMMON = 1.21
     TIMERCOMMON_LIB = $(COMDIR)/timerCommon/$(VER_TIMERCOMMON)/timerCommon
     include $(TIMERCOMMON_LIB).mk
    endif

  endif
endif

endif

