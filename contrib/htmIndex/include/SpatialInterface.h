#ifndef _SpatialInterface_h
#define _SpatialInterface_h
//#     Filename:       SpatialInterface.h
//#
//#     Interfaceion interface
//#
//#
//#     Author:         Peter Z. Kunszt
//#
//#     Date:           August 31, 2000
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
#include "SpatialIndex.h"
#include "SpatialDomain.h"
#include "VarVec.h"
#include "VarStr.h"

struct htmRange {
  uint64 lo;
  uint64 hi;
};

struct htmPolyCorner {
  SpatialVector c_;
  bool inside_;
  bool replace_;
};


/**
   htmInterface class.
   The SpatialInterface class contains all methods to interface the
   HTM index with external applications.

*/

class LINKAGE htmInterface {
public:

  /** Constructor. The depth is optional, defaulting to level 5. It
      can be changed with the changeDepth() memberfunction or it can
      be specified with one of the string command interfaces. The
      saveDepth parameter can be specified to keep the given amount of
      levels in memory. This can also be altered by changeDepth. */
  htmInterface(size_t depth = 5, size_t saveDepth = 2);

  /** Destructor. */
  ~htmInterface();

  /** Access the index associated with the interface */
  const SpatialIndex & index() const;

  /** Lookup a node ID from ra,dec.
      Given a certain RA,DEC and index depth return its HTM ID.
  */
  uint64 lookupID(float64 ra, float64 dec) const;

  /** Lookup a node ID from x,y,z.
      Given a certain cartesian vector x,y,z and index depth return its HTM ID.
  */
  uint64 lookupID(float64 x, float64 y, float64 z) const;

  /** Lookup the ID from the Name string.
  */
  uint64 lookupID(char *) const;

  /** Lookup a node ID from a string command.
      The string in the input may have one of the following forms:
      <ul>
      <li> "J2000 depth ra dec"
      <li> "CARTESIAN depth x y z"
      <li> "NAME name"
      </ul>
      The string will be evaluated depending on how many items it has.
      SpatialInterfaceError is thrown if the string is unexpected.
  */
  uint64 lookupIDCmd(char *);

  /** Lookup a node name from ra,dec
      Given a certain RA,DEC and index depth return its HTM NodeName.
  */
  const char * lookupName(float64 ra, float64 dec) ;

  /** Lookup a node name from x,y,z.
      Given a certain cartesian vector x,y,z and index depth return its
      HTM NodeName.
  */
  const char * lookupName(float64 x, float64 y, float64 z) ;

  /** Lookup a node name from a node ID.
  */
  const char * lookupName(uint64 ID) ;

  /** Lookup a node name using a string command.
      The string in the input may have one of the following forms:
      <ul>
      <li> "J2000 depth ra dec"
      <li> "CARTESIAN depth x y z"
      <li> "ID id"
      </ul>
      The string will be evaluated depending on how many items it has.
      SpatialInterfaceError is thrown if the string is unexpected.
  */
  const char * lookupNameCmd(char *);

  /** Request all triangles in a circular region.
      Given are the center coordinate and radius in arcminutes.
  */
  const ValVec<htmRange> & circleRegion( float64 ra,
					 float64 dec,
					 float64 rad ) ;

  /** Request all triangles in a circular region.
      Given are the center coordinate and radius in arcminutes.
  */
  const ValVec<htmRange> & circleRegion( float64 x,
					 float64 y,
					 float64 z,
					 float64 rad ) ;

  /** Request all triangles in a circular region.
      Given are the center coordinate and radius in arcminutes.
      Same as previous two functions but from a string.
  */
  const ValVec<htmRange> & circleRegionCmd( char *str );

  /** Request all triangles in the convex hull of a given set of
      points.
  */
  const ValVec<htmRange> & convexHull( ValVec<float64> ra,
				       ValVec<float64> dec ) ;

  /** Request all triangles in the convex hull of a given set of
      points.
  */
  const ValVec<htmRange> & convexHull( ValVec<float64> x,
				       ValVec<float64> y,
				       ValVec<float64> z ) ;

  /** Request all triangles in the convex hull of a given set of
      points.
      The points are given in the string in the following form:
      <pre>
      " J2000 depth ra dec ra dec ra dec "
      </pre>
      or
      <pre>
      " CARTESIAN depth x y z x y z x y z "
      </pre>
      There may be as many points ra, dec or x,y,z as you want.
  */
  const ValVec<htmRange> & convexHullCmd( char *str );


  /** Give the ranges for an intersection with a proper domain. */
  const ValVec<htmRange> & domain( SpatialDomain & );

  /** String interface for domain intersection.
      The domain should be given in the following form:
      <pre>
      DOMAIN depth
      nConvexes
      nConstraints in convex 1
      x y z d
      x y z d
      .
      .
      x y z d
      nConstraints in convex 2
      x y z d
      x y z d
      .
      .
      x y z d
      .
      .
      .
      nConstrinats in convex n
      x y z d
      x y z d
      .
      .
      x y z d
      <pre>

      <p>
      The numbers need to be separated by whitespace (newlines are allowed).
      Throws SpatialInterfaceError on syntax errors.
  */
  const ValVec<htmRange> & domainCmd( char *str );

  /** Change the current index depth */
  void changeDepth(size_t depth, size_t saveDepth = 2);

  /** Check whether a varstring is an integer */
  static bool isInteger(const VarStr &);

  /** Check whether a varstring is a float */
  static bool isFloat(const VarStr &);

  /** Check whether a range contains a certain id */
  static bool inRange( const ValVec<htmRange> &, uint64 );

  /** Print the ranges to cout */
  static void printRange( const ValVec<htmRange> & );
private:

  enum cmdCode {
    J2000,
    CARTESIAN,
    NAME,
    ID,
    HTMDOMAIN
  };

  char name_[HTMNAMEMAX];
  SpatialIndex *index_;
  ValVec<htmRange> range_;
  ValVec<uint64> idList_;
  ValVec<htmPolyCorner> polyCorners_;
  VarStr cmd_;
  VarStrToken *t_;

  // parse command code
  cmdCode getCode();

  // parse depth
  void getDepth();

  // parse the string, returning the depth
  bool parseVec( cmdCode, float64 *v );

  int32   getInteger();   // get an int off the command string
  uint64  getInt64();     // get an int off the command string
  float64 getFloat();     // get a float off the command string

  // add a polygon corner to the list, sort it counterclockwise
  // and ignore if inside the convex hull
  void setPolyCorner(SpatialVector &v);

  // this routine does the work for all convexHull calls
  const ValVec<htmRange> & doHull();

  // generate ranges from idlist
  void makeRange();

};

#include "SpatialInterface.hxx"
#endif

