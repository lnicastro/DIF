#ifndef _SpatialVector_h
#define _SpatialVector_h
//#     Filename:       SpatialVector.h
//#
//#     Standard 3-d vector class
//#
//#
//#     Author:         Peter Z. Kunszt, based on A. Szalay's code
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
//#
//# LN change for gcc 3.3: 07/12/2003    Last change: 17/11/2018

#include <iostream>
using namespace std;

#include <math.h>
//#include <stdio.h>
#include <iostream>
#include "SpatialGeneral.h"

//########################################################################
/**

   The SpatialVector is a 3D vector usually living on the surface of
   the sphere. The corresponding ra, dec can be obtained if the vector
   has unit length. That can be ensured with the normalize() function.

*/

class LINKAGE SpatialVector {
public:
  /// constructs (1,0,0), ra=0, dec=0.
  SpatialVector();

  /// Constructor from three coordinates, not necessarily normed to 1
  SpatialVector(float64 x,
		float64 y,
		float64 z);

  /// Constructor from ra/dec, this is always normed to 1
  SpatialVector(float64 ra,
		float64 dec);

  /// Copy constructor
  SpatialVector(const SpatialVector &);

  /// Assignment
  SpatialVector& operator =(const SpatialVector &);

  /// Set member function: set values - always normed to 1
  void set(const float64 &x,
	   const float64 &y,
	   const float64 &z);

  /// Set member function: set values - always normed to 1
  void set(const float64 &ra,
	   const float64 &dec);

  /// Get x,y,z
  void get( float64 &x,
	    float64 &y,
	    float64 &z) const;

  /// Get ra,dec - normalizes x,y,z
  void get( float64 &ra,
	    float64 &dec);

  /// return length of vector
  float64 length() const;

  /// return x (only as rvalue)
  float64 x() const;

  /// return y
  float64 y() const;

  /// return z
  float64 z() const;

  /// return ra - this norms the vector to 1 if not already done so
  float64 ra();

  /// return dec - this norms the vector to 1 if not already done so
  float64 dec();

  /// Normalize vector length to 1
  void normalize();

  /// Printf it to stdout
  void show() const;

  /// Read vector from a stream
  void read(istream &);

  /// Write vector to a stream
  void write(ostream &) const;

  /// Comparison
  int operator ==(const SpatialVector & ) const;

  /// dot product
  float64 operator *(const SpatialVector & ) const;

  /// cross product
  SpatialVector operator ^(const SpatialVector & ) const;

  /// addition
  SpatialVector operator +(const SpatialVector & ) const;

  /// subtraction
  SpatialVector operator -(const SpatialVector & ) const;

  /**@name Scalar products with int and float */
  //@{
  /**@name operator *= */
  SpatialVector & operator *=(float64);
  SpatialVector & operator *=(int);
  friend SpatialVector operator *(float64, const SpatialVector &);
  friend SpatialVector operator *(int,     const SpatialVector &);
  friend SpatialVector operator *(const SpatialVector &, float64);
  friend SpatialVector operator *(const SpatialVector &, int);
  //@}

private:
  float64 x_;
  float64 y_;
  float64 z_;
  float64 ra_;
  float64 dec_;
  bool okRaDec_;

  void updateXYZ();
  void updateRaDec();

  friend class SpatialIndex;
  friend class SpatialDomain;
  friend class sxSpatialDomain;
};

#include "SpatialVector.hxx"

#endif

