/*
  Name:  int getHealPBaryC, int getHealPBaryC1

  Description:
   Return the HEALPix pixel barycentre (center) coordinates given scheme
   (RING or NESTED), order and spherical coordinates.
   getHealPBaryC1 always creates a new Healpix_Base base and destroys
   it on exit.

  Parameters:
 ( (i) char*& saved: if not NULL then re-use existing Healpix_Base2 )
   (i) int nested: Scheme ID; if 0 then RING else NESTED
   (i) int k:      Resolution order in the range [0, 29]
   (i) double ra:  Right Ascension (degrees)
   (i) double dec: Declination (degrees)

   (o) double *bc_ra: HEALPix pixel barycentre RA (degrees)
   (o) double *bc_dec: HEALPix pixel barycentre Dec (degrees)

  Note:
   Scheme ID is not necessary, but keep it as a cross-check.
   Use Healpix_Base version 3.

  Return 0 on success.


  LN@IASF-INAF, September 2008                      Last change: 04/08/2016
*/

#include "arr.h"
#include "healpix_base.h"

/* degrees to radians */
static const double DEG2RAD = 1.74532925199432957692369E-2;

/* radians to degrees */
static const double RAD2DEG = 57.2957795130823208767981548;

void cleanHealPUval(char*& saved);

using namespace std;


int getHealPBaryC(char*& saved, int nested, int k, double ra, double dec,
                  double *bc_ra, double *bc_dec)
{
  long long int my_nside;
  T_Healpix_Base<int64>* base;

  if (! saved) {

    if ((k < 0) || (k > 29))
      return -1;

    my_nside = 1 << k;

    if (nested) 
      base = new T_Healpix_Base<int64>(my_nside, NEST, SET_NSIDE);
    else
      base = new T_Healpix_Base<int64>(my_nside, RING, SET_NSIDE);

    saved = (char*) base;

  } else
    base = (T_Healpix_Base<int64>*) saved;

  long long int id;
  double theta = (90. - dec)*DEG2RAD, phi = ra*DEG2RAD;

  (theta < 0.01 || theta > 3.14159-0.01) ?
    id = base->loc2pix(cos(theta), phi, sin(theta), true) :
    id = base->loc2pix(cos(theta), phi, 0., false);

  double sth;
  bool have_sth;
  base->pix2loc(id, *bc_dec, *bc_ra, sth, have_sth);
  *bc_ra *= RAD2DEG;
  have_sth ? *bc_dec = 90. - atan2(sth,*bc_dec)*RAD2DEG :
             *bc_dec = asin(*bc_dec)*RAD2DEG;


  return 0;
}


int getHealPBaryC1(int nested, int k, double ra, double dec,
                   double *bc_ra, double *bc_dec)
{
  char* saved = NULL;
  int ret = getHealPBaryC(saved, nested, k, ra, dec, bc_ra, bc_dec);
  cleanHealPUval(saved);
  return ret;
}
