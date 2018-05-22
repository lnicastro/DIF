//#
//#  File: VarCmp.cpp
//#
//#
//#  Author: Peter Z Kunszt
//#
//#
//#  Creation Date: Jan 5 2001
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

#include "VarCmp.h"

int varCmpInt8( const void *a, const void *b ) {
  return (int)( *((int8 *)a) - *((int8*)b) );
}

int varCmpInt16( const void *a, const void *b ) {
  return (int) ( *((int16 *)a) - *((int16*)b) );
}


int varCmpInt32( const void *a, const void *b ) {
  return (int) ( *((int32 *)a) - *((int32*)b) );
}


int varCmpInt64( const void *a, const void *b ) {
  if ( *((int64 *)a) < *((int64*)b) ) return -1;
  return ( (*((int64 *)a) > *((int64*)b)) ? 1 : 0) ;
}


int varCmpUint8( const void *a, const void *b ) {
  if ( *((uint8 *)a) < *((uint8*)b) ) return -1;
  return ( (*((uint8 *)a) > *((uint8*)b)) ? 1 : 0) ;
}


int varCmpUint16( const void *a, const void *b ) {
  if ( *((uint16 *)a) < *((uint16*)b) ) return -1;
  return ( (*((uint16 *)a) > *((uint16*)b)) ? 1 : 0) ;
}


int varCmpUint32( const void *a, const void *b ) {
  if ( *((uint32 *)a) < *((uint32*)b) ) return -1;
  return ( (*((uint32 *)a) > *((uint32*)b)) ? 1 : 0) ;
}


int varCmpUint64( const void *a, const void *b ) {
  if ( *((uint64 *)a) < *((uint64*)b) ) return -1;
  return ( (*((uint64 *)a) > *((uint64*)b)) ? 1 : 0) ;
}


int varCmpFloat32( const void *a, const void *b ) {
  if ( *((float32 *)a) < *((float32*)b) ) return -1;
  return ( (*((float32 *)a) > *((float32*)b)) ? 1 : 0) ;
}


int varCmpFloat64( const void *a, const void *b ) {
  if ( *((float64 *)a) < *((float64*)b) ) return -1;
  return ( (*((float64 *)a) > *((float64*)b)) ? 1 : 0) ;
}



