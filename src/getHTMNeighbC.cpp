/*
  Name:  int getHTMNeighbC, int getHTMNeighbC1

  Description:
   Return the HTM trixel IDs of the pixel and its neighboring pixels
   calculated from the input spherical coordinates.
   Result contains typically 13 trixels (the central one + 12 neighbors sorted
   in ascending order) or 11 trixels for angles near to any mult. of 90 deg.
   getHTMNeighbC1 always creates a new SpatialIndex base and destroys it
   on exit.

  Parameters:
 ( (i) char*& saved: if not NULL then re-use existing SpatialIndex )
   (i) int depth:   Pixelization depth level in the range [0, 25]
   (i) float64 ra:  Right Ascension (degrees)
   (i) float64 dec: Declination (degrees)

   (o) vector<unsigned long long int>& idn [13]: HTM IDs of neighbors (<= 13)

  Note:
    For coordinates multiple of 90 deg. the number of neighbors can be less
    than 13 (tipically 11).
    If depth is not in the allowed range then return an empty list
    (and not 0 code).

  Return 0 on success.


  LN@IASF-INAF, October 2008                      Last change: 15/10/2015
*/

using namespace std;

#include <vector>
#include "SpatialInterface.h"

/* degrees to radians */
static const double DEG2RAD = 1.74532925199432957692369E-2;

void cleanHTMUval(char*& saved);


int getHTMNeighbC(char*& saved, int depth, float64 ra, float64 dec,
                  vector<unsigned long long int>& idn)
{

  ValVec<uint64> plist, flist;
//  uint64 id;
  unsigned long long int id, this_id;
  size_t i;

// Depth in allowed range
  if ((depth < 0) || (depth > 25))
    return -1;

  idn.resize(13);

  SpatialIndex *index;

  try {
    if (! saved) {
      index = new SpatialIndex(depth);
      saved = (char*) index;
//  idn.clear();

    } else
      index = (SpatialIndex*) saved;

//    id = htm->lookupID(ra, dec);
    id = index->idByPoint(ra, dec);

// Vertices of central trixel
    static ValVec<SpatialVector> v;
    v.at(3);
    ValVec<SpatialConvex> cvx;
    cvx.at(3);

    static SpatialConstraint constr;

    index->nodeVertex(id, v[0], v[1], v[2]);

// Cross product
    SpatialVector p = (v[0] ^ v[1]);
//    p.show();

// Modulus -> ~triangle side; divide by 2 for radius
    double r = p.length() / 2.;

//cout << "r= " << r/DEG2RAD << endl;

    r = cos( r );

    SpatialDomain domain;    // initialize empty domain

    for (i=0; i<3; i++) {
//cout << i << ": RA, Dec: " << v[i].ra() << "  " << v[i].dec() << endl;
      constr.setVector(v[i]);
      constr.setDistance(r);
      cvx[i].add(constr);
      domain.add(cvx[i]);
    }

    domain.intersect(index,plist,flist);

//cout << "plist.length: " << plist.length() << endl;

// Array is already sorted so it is easy to remove duplicated entries
// There is at least the trixel identified by input coordinates
//    idn.push_back(id);
    idn[0] = id;

    int j = 1;
    for (i=0; i<plist.length(); i++) {
      this_id = plist(i);
      if (this_id != idn[j-1] && this_id != id) {
//        idn.push_back(plist(i));
        idn[j] = plist(i);
        j++;
      }
    }

// Resize if central trixel falls at peculiar angles (mult. of 90 deg)
   if (j < 13)
     idn.resize(j);

  }
  catch (SpatialException &x) {
//cout << "Error. SpatialException: " << x.what() << endl;
    return -2;
  }

  return 0;
}


int getHTMNeighbC1(int depth, float64 ra, float64 dec,
                   vector<unsigned long long int>& idn)
{
  char* saved = NULL;
  int ret = getHTMNeighbC(saved, depth, ra, dec, idn);
  
  cleanHTMUval(saved);
  return ret;
}
