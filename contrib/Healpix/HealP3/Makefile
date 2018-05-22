# Makefile for custom HEALPix facilities to be used in DIF.
# 
# LN@IASF-INAF, Aug. 2007                        Last change: 16/05/2016
#--

MAKE ?= make
CCXX ?= g++
LD = g++
PREFIX = .
CPPFLAGS = -Wall -O3 -fPIC
BINDIR = $(PREFIX)/bin
LIBDIR = $(PREFIX)/lib
DOCDIR = $(SRCROOT)/doc

INC = -I$(PREFIX)

LNAME = HealP3
L = lib$(LNAME).a

# do not use any suffix rules
.SUFFIXES:

%.o : %.cc
	$(CCXX) -c $(CPPFLAGS) $(INC) $<

# The C++ library modules
CCMODS = healpix_base healpix_map healpix_tables pointing error_handling geom_utils string_utils

HFILES = healpix_base.h healpix_map.h healpix_tables.h pointing.h \
	arr.h compress_utils.h datatypes.h error_handling.h geom_utils.h lsconstants.h \
	math_utils.h openmp_support.h pointing.h string_utils.h vec3.h

CCSRCS = ${CCMODS:%=%.cc}
LIBOBJS = ${CCMODS:%=%.o}

default: all

all: creadirs $(L) share install

creadirs:
	@if [ ! -d $(LIBDIR) ]; then mkdir $(LIBDIR) ; fi

$(L): $(LIBOBJS) $(HFILES)
	ar crv $(L) $(LIBOBJS)

share:
	$(LD) -shared -o lib$(LNAME).so $(LIBOBJS)

install:
	ranlib $(L)
	mv $(L) lib$(LNAME).so $(LIBDIR)

clean:
	rm -f *.o *.a $(BINDIR)/*
	rm -f $(LIBDIR)/*.a

# Dependencies
healpix_base.o: healpix_base.h arr.h geom_utils.h
healpix_map.o: healpix_base.h healpix_map.h arr.h
