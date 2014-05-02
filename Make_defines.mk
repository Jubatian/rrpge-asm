######################
# Make - definitions #
######################
#
# Author    Sandor Zsuga (Jubatian)
# Copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
#           License) extended as RRPGEv2 (version 2 of the RRPGE License): see
#           LICENSE.GPLv3 and LICENSE.RRPGEv2 in the project root.
#
#
#
#
# This file holds general definitions used by any makefile, like compiler
# flags, optimization and such. OS - specific building schemes should also
# be written here.
#
#
include Make_config.mk
CFLAGS=
#
#
# Linux - specific parameters (TSYS=linux)
#
ifeq ($(TSYS),linux)
CFLAGS+= -DTARGET_LINUX
LINKB=
endif
#
#
# Windows - MinGW specific
#
ifeq ($(TSYS),windows_mingw)
CFLAGS+= -DTARGET_WINDOWS_MINGW
LINKB=-lmingw32 -mwindows
SHRM=del
SHMKDIR=md
DIRSP=\\
endif
#
#
# When asking for debug edit
#
ifeq ($(GO),test)
CFSPD=-O0 -g
CFSIZ=-O0 -g
CFLAGS+= -DTARGET_DEBUG
endif
#
#
# 'Production' edit
#
CFSPD?=-O2 -s
CFSIZ?=-Os -s
#
#
# Now on the way...
#

SHRM?=rm -r -f
SHMKDIR?=mkdir
DIRSP=/

LINKB?=
LINK= $(LINKB)

OBB=_obj_
OBD=$(OBB)$(DIRSP)

CFLAGS+= -Wall -pipe -pedantic
ifneq ($(CC_BIN),)
CFLAGS+= -B$(CC_BIN)
endif
ifneq ($(CC_LIB),)
CFLAGS+= -L$(CC_LIB)
endif
ifneq ($(CC_INC),)
CFLAGS+= -I$(CC_INC)
endif

CFSPD+= $(CFLAGS)
CFSIZ+= $(CFLAGS)

