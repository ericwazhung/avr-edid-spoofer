APP_NAME = dmsTimerTest

COMDIR = ../../../

SRC = 
CFLAGS += -I.

#VER_DMSTIMER = 1.09
DMSTIMER_LIB = ../dmsTimer
#DMS_EXTERNALUPDATE = TRUE
include $(DMSTIMER_LIB).mk

SRC += main.c
OBJ_FILES = $(SRC:.c=.o)

CFLAGS += -D'TRUE=1' -D'FALSE=0'

$(APP_NAME): $(OBJ_FILES)
	@echo "Compiling $(APP_NAME)" 
	gcc $(OBJ_FILES) -o $(APP_NAME)

%.o: %.c
	@echo "Compiling $@ from $<"
	gcc -c $(CFLAGS) -o $@ $<

clean:
	/Users/meh/myexecs/myrm.sh -f $(APP_NAME) $(OBJ_FILES)

