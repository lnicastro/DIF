/*
  Name:  double getHTMBaryDist, double getHTMBaryDist1

  Description:
   Return the distance from the HTM trixel barycenter given depth, pixel ID
   and coordinates.
   getHTMBaryDist1 always creates a new SpatialIndex class instance and destroys
   it on exit.

  Parameters:
 ( (i) char*& saved: if not NULL then re-use existing SpatialIndex )
   (i) int depth:   Pixelization depth level in the range [0, 25]
   (i) unsigned long long int id: Pixel ID
   (i) float64 ra:  Right Ascension (degrees)
   (i) float64 dec: Declination (degrees)

  Note:
    If depth not in the allowed range then return -1.


  LN@IASF-INAF, October 2008                      Last change: 15/10/2015
*/

using namespace std;

#include <vector>
#include "SpatialInterface.h"

/* degrees to radians */
static const double DEG2RAD = 1.74532925199432957692369E-2;

void cleanHTMUval(char*& saved);


double getHTMBaryDist(char*& saved, int depth, unsigned long long int id,
                      float64 ra, float64 dec)
{

  double bc_ra, bc_dec;

// Depth in allowed range
  if ((depth < 0) || (depth > 25))
    return -1;

// ID in allowed range
  long long unsigned int npix = (1 << (2*depth+3));
  if (id < npix || id > 2*npix-1)
    return -2;

  SpatialIndex *index;

  try {
    if (! saved) {
      index = new SpatialIndex(depth);
      saved = (char*) index;

    } else
      index = (SpatialIndex*) saved;

// Vertices of central trixel
    static ValVec<SpatialVector> v;
    v.at(3);

    index->nodeVertex(id, v[0], v[1], v[2]);

// Centroid
    bc_ra = (v[0].ra() + v[1].ra() + v[2].ra()) / 3. * DEG2RAD;
    bc_dec = (v[0].dec() + v[1].dec() + v[2].dec()) / 3. * DEG2RAD;

// Calc. distance
    double radif = fabs(bc_ra-ra*DEG2RAD);
    if (radif > M_PI) radif = M_PI*2 - radif;
    double sin2a = sin(radif/2.);
    sin2a *= sin2a;
    double sin2d = sin((bc_dec-dec*DEG2RAD)/2.);
    sin2d *= sin2d;

    return (2 * asin( sqrt(sin2d + cos(dec*DEG2RAD)*cos(bc_dec)*sin2a) )/DEG2RAD * 60.);

  }
  catch (SpatialException &x) {
//cout << "Error. SpatialException: " << x.what() << endl;
    return -2;
  }

}


double getHTMBaryDist1(int depth, unsigned long long int id, float64 ra, float64 dec)
{
  char* saved = NULL;
  double d = getHTMBaryDist(saved, depth, id, ra, dec);
  cleanHTMUval(saved);
  return d;
}
