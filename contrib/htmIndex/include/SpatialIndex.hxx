//#     Filename:       SpatialIndex.hxx
//#
//#     H Implementations for spatialindex
//#
//#
//#     Author:         Peter Z. Kunszt, based on A. Szalay s code
//#
//#     Date:           October 15, 1998
//#
//#
//# Copyright (C) 2000  Peter Z. Kunszt, Alex S. Szalay, Aniruddha R. Thakar
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
//#
//#

/////////////leafCount//////////////////////////////////////
// leafCount: return number of leaf nodes
inline uint64
SpatialIndex::leafCount() const
{
  return leaves_;
}

/////////////NVERTICES////////////////////////////////////
// nVertices: return number of vertices
inline size_t
SpatialIndex::nVertices() const
{
  return vertices_.length();
}

//////////////////LEAFNUMBERBYID///////////////////////////////////////////
//
inline uint32
SpatialIndex::leafNumberById(uint64 id) const{
  if(maxlevel_ > HTMMAXBIT)
    throw SpatialInterfaceError("SpatialIndex:leafNumberById","BitList may only be used up to level HTMMAXBIT deep");

  return (uint32)(id - leafCount());
}

//////////////////IDBYLEAFNUMBER///////////////////////////////////////////
//
inline uint64
SpatialIndex::idByLeafNumber(uint32 n) const{
  uint64 l = leafCount();
  l += n;
  return l;
}

//////////////////NAMEBYLEAFNUMBER////////////////////////////////////////
//
inline char *
SpatialIndex::nameByLeafNumber(uint32 n, char * name) const{
  return nameById(idByLeafNumber(n), name);
}

//////////////////IDBYPOINT////////////////////////////////////////////////
// Find a leaf node where a ra/dec points to
//

inline uint64
SpatialIndex::idByPoint(const float64 & ra, const float64 & dec) const {
  SpatialVector v(ra,dec);
  return idByPoint(v);
}

//////////////////NAMEBYPOINT//////////////////////////////////////////////
// Find a leaf node where a ra/dec points to, return its name
//

inline char*
SpatialIndex::nameByPoint(const float64 & ra, const float64 & dec, 
			  char* name) const {
  return nameById(idByPoint(ra,dec), name);
}

//////////////////NAMEBYPOINT//////////////////////////////////////////////
// Find a leaf node where v points to, return its name
//

inline char*
SpatialIndex::nameByPoint(SpatialVector & vector, char* name) const {
  return nameById(idByPoint(vector),name);
}
