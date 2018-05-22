#ifndef _SpatialEdge_h
#define _SpatialEdge_h
//#     Filename:       SpatialEdge.h
//#
//#     SpatialEdge is a helper class for the spatial index at construction
//#     time.
//#
//#
//#     Author:         Peter Z. Kunszt, based on A. Szalay's code
//#
//#     Date:           October 15, 1998
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

#include "SpatialIndex.h"

// Forward declarations
class SpatialIndex;

//########################################################################
//
// <GROUP>
// <SUMMARY>Class declarations</SUMMARY>
//

//########################################################################
//
// <SUMMARY> Spatial Edge class </SUMMARY>
//
// The Edges are needed at construction time of the spatial index.
// They are used to generate the midpoints of the nodes in a certain layer.
// The interface is simple: construct a class giving it the SpatialIndex
// and the layer number. Then call makeMidPoints. The SpatialIndex will
// then have its midpoint constructed in every QuadNode.

class LINKAGE SpatialEdge {
public:
  // Constructor : give the tree and its layer
  SpatialEdge(SpatialIndex & tree, size_t layerindex);

  // Destructor
  ~SpatialEdge();

  // Interface to class: generate midpoints.
  void makeMidPoints();

private:
  struct Edge {
    size_t	start_;		// starting vertex index of edge
    size_t	end_;		// index of end
    size_t 	mid_;		// index of center
  };

  // Make a new edge, in the temporary edges_ at emindex, at node_[index]
  // using the k'th side. Since every edge belongs to two faces, we have]
  // to check wether an edge has been already processed or not (i.e. the
  // midpoint has been constructed or not). We have a lookup table for
  // this purpose. Every edge is stored at lTab[start_]. There may be
  // up to 6 edges in every vertex[start_] so if that table place is occupied,
  // store it in the next table position (and so on). So we only have to
  // look up 6 positions at most.
  size_t newEdge(size_t emindex, size_t index, int k);

  // insert the edge em into the lookup table
  void insertLookup(Edge *em);

  // lookup the edge em in the lookup table
  Edge * edgeMatch(Edge *em);

  // generate a new vertex, which is the midpoint of the current edge.
  size_t getMidPoint(Edge * em);

  SpatialIndex &	tree_;		// reference to the tree class
  size_t		layerindex_;	// index of the layer
  Edge ** 		lTab_;		// Edges lookup table
  Edge *  		edges_;		// Edges array
  size_t		index_;		// index of the vertex that is built
};

// </GROUP>
//

#endif
