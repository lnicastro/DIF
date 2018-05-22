//#     Filename:       SpatialVector.hxx
//#
//#     Standard 3-d vector class: .h implementations
//#
//#
//#     Author:         Peter Z. Kunszt
//#     
//#     Date:           October 15, 1998
//#
//#
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
// Friend operators
SpatialVector operator *(float64, const SpatialVector&);
SpatialVector operator *(int, const SpatialVector&);
SpatialVector operator *(const SpatialVector&, float64);
SpatialVector operator *(const SpatialVector&, int);

// inline functions

inline
float64 SpatialVector::x() const {
  return x_;
}

inline
float64 SpatialVector::y() const {
  return y_;
}

inline
float64 SpatialVector::z() const {
  return z_;
}

/////////////>>///////////////////////////////////////////
// read from istream
//
inline
istream& operator >>( istream& in, SpatialVector & v) {
  v.read(in);
  return(in);
}

/////////////<<///////////////////////////////////////////
// write to ostream
//
inline
ostream& operator <<( ostream& out, const SpatialVector & v) {
  v.write(out);
  return(out);
}
