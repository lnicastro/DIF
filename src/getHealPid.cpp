/*
  Name:  int getHealPid, int getHealPid1

  Description:
   Returns the pixel ID (RING or NESTED) which contains the given angular
   coordinates.
   getHealPid1 always creates a new Healpix_Base2 base and destroys it on exit.

  Parameters:
 ( (i) char*& saved: if not NULL then re-use existing Healpix_Base2 )
   (i) int nested: Scheme ID; if 0 then RING else NESTED
   (i) int k:      Resolution level of map; if 0 then use 6 -> Nside=64
   (i) double ra:  Right Ascension (degrees)
   (i) double dec: Declination (degrees)

   (o) long long int *id: HEALPix ID

  Note:
    Npix = 12 * Nside^2
    Nside = 2^k, k = [0, ..., 29] -> order = WMAP resolution parameter
    Sky resolution: Omega=PI/(3Nside^2)
    For Nside=64 it is ~1 deg (12 * 64^2 = 49152 pixels).

    If k is not in the allowed range then set ID to 0 and return.

  Return 0 on success.

  25/09/2008: Use Healpix_Base2
  30/09/2008: Use ang2pix_z_phi avoiding pointing structure definition
  16/05/2016: Use Healpix_Base version 3


  LN@IASF-INAF, July 2007                        Last change: 16/05/2016
*/

#include "healpix_base.h"

//static const int MY_NSIDE_DEF = 64;

/* degrees to radians */
static const double DEG2RAD = 1.74532925199432957692369E-2;

using namespace std;


int getHealPid(char*& saved, int nested, int k, double ra, double dec,
               long long int *id)
{
  long long int my_nside;
  T_Healpix_Base<int64>* base;

  if (! saved) {

//    if ((k < 0)  ||  (k > 29))  my_nside = MY_NSIDE_DEF;
    if ((k < 0) || (k > 29)) {
      *id = 0;
      return -1;
    }

    my_nside = 1 << k;

    if (nested) 
      base = new T_Healpix_Base<int64>(my_nside, NEST, SET_NSIDE);
    else
      base = new T_Healpix_Base<int64>(my_nside, RING, SET_NSIDE);

    saved = (char*) base;

  } else
    base = (T_Healpix_Base<int64>*) saved;

  //*id = base->ang2pix_z_phi(sin(dec*DEG2RAD), ra*DEG2RAD);
  double theta = (90. - dec)*DEG2RAD, phi = ra*DEG2RAD;

  (theta < 0.01 || theta > 3.14159-0.01) ?
    *id = base->loc2pix(cos(theta), phi, sin(theta), true) :
    *id = base->loc2pix(cos(theta), phi, 0., false);

//if (!nested) cout << "nested= " << base->ring2nest(*id) << endl;
  
  return 0;
}


void cleanHealPUval(char*& saved)
{
  T_Healpix_Base<int64>* base;

  if (saved) {
    base = (T_Healpix_Base<int64>*) saved;
    delete base;
  }
}


int getHealPid1(int nested, int k, double ra, double dec, long long int *id)
{
  char* saved = NULL;
  int ret = getHealPid(saved, nested, k, ra, dec, id);
  cleanHealPUval(saved);
  return ret;
}
