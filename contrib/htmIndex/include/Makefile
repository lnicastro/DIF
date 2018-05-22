# SDSS Science Archive Package/include Makefile:
#
#	PACKAGE:	SpatialIndex
#
# Copyright (C) 2000  Peter Z. Kunszt, Alex S. Szalay, Aniruddha R. Thakar
#                     The Johns Hopkins University
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

# --- Macros -----------------------------------------------------------------

# For the 'html' target.  All include files to be installed in the
# doc/www directory for this package:

HTMLDOC =	SpatialConstraint.h\
		SpatialConvex.h\
		SpatialDomain.h\
		SpatialIndex.h\
		SpatialInterface.h\
		SpatialVector.h\
		SpatialException.h\
		BitList.h\
		VarVec.h\
		VarStr.h\
		sqlInterface.h\
		SpatialDoc.h

SHELL = ${MAKESHELL}

DOCXX_FLAGS = -H -b -d ../doc/www -B ../etc/copyright.html

INSTALL_DIR   = $(SPATIALINDEX_INSTALL_DIR)
INSTALL_FILES = *.h *.hxx *.inc Makefile
# --- Targets ----------------------------------------------------------------

default: default_message

# Make online documentation, anchored to this directory, for each of
# the files specified in HTML_CXX.  

html : ${HTMLDOC:.h=.dxx}
	  @echo Making Documentation...
	  ${TERSE} ${DOCXX_DIR}/doc++ ${DOCXX_FLAGS} ${HTMLDOC:.h=.dxx} > dxx 2>&1
	  ${TERSE} ${RM} ${HTMLDOC:.h=.dxx} dxx

clean :
	- cd ../doc/www; ${RM} *.html .html *.class *.tex gifs.db icon*.gif down.gif logo.gif;

# Naive attempt to force use of sxmake.  See 'default' target above.

default_message	:
	@ echo ############################################################
	@ echo
	@ echo ERROR: Invalid make request.
	@ echo
	@ echo Please invoke this makefile using sxmake.
	@ echo
	@ echo ############################################################
	@ echo

install :
	@if [ "$(INSTALL_DIR)" = "" ]; then \
		echo You have not specified a destination directory >&2; \
		exit 1; \
	fi
	@ rm -rf $(INSTALL_DIR)/include
	@ mkdir $(INSTALL_DIR)/include
	@if [ "$(INSTALL_FILES)" != "" ]; then \
		cp $(INSTALL_FILES) $(INSTALL_DIR)/include; \
		chmod 644 $(INSTALL_DIR)/include/*; \
	fi

# --- Suffix Rules -----------------------------------------------------------

# First clear out the default suffixes, then declare our own and
# define the rules.

.SUFFIXES:
.SUFFIXES: .h .dxx

# generating doc files

.h.dxx:
	${TERSE} ${DOCXX_DIR}/docify $*.h $*.d
	${TERSE} sed -e 's/LINKAGE //g' $*.d > $*.dxx
	${TERSE} rm -f $*.d
