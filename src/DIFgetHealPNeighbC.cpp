/*
  Name:  int DIFgetHealPNeighbC, int DIFgetHealPNeighbC1

  Description:
   Return into the input DIF_Region class the HEALPix IDs of the pixel and
   its neighboring pixels (RING or NESTED) calculated from the input
   spherical coordinates.
   Result contains (tipically) 9 pixel numbers (in this order): central one,
   the SW, W, NW, N, NE, E, SE and S neighbor.
   DIFgetHealPNeighb1C always creates a new Healpix_Base2 base and destroys
   it on exit.
   This is a DIF custom version.

  Parameters:
 ( (i) char*& saved: if not NULL then re-use existing Healpix_Base2 )
   (i) DIF_Region &p: Pointer to the DIF_Region class
   (i) double ra:   Right Ascension (degrees)
   (i) double dec:  Declination (degrees)

  DIFgetHealPNeighbC1:
   (i) DIF_Region &p: Pointer to the DIF_Region class

  Note:
    The W, N, E or S neighbors could not exist therefore its number can be
    less than 9.
    If k is not in the allowed range then return an empty list
    (and not 0 code).
    Use Healpix_Base version 3.

  Return 0 on success.


  LN@IASF-INAF, March 2009                      Last change: 16/05/2016
*/

#include "arr.h"
#include "healpix_base.h"

//static const int MY_NSIDE_DEF = 64;

/* degrees to radians */
static const double DEG2RAD = 1.74532925199432957692369E-2;

void cleanHealPUval(char*& saved);

using namespace std;

#include "dif.hh"


int DIFgetHealPNeighbC(char*& saved, DIF_Region &p, double ra, double dec)
{
  int64 id;
  fix_arr<int64, 8> idn_temp;
  long long int my_nside;
  T_Healpix_Base<int64>* base;

// Not an available order?
  if (p.params.size() == 0)
    return -1;

// Default is RING scheme
  int nested = 0, ik;

// Use the smaller pixel (if more than one order)
//   int k = p.params[0];
  int k = p.params.back();

// Nested?
  if (k < 0) {
    nested = 1;
    ik = -k;
  }

//cout << "Nested: "<< nested << "  Order: " << ik << endl;

  if (! saved) {

// Out of range: return here
    if ( ik > 29 )
      return -2;

    my_nside = 1 << ik;

    if (nested) 
      base = new T_Healpix_Base<int64>(my_nside, NEST, SET_NSIDE);
    else
      base = new T_Healpix_Base<int64>(my_nside, RING, SET_NSIDE);

    saved = (char*) base;

  } else
    base = (T_Healpix_Base<int64>*) saved;


  vector<long long int> &list = p.flist(k);
  list.resize(9);

  //id = base->ang2pix_z_phi(sin(dec*DEG2RAD), ra*DEG2RAD);
  double theta = (90. - dec)*DEG2RAD, phi = ra*DEG2RAD;

  (theta < 0.01 || theta > 3.14159-0.01) ?
    id = base->loc2pix(cos(theta), phi, sin(theta), true) :
    id = base->loc2pix(cos(theta), phi, 0., false);


  base->neighbors(id, idn_temp);

  list[0] = id;
  int j = 1;
  for (int i=0; i<8; i++)
    if (idn_temp[i] >= 0) {
      list[j] = idn_temp[i];
      j++;
    }

  if (j < 9)
    list.resize(j);

#ifdef DEBUG_PRINT
for (i = 0; i < list.size(); i++)
cout <<"ID "<<i<<": "<< list[i] <<endl;
#endif

//#ifdef DEBUG_PRINT
//    cout << "Neighbors IDs:" << endl;
//  for (int i=0; i<9; i++)
//    cout << idn_temp[i] << endl;
//#endif

  return 0;
}


int DIFgetHealPNeighbC1(DIF_Region &p)
{
  char* saved = NULL;
  int ret = DIFgetHealPNeighbC(saved, p, p.ra1, p.de1);
  cleanHealPUval(saved);
  return ret;
}
