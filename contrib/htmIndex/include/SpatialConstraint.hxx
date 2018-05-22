//#     Filename:       SpatialConstraint.hxx
//#
//#     H implementations for spatialconstraint
//#
//#
//#     Author:         Peter Z. Kunszt, based on A. Szalay's code
//#     
//#     Date:           October 16, 1998
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
//#
extern istream& operator >>( istream&, SpatialConstraint &);
extern ostream& operator <<( ostream&, const SpatialConstraint &);

// 
inline
SpatialVector &
SpatialConstraint::v() {
  return a_;
}

inline
float64
SpatialConstraint::d() const {
  return d_;
}

inline
void
SpatialConstraint::setVector(SpatialVector &v) {
  a_.set(v.x(),v.y(),v.z());
}

inline
void
SpatialConstraint::setDistance(float64 d) {
  d_ = d;
}

