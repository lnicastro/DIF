/*
  Name:  int getHTMBaryC, int getHTMBaryC1

  Description:
   Return the HTM trixel barycenter coordinates given depth and
   spherical coordinates.
   getHTMBaryC1 always creates a new SpatialIndex class instance and destroys
   it on exit.

  Parameters:
 ( (i) char*& saved: if not NULL then re-use existing SpatialIndex )
   (i) int depth:   Pixelization depth level in the range [0, 25]
   (i) float64 ra:  Right Ascension (degrees)
   (i) float64 dec: Declination (degrees)

   (o) double *bc_ra:  HTM trixel barycenter RA (degrees)
   (o) double *bc_dec: HTM trixel barycenter Dec (degrees)

  Return 0 on success.


  LN@IASF-INAF, October 2008                      Last change: 23/04/2009
*/

using namespace std;

#include <vector>
#include "SpatialInterface.h"

/* degrees to radians */
//static const double DEG2RAD = 1.74532925199432957692369E-2;

void cleanHTMUval(char*& saved);


int getHTMBaryC(char*& saved, int depth, float64 ra, float64 dec,
                double *bc_ra, double *bc_dec)
{

  long long int id;
  double wrap_add = 0.;

// Depth in allowed range
  if ((depth < 0) || (depth > 25))
    return -1;

  SpatialIndex *index;

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

    index->nodeVertex(id, v[0], v[1], v[2]);

// Must note the 360 deg wrap! Note that the poles are singular points.
    if (v[0].ra() == 0.) {

// Away from the poles
      if (v[1].ra() > 270. || v[2].ra() > 270.) {
        wrap_add = 360.;
        if (v[1].ra() == 0. || v[2].ra() == 0.)
          wrap_add = 720.;

// Near the poles
      } else {

        if (v[0].dec() == 90.) {  // North
          //if (v[1].ra() == 270. && v[2].ra() == 0.) { // 4th quadrant
          if (v[1].ra() == 270.) { // 4th quadrant
            wrap_add = 675.;
          //} else if (v[1].ra() == 180. && v[2].ra() == 270.) { // 3rd quadrant
          } else if (v[1].ra() == 180.) { // 3rd quadrant
            wrap_add = 225.;
          } else if (v[1].ra() == 90.) { // 2nd quadrant
            wrap_add = 135.;
          } else
            wrap_add = 45.;

        } else if (v[0].dec() == -90.) {  // South
          //if (v[2].ra() == 270. && v[1].ra() == 0.) { // 4th quadrant
          if (v[2].ra() == 270.) { // 4th quadrant
            wrap_add = 675.;
          //} else if (v[2].ra() == 180. && v[1].ra() == 270.) { // 3rd quadrant
          } else if (v[2].ra() == 180.) { // 3rd quadrant
            wrap_add = 225.;
          } else if (v[2].ra() == 90.) { // 2nd quadrant
            wrap_add = 135.;
          } else
            wrap_add = 45.;
        }

      }

// Away from the poles
    } else if (v[1].ra() == 0.) {
      if (v[0].ra() > 270. || v[2].ra() > 270.) {
        wrap_add = 360.;
        if (v[0].ra() == 0. || v[2].ra() == 0.)
          wrap_add = 720.;
      }

    } else if (v[2].ra() == 0.) {
      if (v[0].ra() > 270. || v[1].ra() > 270.) {
        wrap_add = 360.;
        if (v[0].ra() == 0. || v[1].ra() == 0.)
          wrap_add = 720.;
      }
    }

// Centroid
    *bc_ra = (v[0].ra() + v[1].ra() + v[2].ra() + wrap_add) / 3.;
    *bc_dec = (v[0].dec() + v[1].dec() + v[2].dec()) / 3.;

#ifdef DEBUG_PRINT

SpatialVector w = v[1] + v[2];  w.normalize();
SpatialVector bc;

bc = v[0] + w; bc.normalize();
bc.show();
cout << "w0: RA, Dec: " << bc.ra() << "  " << bc.dec() << endl;
w = v[0] + v[2];  w.normalize();
bc = v[1] + w; bc.normalize();
bc.show();
cout << "w1: RA, Dec: " << bc.ra() << "  " << bc.dec() << endl;
w = v[0] + v[1];  w.normalize();
bc = v[2] + w; bc.normalize();
bc.show();
cout << "w2: RA, Dec: " << bc.ra() << "  " << bc.dec() << endl;


bc.set(*bc_ra, *bc_dec);
bc.normalize();
bc.show();
cout << "bc: RA, Dec: " << bc.ra() << "  " << bc.dec() << endl;

#endif

  }
  catch (SpatialException &x) {
//cout << "Error. SpatialException: " << x.what() << endl;
    return -2;
  }

  return 0;
}


int getHTMBaryC1(int depth, float64 ra, float64 dec,
                 double *bc_ra, double *bc_dec)
{
  char* saved = NULL;
  int ret = getHTMBaryC(saved, depth, ra, dec, bc_ra, bc_dec);
  cleanHTMUval(saved);
  return ret;
}
