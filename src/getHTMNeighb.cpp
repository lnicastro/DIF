/*
  Name:  int getHTMNeighb, int getHTMNeighb1

  Description:
   Return the HTM trixel IDs of neighboring pixels of a given pixel ID.
   Result contains typically 12 trixels (sorted in ascending order)
   or 10 trixels for trixels touching any multiple of 90 deg angles.
   getHTMNeighb1 always creates a new SpatialIndex base and destroys it
   on exit.

  Parameters:
 ( (i) char*& saved: if not NULL then re-use existing SpatialIndex )
   (i) int depth:   Pixelization depth level in the range [0, 25]
   (i) unsigned long long int id: Pixel ID

   (o) vector<unsigned long long int>& idn [12]: HTM IDs of neighbors (<= 12)

  Note:
    If depth is not in the allowed range then set all ID to -1 and return.
    For coordinates multiple of 90 deg. the number of neighbors can be less
    than 12 (tipically 10).
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


int getHTMNeighb(char*& saved, int depth, unsigned long long int id,
                 vector<unsigned long long int>& idn)
{

  ValVec<uint64> plist, flist;
//  uint64 id;
  unsigned long long int this_id=0;
  size_t i;

// Depth in allowed range
  if ((depth < 0) || (depth > 25))
    return -1;

// ID in allowed range
  long long unsigned int npix = (1 << (2*depth+3));
  if (id < npix || id > 2*npix-1)
    return -2;

  idn.resize(12);
  SpatialIndex *index;

  try {
    if (! saved) {
      index = new SpatialIndex(depth);
      saved = (char*) index;
//  idn.clear();

    } else
      index = (SpatialIndex*) saved;


// Vertices of central trixel
    ValVec<SpatialVector> v;
    v.at(3);
    ValVec<SpatialConvex> cvx;
    cvx.at(3);

    SpatialConstraint constr;

    index->nodeVertex(id, v[0], v[1], v[2]);

// Cross product
    SpatialVector p = (v[0] ^ v[1]);
//    p.show();

// Modulus -> ~triangle side; divide by 2 for radius
    double r = p.length() / 2.;

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

// Array is already sorted so it is easy to remove duplicated entries
// Do not list input trixel ID
//    idn.push_back(id);
    for (i=0; i<plist.length(); i++) {
      this_id = plist(i);
      if (this_id != id)
        break;
    }

//    idn.push_back(this_id);
    idn[0] = this_id;

    int j = 1;
    for (i=0; i<plist.length(); i++) {
      this_id = plist(i);
      if (this_id != idn[j-1] && this_id != id) {
//        idn.push_back(plist(i));
        idn[j] = this_id;
        j++;
      }
    }

// Resize if central trixel falls at peculiar angles (mult. of 90 deg)
   if (j < 12)
     idn.resize(j);

  }
  catch (SpatialException &x) {
//cout << "Error. SpatialException: " << x.what() << endl;
    return -2;
  }

  return 0;
}


int getHTMNeighb1(int depth, unsigned long long int id,
                   vector<unsigned long long int>& idn)
{
  char* saved = NULL;
  int ret = getHTMNeighb(saved, depth, id, idn);
  cleanHTMUval(saved);
  return ret;
}
