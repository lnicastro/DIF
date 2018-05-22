#ifndef _SpatialConstraint_h
#define _SpatialConstraint_h
//#     Filename:       SpatialConstraint.h
//#
//#     Classes defined here: SpatialConstraint SpatialSign
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
//#

#include "SpatialVector.h"

//########################################################################
//#
//# Spatial Sign helper class

/**
   The sign class is inherited by Constraint and Convex. Assignment and
   copy operators are used in both scopes.
*/

class LINKAGE SpatialSign {
public:
  enum Sign {
    nEG,			// All constraints negative or zero
    zERO,			// All constraints zero
    pOS,			// All constraints positive or zero
    mIXED			// At least one pos and one neg
  };

  /// Constructor
  SpatialSign(Sign sign = zERO);

  /// Copy constructor
  SpatialSign(const SpatialSign &);

  /// Assignment
  SpatialSign & operator =(const SpatialSign &);

protected:
  /// Sign value
  Sign sign_;
};

//########################################################################
//#
//# Spatial Constraint class
//#
/**
   The Constraint is really a cone on the sky-sphere. It is characterized
   by its direction a_, the opening angle s_ and its cosine -- the distance
   of the plane intersecting the sphere and the sphere center.
   If d_ = 0, we have a half-sphere. If it is negative, we have a 'hole'
   i.e. the room angle is larger than 90degrees.

   Example: positive distance
<pre>
.                   ____
.                ---    ---
.               /        /|\
.              /        / |=\
.             |        /  |==|     this side is in the convex.
.            |        /\s |===|
.            |------------|---| -> direction a
.            |        \   |===|
.             |        \  |==|
.              \        \ |=/
.               \        \|/
.                ---____---
.
.
.                     <-d-> is positive (s < 90)

</pre>
 Example: negative distance
<pre>
.                   ____
.                ---====---
.  this side is /========/|\
.  in the      /========/=| \
.  convex     |==== s__/==|  |
.            |===== / /===|   |
.  dir. a <- |------------|---|  'hole' in the sphere
.            |========\===|   |
.             |========\==|  |
.              \========\=| /
.               \========\|/
.                ---____---
.
.
.                     <-d-> is negative (s > 90)
</pre>
 for d=0 we have a half-sphere. Combining such, we get triangles, rectangles
 etc on the sphere surface (pure ZERO convexes)

*/

class LINKAGE SpatialConstraint : public SpatialSign {
public:
  /// Constructor
  SpatialConstraint() {};

  /// Initialization constructor
  SpatialConstraint(SpatialVector, float64);

  /// Copy constructor
  SpatialConstraint(const SpatialConstraint &);

  /// Assignment
  SpatialConstraint & operator =(const SpatialConstraint &);

  /// set vector
  void setVector(SpatialVector &);

  /// set distance
  void setDistance(float64);

  /// Invert
  void invert();

  /// check whether a vector is inside this
  bool contains(const SpatialVector v);

  /// give back vector
  SpatialVector & v() ;

  /// give back distance
  float64 d() const ;

  /// read
  void read(istream &in);

  /// read
  void readRaDec(istream &in);

  /// write
  void write(ostream &out) const;

private:
  SpatialVector a_;			// normal vector
  float64       d_;			// distance from origin
  float64       s_;			// cone angle in radians

  friend class SpatialIndex;
  friend class SpatialConvex;
  friend class SpatialDomain;
  friend class sxSpatialDomain;
};

#include "SpatialConstraint.hxx"
#endif
