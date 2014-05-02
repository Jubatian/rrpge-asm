############
# Makefile #
############
#
# Author    Sandor Zsuga (Jubatian)
# Copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
#           License) extended as RRPGEv2 (version 2 of the RRPGE License): see
#           LICENSE.GPLv3 and LICENSE.RRPGEv2 in the project root.
#
#
#
#
# The main makefile of the program
#
#
# make all (or make): build the program
# make clean:         to clean up
#
#

include Make_defines.mk

CFLAGS+=

OBJECTS= $(OBD)main.o
OBJECTS+=$(OBD)compst.o $(OBD)fault.o  $(OBD)firead.o $(OBD)incstk.o
OBJECTS+=$(OBD)litpr.o  $(OBD)opcpr.o  $(OBD)pass1.o  $(OBD)pass2.o
OBJECTS+=$(OBD)pass3.o  $(OBD)ps1sup.o $(OBD)strpr.o  $(OBD)valwr.o


all: $(OUT)
clean:
	$(SHRM) $(OBJECTS) $(OUT)
	$(SHRM) $(OBB)


$(OUT): $(OBB) $(OBJECTS)
	$(CC) -o $(OUT) $(OBJECTS) $(CFSIZ) $(LINK)

$(OBB):
	$(SHMKDIR) $(OBB)

$(OBD)main.o: main.c
	$(CC) -c main.c -o $(OBD)main.o $(CFSIZ)

$(OBD)compst.o: compst.c
	$(CC) -c compst.c -o $(OBD)compst.o $(CFSIZ)

$(OBD)fault.o: fault.c
	$(CC) -c fault.c -o $(OBD)fault.o $(CFSIZ)

$(OBD)firead.o: firead.c
	$(CC) -c firead.c -o $(OBD)firead.o $(CFSIZ)

$(OBD)incstk.o: incstk.c
	$(CC) -c incstk.c -o $(OBD)incstk.o $(CFSIZ)

$(OBD)litpr.o: litpr.c
	$(CC) -c litpr.c -o $(OBD)litpr.o $(CFSIZ)

$(OBD)opcpr.o: opcpr.c
	$(CC) -c opcpr.c -o $(OBD)opcpr.o $(CFSIZ)

$(OBD)pass1.o: pass1.c
	$(CC) -c pass1.c -o $(OBD)pass1.o $(CFSIZ)

$(OBD)pass2.o: pass2.c
	$(CC) -c pass2.c -o $(OBD)pass2.o $(CFSIZ)

$(OBD)pass3.o: pass3.c
	$(CC) -c pass3.c -o $(OBD)pass3.o $(CFSIZ)

$(OBD)ps1sup.o: ps1sup.c
	$(CC) -c ps1sup.c -o $(OBD)ps1sup.o $(CFSIZ)

$(OBD)strpr.o: strpr.c
	$(CC) -c strpr.c -o $(OBD)strpr.o $(CFSIZ)

$(OBD)valwr.o: valwr.c
	$(CC) -c valwr.c -o $(OBD)valwr.o $(CFSIZ)


.PHONY: all clean
