#ifndef _VARCMP_H
#define _VARCMP_H
//#
//#  File: VarCmp.h
//#
//#
//#  Author: Peter Z Kunszt
//#
//#
//#  Creation Date:  Jan 5 2001
//#
//#...........................................................................
//# Copyright (C) 2000  Peter Z. Kunszt, Alex S. Szalay, Aniruddha R. Thakar
//#                     The SDSS Collaboration and
//#                     The Johns Hopkins University
//#
//# This program is free software; you can redistribute it and/or
//# modify it under the terms of the GNU General Public License
//# as published by the Free Software Foundation; either version 2
//# of the License, or (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//#...........................................................................
//#
//#
//#     Modification History:
//#

#include "SpatialGeneral.h"

/** Functions to compare standard values are provided here.
    These can be used by VarVec<>::sort()
*/


int varCmpInt8( const void *, const void * );
int varCmpInt16( const void *, const void * );
int varCmpInt32( const void *, const void * );
int varCmpInt64( const void *, const void * );
int varCmpUint8( const void *, const void * );
int varCmpUint16( const void *, const void * );
int varCmpUint32( const void *, const void * );
int varCmpUint64( const void *, const void * );
int varCmpFloat32( const void *, const void * );
int varCmpFloat64( const void *, const void * );


#endif
