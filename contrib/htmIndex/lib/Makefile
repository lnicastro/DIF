# SDSS Science Archive Package Makefile:	
#
#	PACKAGE:	SPATIALINDEX
#

# --- Macros -----------------------------------------------------------------

SHELL	=	${MAKESHELL}

INSTALL_DIR   = $(SPATIALINDEX_INSTALL_DIR)
INSTALL_FILES = *.a Makefile
# --- Targets ----------------------------------------------------------------

default: default_message

clean :
	rm -rf Templates.DB

all :
	@ echo OK
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
	@ rm -rf $(INSTALL_DIR)/lib
	@ mkdir $(INSTALL_DIR)/lib
	@if [ "$(INSTALL_FILES)" != "" ]; then \
		cp $(INSTALL_FILES) $(INSTALL_DIR)/lib; \
		chmod 644 $(INSTALL_DIR)/lib/*; \
	fi

