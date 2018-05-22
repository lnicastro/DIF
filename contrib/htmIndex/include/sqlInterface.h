#ifndef _sqlInterface_h
#define _sqlInterface_h
//#     Filename:       SpatialInterface.h
//#
//#     Interfaceion interface
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
#include "VarVec.h"
#include "SpatialInterface.h"

typedef uint64 HTM_ID;
typedef ValVec<htmRange> ValueVector;
typedef VarStr MsgStr;

/**
   htmSqlInterface class.
   Interface to SQL server.

*/

class LINKAGE htmSqlInterface {
public:

  /** Constructor. */
  htmSqlInterface(size_t depth = 0);
  /** Lookup a node ID from a string command.
      The string in the input may have one of the following forms:
      <ul>
      <li> "J2000 depth ra dec"
      <li> "CARTESIAN depth x y z"
      <li> "NAME name"
      </ul>
      The string will be evaluated depending on how many items it has.
  */
  HTM_ID lookupID(char *);

  /** Check the lookupID syntax */
  MsgStr lookupIDDiagnostic(char *);


  /** Intersect function 1. The string passed to this command
      can be any of the strings accepted by the circleRegion,
      convexHull or domain methods. The circleRegion string
      needs to have a prefix CIRCLE and the convexHull string
      needs to have a prefix CONVEX to be recognized.
      <p>So the accepted strings are
      <ul>
      <li> "CIRCLE J2000 depth ra dec rad"
      <li> "CIRCLE CARTESIAN depth x y z rad"
      <li> "CONVEX J2000 depth ra dec ra dec ra dec "
      <li> "CONVEX CARTESIAN depth x y z x y z x y z "
      <li> the same domain string as in the domain() method
      </ul>
  */
  size_t intersect1( char*, ValueVector & );

  /** Intersect function 2. This only differs from the version
      above that the depth parameter is omitted. The constructor
      has to be given the depth at which the intersection will
      operate.
      <p>So the accepted strings are
      <ul>
      <li> "CIRCLE J2000 ra dec rad"
      <li> "CIRCLE CARTESIAN x y z rad"
      <li> "CONVEX J2000 ra dec ra dec ra dec "
      <li> "CONVEX CARTESIAN x y z x y z x y z "
      <li> the same domain string as in the domain() method except
           for the depth parameter.
      </ul>
  */
  size_t intersect2( char*, ValueVector & );

  /** Request all triangles in a circular region.
      Given are the center coordinate and radius in arcminutes.
	  The Vector to fill and its size are given back. The size
	  is also the return value.
      <ul>
      <li> "J2000 depth ra dec rad"
      <li> "CARTESIAN depth x y z rad"
      </ul>
  */
  size_t  circleRegion( char *, ValueVector & );

  /** Check the circleRegion syntax */
  MsgStr circleRegionDiagnostic(char *);

  /** Request all triangles in the convex hull of a given set of
      points.
      The points are given in the string in the following form:
      <pre>
      " J2000 depth ra dec ra dec ra dec "
      </pre>
      or
      <pre>
      " CARTESIAN depth x y z x y z x y z "
      " J2000 depth ra dec ra dec ra dec "
      </pre>
      or
      <pre>
      " CARTESIAN depth x y z x y z x y z "
      </pre>
      There may be as many points ra, dec or x,y,z as you want.

	  <p> The vector to be filled will be cleared by the routine.
  */
  size_t convexHull( char *str, ValueVector & );

  /** Check convexHull Syntax */
  MsgStr convexHullDiagnostic(char *);


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
  */
  size_t domain( char *str, ValueVector & );

  /** Check the domain syntax */
  MsgStr domainDiagnostic(char *);

  /** Test whether the call gave an error */
  bool err();

  /** Retrieve error message */
  MsgStr error();

private:
  int parseKey( char *str, VarStr &key );

	enum Result {
		nONE,
		lOOKUP,
		cIRCLE,
		dOMAIN,
		cHULL
	};
	htmInterface htm_;
	VarStr error_;
	bool err_;
	Result result_;
	const ValVec<htmRange> *resVec_;
	HTM_ID resID_;
        VarStr depth_;
};

#include "sqlInterface.hxx"
#endif

