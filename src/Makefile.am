# ----------------------------------------------------------------------^
# Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009 Giorgio Calderone <gcalderone@ifc.inaf.it>
# 
# This file is part of DIF.
# 
# DIF is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# DIF is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with DIF; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
# 
# ----------------------------------------------------------------------$

#
# NOTE: Both HTM and HEALPix libraries must be compiled with the -fPIC option
#

HTM_INC = -I../contrib/htmIndex/include
HEALP_INC = -I../contrib/Healpix/HealP3

AM_CPPFLAGS =  -DMYSQL_DYNAMIC_PLUGIN $(HTM_INC) $(HEALP_INC)

noinst_HEADERS = dif.hh ha_dif.h ha_dif_maria.h udf_utils.hh my_stmt_db.h my_stmt_db2.h pix_myXmatch_def.hh

lib_LTLIBRARIES = ha_dif.la
ha_dif_la_CXXFLAGS = $(INCLUDES) -fno-rtti
ha_dif_la_LDFLAGS = -module
ha_dif_la_SOURCES = \
   udf.cc \
   difflist_i.cpp skysep_h.cpp \
   getHTMid.cpp getHTMidByName.cpp getHTMnameById.cpp \
   DIFhtmCircleRegion.cpp DIFhtmRectRegion.cpp \
   getHTMNeighb.cpp getHTMNeighbC.cpp getHTMBary.cpp getHTMBaryC.cpp \
   getHTMBaryDist.cpp DIFgetHTMNeighbC.cpp \
   getHealPBound.cpp getHealPBoundC.cpp getHealPid.cpp \
   getHealPNeighb.cpp getHTMsNeighb.cpp getHealPNeighbC.cpp \
   getHealPBary.cpp getHealPBaryC.cpp \
   getHealPBaryDist.cpp \
   DIFmyHealPCone.cpp DIFmyHealPRect.cpp \
   DIFgetHealPNeighbC.cpp \
   DIFgetHTMsNeighb.cpp \
   getHealPMaxS.cpp

if MYSQL8
ha_dif_la_SOURCES += ha_dif_my8.cc
else
ha_dif_la_SOURCES += ha_dif.cc
endif

ha_dif_la_LIBADD = ../contrib/htmIndex/lib/libSpatialIndex.a \
                   ../contrib/Healpix/HealP3/lib/libHealP3.a \
                   ../contrib/Spherematch/lib/libspheregroup.a

noinst_LIBRARIES = libMyUtils.a libdif_alone.a
libMyUtils_a_SOURCES = \
   my_stmt_db.c my_stmt_db2.c deg_radec.c

libdif_alone_a_SOURCES = $(ha_dif_la_SOURCES)

bin_PROGRAMS = testMySearch fakesky_H6 fakesky_RND fakesky_HPx myXmatch pix_myXmatch
fakesky_H6_SOURCES = fakesky_H6.cc
fakesky_RND_SOURCES = fakesky_RND.cc
fakesky_HPx_SOURCES = fakesky_HPx.cc
myXmatch_SOURCES = myXmatch.cc
pix_myXmatch_SOURCES = pix_myXmatch.cc
myXmatch_LDADD = ./libMyUtils.a ../contrib/Spherematch/lib/libspheregroup.a @MYSQL_LIBS@
pix_myXmatch_LDADD = ./libMyUtils.a ../contrib/Spherematch/lib/libspheregroup.a @MYSQL_LIBS@
fakesky_H6_LDADD = ./libdif_alone.a ./libMyUtils.a ../contrib/htmIndex/lib/libSpatialIndex.a @MYSQL_LIBS@
fakesky_RND_LDADD = ./libdif_alone.a ./libMyUtils.a ../contrib/htmIndex/lib/libSpatialIndex.a @MYSQL_LIBS@
fakesky_HPx_LDADD = ./libdif_alone.a ./libMyUtils.a ../contrib/Healpix/HealP3/lib/libHealP3.a @MYSQL_LIBS@
testMySearch_SOURCES = testMySearch.cc
testMySearch_LDADD = ./libdif_alone.a ../contrib/htmIndex/lib/libSpatialIndex.a \
                     ../contrib/Healpix/HealP3/lib/libHealP3.a \
                     ../contrib/Spherematch/lib/libspheregroup.a

testMySearch_LDFLAGS = -pthread



install-data-hook:
# LN 09/06/2011: manage shared lib extension
# LN 16/11/2018: place the libraries in the plugin dir to avoid apparmor issues
#
# Since version 5.1.24 the option --plugindir was added
#	if test -z $$pd; then pd="`@MYSQL_CONFIG@ --libs | @PERL@ ../selectflags.pl | @PERL@ -pe 'chomp; s/-L//g; s/ +//g;'`/plugin"; fi;
# Do not check in my.cnf anymore
#	@if test -e /etc/my.cnf; then pd="`cat /etc/my.cnf | grep '^plugin_dir' | @AWK@ -F '=' '{print $$2}'`"; \
#else echo WARNING: no /etc/my.cnf file found. && pd=""; fi; \
#if test -z $$pd; then pd="`@MYSQL_CONFIG@ --plugindir`"; fi; \
#	echo "MySQL plugin dir: $$pd"; \
#	if ! test $$pd = ${libdir}; \
#	then echo "Creating a symbolic link in $$pd to ${libdir}/ha_dif.so"; \
#	ln -sf ${libdir}/ha_dif.so $$pd; fi
	@pd="`@MYSQL_CONFIG@ --plugindir`"; \
	echo "MySQL plugin dir: $$pd"; \
	cp -a ${libdir}/ha_dif.* $$pd
