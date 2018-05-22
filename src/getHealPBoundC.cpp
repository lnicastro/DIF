/*
  Name:  int getHealPBoundC, int getHealPBoundC1

  Description:
   Return the HEALPix pixel boundaries coordinates given scheme
   (RING or NESTED), order and spherical coordinates.
   If step=1 then return the 4 corners (north, west, south and east).
   getHealPBoundC1 always creates a new Healpix_Base base and destroys
   it on exit.

  Parameters:
 ( (i) char*& saved: if not NULL then re-use existing Healpix_Base )
   (i) int nested: Scheme ID; if 0 then RING else NESTED
   (i) int k:      Resolution order in the range [0, 29]
   (i) double ra:  Right Ascension (degrees)
   (i) double dec: Declination (degrees)
   (i) int step:   pixel side oversampling factor

   (o) vector<double> &b_ra:  HEALPix pixel border RAs (degrees)
   (o) vector<double> &b_dec: HEALPix pixel border DECs (degrees)

  Note:
   Scheme ID is not necessary, but keep it as a cross-check.
   Use Healpix_Base version 3.

  Return 0 on success.


  LN@IASF-INAF, May 2016                      Last change: 04/08/2016
*/

#include "arr.h"
#include "healpix_base.h"

/* degrees to radians */
static const double DEG2RAD = 1.74532925199432957692369E-2;

/* radians to degrees */
static const double RAD2DEG = 57.2957795130823208767981548;

void cleanHealPUval(char*& saved);

using namespace std;


int getHealPBoundC(char*& saved, int nested, int k, double ra, double dec, unsigned int step,
                 vector<double> &b_ra, vector<double> &b_dec)
{
  long long int my_nside;

  if ((k < 0) || (k > 29))
    return -1;

  my_nside = 1 << k;

  T_Healpix_Base<int64>* base;

  b_ra.resize(4*step);
  b_dec.resize(4*step);

  if (! saved) {

    if (nested) 
      base = new T_Healpix_Base<int64>(my_nside, NEST, SET_NSIDE);
    else
      base = new T_Healpix_Base<int64>(my_nside, RING, SET_NSIDE);

    saved = (char*) base;

  } else
    base = (T_Healpix_Base<int64>*) saved;

  try {

  long long int id;
  double theta = (90. - dec)*DEG2RAD, phi = ra*DEG2RAD;

  (theta < 0.01 || theta > 3.14159-0.01) ?
    id = base->loc2pix(cos(theta), phi, sin(theta), true) :
    id = base->loc2pix(cos(theta), phi, 0., false);

  int ix, iy, face;
  base->pix2xyf(id, ix, iy, face);
  double dc = 0.5 / my_nside;
  double xc = (ix + 0.5)/my_nside, yc = (iy + 0.5)/my_nside;
  double d = 1.0/(step*my_nside);
  for (unsigned int i=0; i<step; ++i)
  {
    double sth;
    bool have_sth;
    base->xyf2loc(xc+dc-i*d, yc+dc, face, b_dec[i], b_ra[i], sth, have_sth);
    b_ra[i] *= RAD2DEG;
    have_sth ? b_dec[i] = 90. - atan2(sth,b_dec[i])*RAD2DEG :
               b_dec[i] = asin(b_dec[i])*RAD2DEG;

    base->xyf2loc(xc-dc, yc+dc-i*d, face, b_dec[i+step], b_ra[i+step], sth, have_sth);
    b_ra[i+step] *= RAD2DEG;
    have_sth ? b_dec[i+step] = 90. - atan2(sth,b_dec[i+step])*RAD2DEG :
               b_dec[i+step] = asin(b_dec[i+step])*RAD2DEG;

    base->xyf2loc(xc-dc+i*d, yc-dc, face, b_dec[i+2*step], b_ra[i+2*step], sth, have_sth);
    b_ra[i+2*step] *= RAD2DEG;
    have_sth ? b_dec[i+2*step] = 90. - atan2(sth,b_dec[i+2*step])*RAD2DEG :
               b_dec[i+2*step] = asin(b_dec[i+2*step])*RAD2DEG;

    base->xyf2loc(xc+dc, yc-dc+i*d, face, b_dec[i+3*step], b_ra[i+3*step], sth, have_sth);
    b_ra[i+3*step] *= RAD2DEG;
    have_sth ? b_dec[i+3*step] = 90. - atan2(sth,b_dec[i+3*step])*RAD2DEG :
               b_dec[i+3*step] = asin(b_dec[i+3*step])*RAD2DEG;
  }

  return 0;

  }
  catch (std::exception &e) {
    cout <<"Error executing getHealPBoundC. std::exception: "<< e.what() << std::endl;
    return -3;
  }

}


int getHealPBoundC1(int nested, int k, double ra, double dec, unsigned int step,
                  vector<double> &b_ra, vector<double> &b_dec)
{
  char* saved = NULL;
  int ret = getHealPBoundC(saved, nested, k, ra, dec, step, b_ra, b_dec);
  cleanHealPUval(saved);
  return ret;
}
