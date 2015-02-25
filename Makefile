############
# Makefile #
############
#
# Author    Sandor Zsuga (Jubatian)
# Copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
#           License) extended as RRPGEvt (temporary version of the RRPGE
#           License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
#           root.
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
OBJECTS+=$(OBD)bindata.o $(OBD)compst.o  $(OBD)fault.o   $(OBD)firead.o
OBJECTS+=$(OBD)incstk.o  $(OBD)litpr.o   $(OBD)opcdec.o  $(OBD)opcpr.o
OBJECTS+=$(OBD)pass1.o   $(OBD)pass2.o   $(OBD)pass3.o   $(OBD)ps1sup.o
OBJECTS+=$(OBD)section.o $(OBD)strpr.o   $(OBD)symtab.o  $(OBD)valwr.o


all: $(OUT)
clean:
	$(SHRM) $(OBJECTS) $(OUT)
	$(SHRM) $(OBB)


$(OUT): $(OBB) $(OBJECTS)
	$(CC) -o $(OUT) $(OBJECTS) $(CFSIZ) $(LINK)

$(OBB):
	$(SHMKDIR) $(OBB)

$(OBD)main.o: main.c *.h
	$(CC) -c main.c -o $(OBD)main.o $(CFSIZ)

$(OBD)bindata.o: bindata.c *.h
	$(CC) -c bindata.c -o $(OBD)bindata.o $(CFSIZ)

$(OBD)compst.o: compst.c *.h
	$(CC) -c compst.c -o $(OBD)compst.o $(CFSIZ)

$(OBD)fault.o: fault.c *.h
	$(CC) -c fault.c -o $(OBD)fault.o $(CFSIZ)

$(OBD)firead.o: firead.c *.h
	$(CC) -c firead.c -o $(OBD)firead.o $(CFSIZ)

$(OBD)incstk.o: incstk.c *.h
	$(CC) -c incstk.c -o $(OBD)incstk.o $(CFSIZ)

$(OBD)litpr.o: litpr.c *.h
	$(CC) -c litpr.c -o $(OBD)litpr.o $(CFSIZ)

$(OBD)opcdec.o: opcdec.c *.h
	$(CC) -c opcdec.c -o $(OBD)opcdec.o $(CFSIZ)

$(OBD)opcpr.o: opcpr.c *.h
	$(CC) -c opcpr.c -o $(OBD)opcpr.o $(CFSIZ)

$(OBD)pass1.o: pass1.c *.h
	$(CC) -c pass1.c -o $(OBD)pass1.o $(CFSIZ)

$(OBD)pass2.o: pass2.c *.h
	$(CC) -c pass2.c -o $(OBD)pass2.o $(CFSIZ)

$(OBD)pass3.o: pass3.c *.h
	$(CC) -c pass3.c -o $(OBD)pass3.o $(CFSIZ)

$(OBD)ps1sup.o: ps1sup.c *.h
	$(CC) -c ps1sup.c -o $(OBD)ps1sup.o $(CFSIZ)

$(OBD)section.o: section.c *.h
	$(CC) -c section.c -o $(OBD)section.o $(CFSIZ)

$(OBD)strpr.o: strpr.c *.h
	$(CC) -c strpr.c -o $(OBD)strpr.o $(CFSIZ)

$(OBD)symtab.o: symtab.c *.h
	$(CC) -c symtab.c -o $(OBD)symtab.o $(CFSIZ)

$(OBD)valwr.o: valwr.c *.h
	$(CC) -c valwr.c -o $(OBD)valwr.o $(CFSIZ)


.PHONY: all clean
