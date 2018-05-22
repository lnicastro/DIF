SHELL = /bin/sh

#=============================================================================
#
#  INSTALL_DIR   --- Root directory into which the product is to be installed.
#  INSTALL_FILES --- Names of the files to be installed.
#  CLEAN_FILES   --- Names of specific files to be deleted when the directory
#                    is cleaned.  Files matching one of a default set of file
#                    name templates will be deleted automatically
#                    (i.e., *~ core *.bak *.orig *.old .#* #*# *.o).  This
#                    variable is for declaring additional files to delete which
#                    don't match any of the file name templates.
#
#=============================================================================

INSTALL_DIR   = $(SPATIALINDEX_INSTALL_DIR)/doc
SUBDIRECTORIES= www
CLEAN_FILES   =

#===========================================================================
# STANDARD TARGETS
#===========================================================================

default:
	@echo Please invoke this makefile using sdssmake. >&2
	@exit 1

all :;

install :
	@if [ "$(INSTALL_DIR)" = "" ]; then \
		echo You have not specified a destination directory >&2; \
		exit 1; \
	fi     
	@ rm -rf $(INSTALL_DIR)
	@ mkdir $(INSTALL_DIR)
	/bin/cp Makefile $(INSTALL_DIR)
	@for i in *; do   \
		if [ -f $$i ]; then \
				cp $$i $(INSTALL_DIR) ; \
		fi;\
        done
	@ for f in $(SUBDIRECTORIES); do \
	(cd $$f ; echo In $$f; $(MAKE) $(MFLAGS) install ); \
        done

clean :

