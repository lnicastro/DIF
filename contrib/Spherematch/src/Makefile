###############################################################################
# Sloan Digital Sky Survey (SDSS)
# IDL utilities for group finding and matching quickly
#
# M.R. Blanton
###############################################################################
#
# LN modif

SHELL = /bin/sh
CCPP ?= g++
LD = g++
INC = ./
LIB = ../lib
CFLAGS  = -g -O3 -Wall -fPIC -I$(INC) # -DDEBUG -DCHECK_LEAKS
CPPFLAGS = -g -O3 -Wall -fPIC -I$(INC) # -DDEBUG -DCHECK_LEAKS
LDFLAGS =
LIB_A = $(LIB)/libspheregroup.a
LIB_SO = $(LIB)/libspheregroup.so

OBJECTS = \
	spherematch2.o \
	spheregroup.o \
	chunks.o \
	rarange.o \
	separation.o \
	friendsoffriends.o \
	chunkfriendsoffriends.o 

all : $(LIB_SO) $(LIB_A)
def : $(LIB_A)


$(LIB_SO): $(OBJECTS)
	$(LD) -shared $(LDFLAGS) -o $(LIB_SO) $(OBJECTS)
$(LIB_A): $(OBJECTS)
	ar rv $(LIB_A) $?

clean :
	- /bin/rm -f *~ core *.o $(LIB)/libspheregroup.*
