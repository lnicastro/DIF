# Makefile for custom HEALPix facilities to be used in DIF.
#
# Here just compile HEALPix libraries
#
# LN@IASF-INAF, March 2009                        Last change: 18/05/2016
#--

MAKE = make
PREFIX = .
HEALPDIR = $(PREFIX)/HealP3


default: all

# Compile/clean HEALPix libraries
all clean :
	@ cd $(HEALPDIR) ; $(MAKE) $@

#linux :
	#@ cd $(HEALPDIR) ; cp Makefile.linux Makefile

#darwin :
	#@ cd $(HEALPDIR) ; cp Makefile.darwin Makefile
