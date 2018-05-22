/*
  Name:  int DIFmyHealPRect4V

  Description:
   Intersect a rectangular region with the HEALPix grid returning into the input
   DIF_Region class IDs of fully contained trixels of various depths and partial
   pixels of the highest available depth.
   Note: sides are along the RA/Dec axes.
 
   For circular domains see 'DIFmyHealPCone'.

  Parameters:
   (i) DIF_Region &p: Pointer to the DIF_Region class

  Note:
   Npix = 12 * Nside^2
   Nside = 2^k, k = [0, ..., 29] -> order = WMAP resolution parameter
   Sky resolution: Omega=PI/(3Nside^2)
   For Nside=64 it is ~1 deg (12 * 64^2 = 49152 pixels).

   Coordinates assumed within allowed ranges and clock-wise from SW corner.

   The minimum side is TBD (1 mas assumed). See Healpix_Base::query_disc and pix2loc.

   Returned list of partial pixels could be > real ones, that of full pixels
   could be < real ones.
   If k is not in the allowed range then return empty lists (and not 0 code).

  Return 0 on success.


  LN@IASF-INAF, July 2016                   ( Last change: 08/06/2017 )
*/

#include <algorithm>

/* degrees to radians */
static const double DEG2RAD = 1.74532925199432957692369E-2;

/* Half PI */
static const double PID2    = 1.57079632679489661923132169163975144;

#include "arr.h"
#include "geom_utils.h"
#include "healpix_base.h"

using namespace std;

#include "dif.hh"


extern void difflist_i(vector<long long int>& v1,
                       vector<long long int>& v2,
                       vector<long long int>& vdiff);


// Input RA and Dec arrays of the four corners.
// This is the main function and the only one used by DIF.

int DIFmyHealPRect4V(DIF_Region &p)
{

// No available depth: return
  if (p.params.size() == 0)
  return -1;

  int64 my_nside;
  vector<int64> tmp_list;
  vector<long long int> all;
  unsigned long long int j;

// Default is NESTED scheme
  int nested = 1, ik;


  if (p.getSchema() == DIF_HEALP_RING) { nested = 0; } // Ring schema?
  int k = p.params[0];
  ik = k;

// Out of range: return here
  if ((k < 0) || (k > 29))
    return -2;

  my_nside = 1 << k;

  T_Healpix_Base<int64>* base;

  if (nested)
    base = new T_Healpix_Base<int64>(my_nside, NEST, SET_NSIDE);
  else
    base = new T_Healpix_Base<int64>(my_nside, RING, SET_NSIDE);

  try {
    std::vector<pointing> vertex;
    rangeset<int64> pixset;
    pointing ptg;

    ptg.theta = (90. - p.de1)*DEG2RAD;
    ptg.phi   = p.ra1*DEG2RAD;
    vertex.push_back(ptg);

    ptg.theta = (90. - p.de2)*DEG2RAD;
    ptg.phi   = p.ra2*DEG2RAD;
    vertex.push_back(ptg);

    ptg.theta = (90. - p.de3)*DEG2RAD;
    ptg.phi   = p.ra3*DEG2RAD;
    vertex.push_back(ptg);

    ptg.theta = (90. - p.de4)*DEG2RAD;
    ptg.phi   = p.ra4*DEG2RAD;
    vertex.push_back(ptg);

#ifdef DEBUG_PRINT
for (unsigned short i=0; i<4; i++)
  cout<<"theta="<<vertex[i].theta<<"  phi="<<vertex[i].phi <<endl;
#endif

// All interested pixels!
      base->query_polygon_inclusive(vertex, pixset, 2);

// If just 1 then stop here

    if (pixset.size() == 0) {
#ifdef DEBUG_PRINT
      cout <<"Error in query_polygon_inclusive\n";
#endif
      return 1;
    }

    vector<long long int> *flist, *plist;
    flist = &p.flist(k);
    plist = &p.plist(k);

    pixset.toVector(tmp_list);

// If just 1 then stop here
    if (pixset.size() == 1 && pixset.ivlen(0) == 1) {
      plist->push_back(tmp_list[0]);
#ifdef DEBUG_PRINT
    cout <<"Partial node:\n"
         <<"0: "<< plist[0] << endl;
#endif
      return 0;
    }

    for (j=0; j<tmp_list.size(); j++)
      all.push_back( tmp_list[j] );

    tmp_list.clear();

  //pixset.toVector(all);

// Sort array - already sorted by toVector method
    //sort(all.begin(),all.end());

#ifdef DEBUG_PRINT
  cout <<"ALL nodes:\n";
  for (j=0; j<all.size(); j++)
    cout << j <<": "<< all[j] << endl;
#endif

// maximum angular distance between any pixel center and its corners, in radians
    double mpr = base->max_pixrad();

// Decrease by the max pix. radius. This would give all pixel (approx ?!)
// fully covered by the rectangle.

    double mpr_c = mpr / cos(PID2 - (vertex[0].theta+vertex[1].theta)/2.);
    vertex[0].phi += mpr_c;
    vertex[0].theta -= mpr;
    vertex[2].phi -= mpr_c;
    vertex[1].theta += mpr;
    vertex[1].phi = vertex[0].phi;
    vertex[2].theta = vertex[1].theta;
    vertex[3].phi = vertex[2].phi;
    vertex[3].theta = vertex[0].theta;

#ifdef DEBUG_PRINT
cout<<"mpr, mpr_c: "<<mpr<<", "<<mpr_c<<endl;
for (unsigned short i=0; i<4; i++)
  cout<<"theta="<<vertex[i].theta<<"  phi="<<vertex[i].phi <<endl;
#endif

    if (vertex[0].theta - vertex[1].theta > 0) {
      base->query_polygon_inclusive(vertex, pixset, 4);

      if (pixset.size() > 0) {

        pixset.toVector(tmp_list);

        for (j=0; j<tmp_list.size(); j++)
          flist->push_back( tmp_list[j] );

        tmp_list.clear();

// Sort array - already sorted by toVector method
        //sort(flist->begin(),flist->end());
#ifdef DEBUG_PRINT
  cout <<"Full nodes:\n";
  for (j=0; j<flist->size(); j++)
    cout << j <<": "<< (*flist)[j] << endl;
#endif
      }
    }

// Difference -> partial nodes
    difflist_i(all,*flist,*plist);
    return 0;

  }
  catch (std::exception &e) {
    cout <<"Error executing DIFmyHealPRect. std::exception: "<< e.what() << std::endl;
    return -3;
  }

}
