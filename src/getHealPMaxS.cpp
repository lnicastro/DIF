/*
  Name:  double getHealPMaxS, double getHealPMaxS1

  Description:
   Return the HEALPix max size (in arcmin) from center to corner
   (RING or NESTED), given the order.
   getHealPMaxS1 always creates a new Healpix_Base2 base and destroys
   it on exit.

  Parameters:
 ( (i) char*& saved: if not NULL then re-use existing Healpix_Base2 )
   (i) int k:            Resolution level of map in the range [0, 29]


  Note:
    Use Healpix_Base version 3.

  Return 0 on success.


  LN@IASF-INAF, June 2013                      Last change: 16/05/2016
*/

#include "arr.h"
#include "healpix_base.h"

/* degrees to radians */
static const double RAD2DEG = 57.295779513082320876798154814105170332405472466564;

void cleanHealPUval(char*& saved);

using namespace std;


double getHealPMaxS(char*& saved, int k)
{
  long long int my_nside;
  T_Healpix_Base<int64>* base;

  if (! saved) {

    if ((k < 0) || (k > 29))
      return -1;

    my_nside = 1 << k;

    base = new T_Healpix_Base<int64>(my_nside, NEST, SET_NSIDE);

    saved = (char*) base;

  } else
    base = (T_Healpix_Base<int64>*) saved;

  return base->max_pixrad() * RAD2DEG * 60.;
}


double getHealPMaxS1(int k)
{
  char* saved = NULL;
  double maxs = getHealPMaxS(saved, k);
  cleanHealPUval(saved);
  return maxs;
}
