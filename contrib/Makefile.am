# ----------------------------------------------------------------------^
# Copyright (C) 2004, 2005, 2006, 2007 Giorgio Calderone <gcalderone@ifc.inaf.it>
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

EXTRA_DIST = htmIndex Healpix Spherematch

noinst_DATA = dummy

dummy:
	make -C htmIndex/src
	make -C Healpix
	make -C Spherematch/src
	touch dummy

clean-local:
	make -C htmIndex clean
	make -C Healpix clean
	make -C Spherematch clean
	rm -f dummy
