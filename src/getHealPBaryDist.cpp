/*
  Name:  double getHealPBaryDist, double getHealPBaryDist1

  Description:
   Return the distance from the HEALPix pixel barycentre (center) given scheme
   (RING or NESTED), order, pixel ID and coordinates.
   getHealPBaryDist1 always creates a new Healpix_Base base and destroys
   it on exit.
   This is a DIF oriented function.

  Parameters:
   (i) int nested:       Scheme ID; if 0 then RING else NESTED
 ( (i) char*& saved: if not NULL then re-use existing Healpix_Base2 )
   (i) int k:            Resolution level of map in the range [0, 29]
   (i) long long int id: Pixel ID
   (i) double ra:        Right Ascension (degrees)
   (i) double dec:       Declination (degrees)

  Note:
   Use Healpix_Base version 3.

  Return HEALPix pixel barycenter distance (in "arcmin").
  If k not in the allowed range then return -1.


  LN@IASF-INAF, October 2008                      Last change: 16/05/2016
*/

#include "arr.h"
#include "healpix_base.h"

/* degrees to radians */
static const double DEG2RAD = 1.74532925199432957692369E-2;

void cleanHealPUval(char*& saved);

using namespace std;


double getHealPBaryDist(int nested, char*& saved, int k, long long int id,
                        double ra, double dec)
{
  long long int my_nside;
  double z, bc_ra, bc_dec;
  T_Healpix_Base<int64>* base;

  if (! saved) {

    if ((k < 0) || (k > 29))
      return -1.;

    my_nside = 1 << k;

    if (nested) 
      base = new T_Healpix_Base<int64>(my_nside, NEST, SET_NSIDE);
    else
      base = new T_Healpix_Base<int64>(my_nside, RING, SET_NSIDE);

    saved = (char*) base;

  } else
    base = (T_Healpix_Base<int64>*) saved;

  double sth;
  bool have_sth;
  base->pix2loc(id, z, bc_ra, sth, have_sth);
  have_sth ? bc_dec = atan2(sth,z) :
             bc_dec = asin(z);


// Calc. distance
  double radif = fabs(bc_ra-ra*DEG2RAD);
  if (radif > M_PI) radif = M_PI*2 - radif;
  double sin2a = sin(radif/2.);
  sin2a *= sin2a;
  double sin2d = sin((bc_dec-dec*DEG2RAD)/2.);
  sin2d *= sin2d;

  return (2 * asin( sqrt(sin2d + cos(dec*DEG2RAD)*cos(bc_dec)*sin2a) )/DEG2RAD * 60.);
}


double getHealPBaryDist1(int nested, int k, long long int id,
                         double ra, double dec)
{
  char* saved = NULL;
  double d = getHealPBaryDist(nested, saved, k, id, ra, dec);
  cleanHealPUval(saved);
  return d;
}
