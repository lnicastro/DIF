/*
  Name:  int DIFgetHTMNeighbC, int DIFgetHTMNeighbC1

  Description:
   Return into the input DIF_Region class the HTM trixel IDs of the pixel and
   its neighboring pixels calculated from the input spherical coordinates.
   Result contains typically 13 trixels (the central one + 12 neighbors sorted
   in ascending order) or 11 trixels for angles near to any mult. of 90 deg.
   DIFgetHTMNeighbC1 always creates a new SpatialIndex class instance and
   destroys it on exit.
   This is a DIF custom version.

  Parameters:
 ( (i) char*& saved: if not NULL then re-use existing SpatialIndex )
   (i) DIF_Region &p: Pointer to the DIF_Region class
   (i) double ra:   Right Ascension (degrees)
   (i) double dec:  Declination (degrees)

  DIFgetHTMNeighbC1:
   (i) DIF_Region &p: Pointer to the DIF_Region class

  Note:
    For coordinates multiple of 90 deg. the number of neighbors can be less
    than 13 (tipically 11).
    If depth is not in the allowed range then return an empty list
    (and not 0 code).

  Return 0 on success.


  LN@IASF-INAF, March 2009                      Last change: 29/01/2010
*/

using namespace std;

#include <vector>
#include "SpatialInterface.h"

#include "dif.hh"

/* degrees to radians */
static const double DEG2RAD = 1.74532925199432957692369E-2;

void cleanHTMUval(char*& saved);


int DIFgetHTMNeighbC(char*& saved, DIF_Region &p, double ra, double dec)
{

  ValVec<uint64> plist, flist;
  long long int id, this_id;
  size_t i;

// No available depth?
//  if (locateParam(depth) < 0)
  if (p.params.size() == 0)
    return -1;

// Use the smallest pixel (if more than one depth)
//  int depth = p.params[0];
  int depth = p.params.back();

//cout <<"DIFgetHTMNeighbC used depth="<< depth<<endl;
// Out of range: return here
  if ((depth < 0) || (depth > 25))
    return -2;

  SpatialIndex *index;

  vector<long long int> &list = p.flist(depth);
  list.resize(13);

  try {
    if (! saved) {
      index = new SpatialIndex(depth);
      saved = (char*) index;

    } else
      index = (SpatialIndex*) saved;

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

//    domain.intersect(&index,plist,flist);
    domain.intersect(index,plist,flist);

//cout << "plist.length: " << plist.length() << endl;

// Array is already sorted so it is easy to remove duplicated entries
// There is at least the trixel identified by input coordinates
//    idn.push_back(id);
    list[0] = id;

    int j = 1;
    for (i=0; i<plist.length(); i++) {
      this_id = plist(i);
      if (this_id != list[j-1] && this_id != id) {
//        idn.push_back(plist(i));
        list[j] = plist(i);
        j++;
      }
    }

// Resize if central trixel falls at peculiar angles (mult. of 90 deg)
   if (j < 13)
     list.resize(j);

#ifdef DEBUG_PRINT
for (i = 0; i < list.size(); i++)
cout <<"ID "<<i<<": "<< list[i] <<endl;
#endif

  }
  catch (SpatialException &x) {
//cout << "Error. SpatialException: " << x.what() << endl;
    return -2;
  }

  return 0;
}


int DIFgetHTMNeighbC1(DIF_Region &p)

{
  char* saved = NULL;
  int ret = DIFgetHTMNeighbC(saved, p, p.ra1, p.de1);

  cleanHTMUval(saved);
  return ret;
}
