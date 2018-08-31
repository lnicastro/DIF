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
LIBDIR = ../lib
CFLAGS  = -g -O3 -Wall -fPIC -I$(INC) # -DDEBUG -DCHECK_LEAKS
CPPFLAGS = -g -O3 -Wall -fPIC -I$(INC) # -DDEBUG -DCHECK_LEAKS
LDFLAGS =
LIB_A = $(LIBDIR)/libspheregroup.a
LIB_SO = $(LIBDIR)/libspheregroup.so

OBJECTS = \
	spherematch2.o \
	spheregroup.o \
	chunks.o \
	rarange.o \
	separation.o \
	friendsoffriends.o \
	chunkfriendsoffriends.o 


all : def $(LIB_SO)
def : creadirs $(LIB_A)

creadirs:
	@if [ ! -d $(LIBDIR) ]; then mkdir $(LIBDIR) ; fi

$(LIB_SO): $(OBJECTS)
	$(LD) -shared $(LDFLAGS) -o $(LIB_SO) $(OBJECTS)
$(LIB_A): $(OBJECTS)
	ar rv $(LIB_A) $?

clean :
	- /bin/rm -f *~ core *.o $(LIBDIR)/libspheregroup.*
