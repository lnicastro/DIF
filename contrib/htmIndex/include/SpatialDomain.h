#ifndef _SpatialDomain_h
#define _SpatialDomain_h
//#     Filename:       SpatialDomain.h
//#
//#     Classes defined here: SpatialDomain
//#
//#
//#     Author:         Peter Z. Kunszt
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

// Modified: L. Nicastro @ IASF-INAF, 25/02/2009

// LN add
#include <vector>

#include "SpatialConvex.h"
#include "BitList.h"

//########################################################################
//
// Spatial Domain class
//
//

/** A spatial domain is a list of spatial convexes. So we can have
 really disjoint pieces of the sky defined by a domain.  */

class LINKAGE SpatialDomain {
public:
  /// Constructor
  SpatialDomain(const SpatialIndex * idx = 0);

  /// Destructor
  ~SpatialDomain();

  /// Set index pointer
  void setIndex(const SpatialIndex *);

  /// Add a convex
  void add(SpatialConvex &);

  /// Simplify the Domain, remove redundancies
  void simplify();

  /** Intersect with index.
      Return the bitlist of the leafnodes that are
      partially and fully intersected by this domain. */
  bool intersect(const SpatialIndex * idx,
		 BitList & partial, BitList & full);


  /// Same intersection, but return vectors of ids instead of bitlists.
  bool intersect(const SpatialIndex * idx,
		 ValVec<uint64> & partial, ValVec<uint64> & full);

//LN add
  bool intersect(const SpatialIndex * idx, vector <int> depths,
                 vector<long long int> & nid_partial,
                 vector<long long int> & nid_full);
  bool intersect(const SpatialIndex * idx, vector <int> depths,
                 vector<long long int> & nid_depths,
		 ValVec<uint64> & partial, ValVec<uint64> & full);

  /// Same intersection, but return just a list of IDs not level depth
  bool intersect(const SpatialIndex * idx, ValVec<uint64> & idlist);

  /// numConvexes: give back the number of convexes
  size_t numConvexes();

  /// [] operator: give back convex
  SpatialConvex & operator [](size_t i);

  /// read from stream
  void read(istream&);

  /// write to stream
  void write(ostream&) const;

  const SpatialIndex * index; 		/// A pointer to the index

  static void ignoreCrLf(istream &);
protected:
  ValVec<SpatialConvex> convexes_;      /// The vector of convexes

public:
  static uint64 topBit_;
};

#include "SpatialDomain.hxx"
#endif
