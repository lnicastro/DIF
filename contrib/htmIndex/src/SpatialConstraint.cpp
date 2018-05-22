//#     Filename:       SpatialConstraint.cpp
//#
//#     The SpatialConstraint, SpatialSign
//#     classes are defined here.
//#
//#     Author:         Peter Z. Kunszt based on A. Szalay's code
//#     
//#     Date:           October 23, 1998
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
//#
//#     Modification History:
//#
#include "SpatialConstraint.h"
#include "SpatialException.h"

#define COMMENT '#'

// ===========================================================================
//
// Member functions for class SpatialSign
//
// ===========================================================================

/////////////CONSTRUCTOR//////////////////////////////////
//
SpatialSign::SpatialSign(Sign sign) : sign_(sign) {
}

/////////////COPY CONSTRUCTOR/////////////////////////////
//
SpatialSign::SpatialSign(const SpatialSign & oldSign) : sign_(oldSign.sign_) {
}

/////////////ASSIGNMENT///////////////////////////////////
//
SpatialSign &
SpatialSign::operator =(const SpatialSign & oldSign) {
  if( & oldSign != this)sign_ = oldSign.sign_;
  return *this;
}

// ===========================================================================
//
// Member functions for class SpatialConstraint
//
// ===========================================================================

/////////////CONSTRUCTOR//////////////////////////////////
//
SpatialConstraint::SpatialConstraint(SpatialVector a, float64 d) :
  a_(a), d_(d)
{
  a_.normalize();
  s_ = acos(d_);
  if(d_ <= -gEpsilon) sign_ = nEG;
  if(d_ >=  gEpsilon) sign_ = pOS;
}

/////////////COPY CONSTRUCTOR/////////////////////////////
//
SpatialConstraint::SpatialConstraint(const SpatialConstraint & old) :
  a_(old.a_), d_(old.d_), s_(old.s_) {
  sign_ = old.sign_;
}

/////////////ASSIGNMENT///////////////////////////////////
//
SpatialConstraint &
SpatialConstraint::operator =(const SpatialConstraint & old)
{
  if ( &old != this ) { // beware of self-assignment
    a_ = old.a_;
    d_ = old.d_;
    s_ = old.s_;
    sign_ = old.sign_;
  }
  return *this;
}

/////////////CONTAINS/////////////////////////////////////
// check whether a vector is inside this
//
bool 
SpatialConstraint::contains(const SpatialVector v) {
    if ( acos(v * a_) < s_ ) return true;
    return false;
}

/////////////INVERT///////////////////////////////////////
//
void
SpatialConstraint::invert() {
  d_ = -d_;
  s_ = acos(d_);
  if(sign_ == nEG) sign_ = pOS;
  if(sign_ == pOS) sign_ = nEG;
}

/////////////READ/////////////////////////////////////////
//
void
SpatialConstraint::read(istream &in) {

  in.setf(ios::skipws);
  while(in.peek() == COMMENT)  // ignore comments
      in.ignore(10000,'\n');
  in >> a_ >> d_ ;
  if(!in.good())
    throw SpatialFailure("SpatialConstraint:read: Could not read constraint");
  a_.normalize();
  s_ = acos(d_);
  if     (d_ <= -gEpsilon) sign_ = nEG;
  else if(d_ >=  gEpsilon) sign_ = pOS;
  else                sign_ = zERO;
}

/////////////READ/////////////////////////////////////////
//
void
SpatialConstraint::readRaDec(istream &in) {

  while(in.peek() == COMMENT)  // ignore comments
      in.ignore(10000,'\n');
  float64 ra,dec;
  in >> ra >> dec >> d_ ; in.ignore();
  a_.set(ra,dec);
  s_ = acos(d_);
  if     (d_ <= -gEpsilon) sign_ = nEG;
  else if(d_ >=  gEpsilon) sign_ = pOS;
  else                sign_ = zERO;
}

/////////////WRITE////////////////////////////////////////
//
void
SpatialConstraint::write(ostream &out) const {
  size_t p = out.precision();
  out.precision(16);
  out << a_ << ' ' << d_ << endl;
  out.precision(p);
}

/////////////>>///////////////////////////////////////////
// read from istream
//
istream& operator >>( istream& in, SpatialConstraint & c) {
  c.read(in);
  return(in);
}

/////////////<<///////////////////////////////////////////
// write to ostream
//
ostream& operator <<( ostream& out, const SpatialConstraint & c) {
  c.write(out);
  return(out);
}
