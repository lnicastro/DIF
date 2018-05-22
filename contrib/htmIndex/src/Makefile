#	PACKAGE:	SpatialIndex
#
# os: linux
# --- Macros -----------------------------------------------------------------

CCXX =		g++
LD = 		g++
INCLUDE =	-I../include
CCFLAGS =	${INCLUDE} -O2 -DLINKAGE= -fPIC
ARFLAGS = 	-rlcs
CP      =       cp
RM      =       rm -f
LIBNAME =	libSpatialIndex

LIBOBJS =       BitList.o\
                SpatialVector.o\
                SpatialIndex.o\
                SpatialInterface.o\
                SpatialEdge.o\
                SpatialException.o\
                SpatialConstraint.o\
                SpatialConvex.o\
                SpatialDomain.o\
		sqlInterface.o\
		VarCmp.o\
                instances.o

LINKLIB = 	../lib/${LIBNAME}.a

# --- Targets ----------------------------------------------------------------

# Build and install the library.

all : ${LINKLIB} share
lib : ${LINKLIB}

${LINKLIB} : ${LIBOBJS}
	${AR} ${ARFLAGS} $@ $?

share:
	$(LD) -shared -o ../lib/${LIBNAME}.so $(LIBOBJS)

# Clean without removing installed products.

tidy :
	- ${RM} ${LIBOBJS}

# Remove all files generated or installed by make.  

clean :	tidy
	- cd ../lib; ${RM} ${LIBNAME}.*; rm -rf Templates.DB
	- cd ../include; ${RM}
	- cd ../doc/www; ${RM}


# --- Suffix Rules -----------------------------------------------------------

# First clear out the default suffixes, then declare our own and
# define the rules.

.SUFFIXES:
.SUFFIXES: .o .cpp

# Compiling C++ code.  The command lines are so long as to be useless.
# Use 'sxmake -terse' to suppress the echo of the compile command.

.cpp.o:
	@ echo compiling $<
	 ${CCXX} -c ${CCFLAGS} $<

