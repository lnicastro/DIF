#	PACKAGE:	SpatialIndex
#
# os: linux
# --- Macros -----------------------------------------------------------------

CCXX =		g++
LD = 		g++
INCLUDE =	-I../include
CCFLAGS =	${INCLUDE} -DLINKAGE=
ARFLAGS = 	-rlcs
CP      =       cp
RM      =       rm -f
LIBRARY =	../lib/libSpatialIndex.a

EXECUTABLES =	timeHTM\
		lookup\
		echoDomain\
		intersect

TEST_OBJS =	timeHTM.o

ISECT_OBJS =	intersect.o

LOOKUP_OBJS =	lookup.o

ECHO_OBJS =	echoDomain.o

LDLIBS =	${LIBRARY} -lm

# --- Targets ----------------------------------------------------------------

# Rules for building each executable.

timeHTM: ${TEST_OBJS} ${LIBRARY}
	@ echo linking $@
	@ ${LD} -o $@ ${CCFLAGS} ${TEST_OBJS} ${LDLIBS}

Mylookup: Mylookup.o ${LIBRARY}
	@ echo linking $@
	@ ${LD} -o $@ ${CCFLAGS} Mylookup.o ${LDLIBS}

lookup: ${LOOKUP_OBJS} ${LIBRARY}
	@ echo linking $@
	@ ${LD} -o $@ ${CCFLAGS} ${LOOKUP_OBJS} ${LDLIBS}

Myintersect: Myintersect.o ${LIBRARY}
	@ echo linking $@
	@ ${LD} -o $@ ${CCFLAGS} Myintersect.o ${LDLIBS}

sqlExample: sqlExample.o ${LIBRARY}
	@ echo linking $@
	@ ${LD} -o $@ ${CCFLAGS} sqlExample.o ${LDLIBS}

rectEx: rectEx.o ${LIBRARY}
	@ echo linking $@
	@ ${LD} -o $@ ${CCFLAGS} rectEx.o ${LDLIBS}

intersect: ${ISECT_OBJS} ${LIBRARY}
	@ echo linking $@
	@ ${LD} -o $@ ${CCFLAGS} ${ISECT_OBJS} ${LDLIBS}

echoDomain: ${ECHO_OBJS} ${LIBRARY}
	@ echo linking $@
	@ ${LD} -o $@ ${CCFLAGS} ${ECHO_OBJS} ${LDLIBS}

# Build and install the executables.

apps : ${EXECUTABLES}
	- ${CP} ${EXECUTABLES} ../bin

all :	apps

# Clean without removing installed products.

tidy :
	- ${RM} ${EXECUTABLES} ${TEST_OBJS} ${LOOKUP_OBJS} ${ECHO_OBJS} ${ISECT_OBJS}

# Remove all files generated or installed by make.  

clean :	tidy
	- cd ../bin; ${RM} ${EXECUTABLES}

# --- Suffix Rules -----------------------------------------------------------

# First clear out the default suffixes, then declare our own and
# define the rules.

.SUFFIXES:
.SUFFIXES: .o .cpp

# Compiling C++ code.  The command lines are so long as to be useless.
# Use 'sxmake -terse' to suppress the echo of the compile command.

.cpp.o:
	@ echo compiling $<
	@ ${CCXX} -c ${CCFLAGS} $<

