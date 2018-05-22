/*
  Name:  int DIFmyHealPCone

  Description:
   Calculates full and partial pixel intersected by a cone in the HEALPix 
   RING or NESTED sheme.
   This is a DIF custom version.

  Parameters:
   (i) DIF_Region &p:  Pointer to the DIF_Region class

  Note:
   Npix = 12 * Nside^2
   Nside = 2^k, k = [0, ..., 29] -> order = WMAP resolution parameter
   Sky resolution: Omega=PI/(3Nside^2)
   For Nside=64 it is ~1 deg (12 * 64^2 = 49152 pixels).

   The minimum radius is TBD (1 mas assumed). See Healpix_Base::query_disc and pix2loc.

   Returned list of partial pixels could be > real ones, that of full pixels
   could be < real ones.
   If k is not in the allowed range then return empty lists (and not 0 code).

  Return 0 on success.

  16/05/2016: Use Healpix_Base version 3
  05/07/2016: Correct DIF_HEALP_RING setting to reflect udf.cc fix


  LN@IASF-INAF, March 2009                        Last change: 07/07/2016
*/

#include <algorithm>


/* degrees to radians */
static const double DEG2RAD = 1.74532925199432957692369E-2;

/* Minimum cone radius (rad): ~ 1 mas - this is function of K and "fact" in "query_disc_inclusive" - TBC */
static const double MIN_CONE_RAD = 5e-9;

#include "arr.h"
#include "geom_utils.h"
#include "healpix_base.h"

using namespace std;

#include "dif.hh"


extern void difflist_i(vector<long long int>& v1,
                       vector<long long int>& v2,
                       vector<long long int>& vdiff);


int DIFmyHealPCone(DIF_Region &p)
{

// No available order: return
  if (p.params.size() == 0)
    return -1;

  int64 my_nside;
  vector<int64> tmp_list;
  vector<long long int> all;
  unsigned long long int j;

  double ra = p.ra1;
  double dec = p.de1;
  double radius = p.rad;

// Default is NESTED scheme
  int nested = 1, ik;


  if (p.getSchema() == DIF_HEALP_RING) { nested = 0; } // Ring schema?
  int k = p.params[0];
  ik = k;

//// Nested?
//  if (k < 0) {
//    nested = 1;
//    ik = -k;
//  } else
//    ik = k;

//cout << "Nested: "<< nested << "  Order: " << ik << endl;

// Out of range: return here
  if ((k < 0) || (k > 29))
    return -2;

  my_nside = 1 << ik;

  T_Healpix_Base<int64>* base;

  if (nested)
//    Healpix_Base2 base (my_nside, NEST, SET_NSIDE);
    base = new T_Healpix_Base<int64>(my_nside, NEST, SET_NSIDE);
  else
//    Healpix_Base2 base (my_nside, RING, SET_NSIDE);
    base = new T_Healpix_Base<int64>(my_nside, RING, SET_NSIDE);

#ifdef DEBUG_PRINT
//  int order = base.nside2order(my_nside);
  cout <<"Nside of map: "<< my_nside << endl;
#endif

  pointing ptg;
  ptg.theta = (90. - dec)*DEG2RAD;
  ptg.phi   = ra*DEG2RAD;
  double rad = radius/60.*DEG2RAD;
#ifdef DEBUG_PRINT
cout<<"rad="<<rad<<endl;
#endif

  if (rad < MIN_CONE_RAD)
    rad = MIN_CONE_RAD;

#ifdef DEBUG_PRINT
  if (rad == MIN_CONE_RAD)
    cout <<"***Cone radius reset to "<< MIN_CONE_RAD <<" radians ("<< MIN_CONE_RAD/DEG2RAD * 3.6e6 <<" mas)***\n";
#endif

  try {

// maximum angular distance between any pixel center and its corners
    double mpr = base->max_pixrad();

// All intersted pixels!
  //base->query_disc(ptg, rad+mpr, tmp_list);
    base->query_disc_inclusive(ptg, rad, tmp_list, 2);

    for (j=0; j<tmp_list.size(); j++)
      all.push_back( tmp_list[j] );

    tmp_list.clear();

// Sort array - already sorted by toVector method in query_disc_inclusive
  //sort(all.begin(),all.end());

// If nothing found then there is an error
    if (all.size() == 0) {
#ifdef DEBUG_PRINT
  cout <<"Error in query_disc_inclusive\n";
#endif
      return 1;
    }

#ifdef DEBUG_PRINT
  cout <<"ALL nodes:\n";
  for (j=0; j<all.size(); j++)
    cout << j <<": "<< all[j] << endl;
#endif

    vector<long long int> *flist, *plist;
    flist = &p.flist(k);
    plist = &p.plist(k);

// If just 1 then stop here
    if (all.size() == 1) {
      plist->push_back(all[0]);
#ifdef DEBUG_PRINT
  cout <<"Partial node:\n"
       <<"0: "<< plist[0] << endl;
#endif
      return 0;
    }


// Decrease by the max pix. radius. This would give all pixel (approx ?!)
// fully covered by the disc.
//--  base.query_disc(ptg, rad-1.362*M_PI/(4*my_nside), tmp_list);
  //base->query_disc(ptg, rad-mpr, tmp_list);

    if (rad-mpr > 0) {
      base->query_disc(ptg, rad-mpr, tmp_list);

      for (j=0; j<tmp_list.size(); j++)
        flist->push_back( tmp_list[j] );

      tmp_list.clear();

// Sort array - already sorted by toVector method in query_disc
    //sort(flist->begin(),flist->end());
#ifdef DEBUG_PRINT
  cout <<"Full nodes:\n";
  for (j=0; j<flist->size(); j++)
    cout << j <<": "<< (*flist)[j] << endl;
#endif
    }

// Difference -> partial nodes
    difflist_i(all,*flist,*plist);

#ifdef DEBUG_PRINT
  cout <<"DIFmyHealPCone: N partial: "<< plist->size() << endl;
  for (j=0; j<plist->size(); j++)
    cout << j << ": " << (*plist)[j] << endl;

  double m_area = M_PI / (3.*base->Nside()*base->Nside()) /
         DEG2RAD / DEG2RAD * 3600;
  double mpr_m = mpr / DEG2RAD * 60;
  double mpr_s = mpr_m * 60;
  cout <<endl
       <<"Total Nr of nodes: "<< all.size() <<"  Full: "<< flist->size()
       <<"  Partial: "<< plist->size() << endl << endl
       <<"Nside: "<< base->Nside() <<"  Npix: "<< base->Npix() << endl
       <<"Pixel area: "<< m_area <<" arcmin^2 ("<< m_area * 3600 <<" arcsec^2)\n"
       <<"Ang. res (SQRT(P_area)): "<< sqrt(m_area) <<" arcmin ("<< sqrt(m_area)*60 <<" arcsec)\n"
       <<"max_pixrad= "<< mpr_m <<" arcmin ("<< mpr_s <<" arcsec)\n";
#endif
 
    return 0;

  }
  catch (std::exception &e) {
    cout <<"Error executing DIFmyHealPCone. std::exception: "<< e.what() << std::endl;
    return -3;
  }

}
