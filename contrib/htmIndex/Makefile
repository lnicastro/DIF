#
#
#	PACKAGE:	SpatialIndex
#

# --- Targets ----------------------------------------------------------------

default: all

all clean :
	@ cd src; ${MAKE} $@
#	@ cd app; ${MAKE} $@

libs :
	@ cd src; ${MAKE} all

apps : libs
	@ cd app; ${MAKE} all

linux :
	@ cd src; cp Makefile.linux Makefile
#	@ cd app; cp Makefile.linux Makefile

darwin :
	@ cd src; cp Makefile.darwin Makefile
#	@ cd app; cp Makefile.darwin Makefile

irix :
	@ cd src; cp Makefile.irix Makefile
#	@ cd app; cp Makefile.irix Makefile

osf :
	@ cd src; cp Makefile.osf Makefile
#	@ cd app; cp Makefile.osf Makefile

solaris :
	@ cd src; cp Makefile.solaris Makefile
#	@ cd app; cp Makefile.solaris Makefile

make tidy :
	@ cd src; ${MAKE} $@
	@ cd app; ${MAKE} $@

