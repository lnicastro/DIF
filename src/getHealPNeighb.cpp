/*
  Name:  int getHealPNeighb, int getHealPNeighb1

  Description:
   Return the HEALPix IDs of neighboring pixels of a given pixel ID
   (RING or NESTED).
   Result contains (typically) 8 pixel numbers of the (in this order)
   SW, W, NW, N, NE, E, SE and S neighbor.
   getHealPNeighb1 always creates a new Healpix_Base base and destroys
   it on exit.

  Parameters:
 ( (i) char*& saved: if not NULL then re-use existing Healpix_Base )
   (i) int nested:       Scheme ID; if 0 then RING else NESTED
   (i) int k:            Resolution order in the range [0, 29]
   (i) long long int id: Pixel ID

   (o) vector<long long int>& idn [8]: HEALPix IDs of neighbors (<= 8)

  Note:
   The W, N, E or S neighbors could not exist therefore its number can be
   less than 8.
   If k is not in the allowed range then return an empty list
   (and not 0 code).
   Use Healpix_Base version 3.

  Return 0 on success.


  LN@IASF-INAF, September 2008                      Last change: 04/08/2016
*/

#include "arr.h"
#include "healpix_base.h"

//static const int MY_NSIDE_DEF = 64;

/* degrees to radians */
static const double DEG2RAD = 1.74532925199432957692369E-2;

void cleanHealPUval(char*& saved);

using namespace std;


int getHealPNeighb(char*& saved, int nested, int k, long long int id,
                   vector<long long int>& idn)
{
  int64 id_temp = id;
  fix_arr<int64, 8> idn_temp;
  long long int my_nside;
  T_Healpix_Base<int64>* base;

  idn.clear();

  if (! saved) {

//    if ((k < 0)  ||  (k > 29))  my_nside = MY_NSIDE_DEF;
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

  base->neighbors(id_temp, idn_temp);

  for (int i=0; i<8; i++)
    if (idn_temp[i] >= 0)
      idn.push_back(idn_temp[i]);

//#ifdef DEBUG_PRINT
//  cout << "Neighbors IDs:" << endl;
//  for (unsigned int i=0; i<idn.size(); i++)
//    cout << idn[i] << endl;
//#endif

  return 0;
}


int getHealPNeighb1(int nested, int k, long long int id,
                    vector<long long int>& idn)
{
  char* saved = NULL;
  int ret = getHealPNeighb(saved, nested, k, id, idn);
  cleanHealPUval(saved);
  return ret;
}
