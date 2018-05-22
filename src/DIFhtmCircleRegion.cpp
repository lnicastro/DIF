/*
  Name:  int DIFhtmCircleRegion

  Description:
   Intersect a circular domain with the HTM grid returning into the input
   MySearch class IDs of fully contained trixels of various depths and partial
   trixels of the highest available depth.
   For rectangular domains see 'DIFhtmRectRegion'.

  Parameters:
   (i) DIF_Region &p:   Pointer to the DIF_Region class

  Note:
    Npix = 8 * 4^d  ( => 1 << (2*d+3) )
    ID range = [Npix, 2*Npix - 1]
    For d=6 it is Pix_area ~1.26 deg^2 (8 * 4^6 = 32768 pixels)

  Return 0 on success.


  LN@IASF-INAF, January 2009                   ( Last change: 06/07/2016 )
*/

//using namespace std;

#include <vector>
#include "SpatialInterface.h"

#include "dif.hh"

/* degrees to radians */
static const double DEG2RAD = 1.74532925199432957692369E-2;
 

int DIFhtmCircleRegion(DIF_Region &p)
{
// No available depth: return
  if (p.params.size() == 0)
    return -1;

  int max_depth = p.params.back();

// Out of range: return here (note that here we reject depth=0)
  if ((max_depth <= 0) || (max_depth > 25))
    return -1;

  double ra = p.ra1;
  double dec = p.de1;
  double radius = p.rad;

  vector<int> depths(p.params);
  vector<long long int> nid_depths;

  ValVec<uint64> plist, flist;  // List results
  unsigned long long int i, j, j0=0;
  double distance = cos(radius/60.*DEG2RAD);


  try {
// Construct index with max depth
    const SpatialIndex index(max_depth);

    SpatialDomain domain;
    SpatialVector v(ra, dec);
    SpatialConvex cvx;
    SpatialConstraint constr(v, distance);
    cvx.add(constr);
    domain.add(cvx);

// Test for just number of trixels
    vector<long long int> nid_part, nid_full;
    domain.intersect(&index,depths,nid_part,nid_full);

#ifdef DEBUG_PRINT
for (i = 0; i < depths.size(); i++)
cout <<"Depth: "<< depths[i] <<"  N full: "<< nid_full[i]
     <<"  N part: "<< nid_part[i] << endl << endl;
#endif


// Domain intersection
    domain.intersect(&index,depths,nid_depths,plist,flist);

    vector<long long int> *list;

#ifdef DEBUG_PRINT
cout <<"N pars: "<< p.params.size() << endl;
for (i = 0; i < p.params.size(); i++)
  cout << p.params[i] << endl;
cout <<"depths.size() = "<< depths.size() << endl;
cout <<"max_depth = "<< max_depth << endl;
#endif

// Returned lists of full nodes at the various depths
    for (i = 0; i < depths.size(); i++) {
//      if (nid_full[i] > 0) {

#ifdef DEBUG_PRINT
cout <<"DIFhtmCircleRegion: Depth: "<<i depths[i] <<"  N full: "<< nid_depths[i]
     << endl;
#endif

//      vector<long long int>& flist2 = p.flist(depths[i]);
        list = &p.flist(depths[i]);

        for (j = 0; j < nid_depths[i]; j++) {    // loop full nodes list

#ifdef DEBUG_PRINT
cout << j <<": "<< index.nameById(flist(j+j0)) <<": "<< flist(j+j0) << endl;
#endif

          list->push_back(flist(j+j0));
        }
        j0 += j;
//      }
    }

#ifdef DEBUG_PRINT
cout <<"\nDIFhtmCircleRegion: Total N full: "<< j0 << endl;
#endif

// Returned list of partial nodes at the max depth
//    vector<long long int>& plist2 = p.plist(max_depth);
    list = &p.plist(max_depth);

#ifdef DEBUG_PRINT
cout <<"DIFhtmCircleRegion: N partial: "<< plist.length() << endl;
#endif

    for (i = 0; i < plist.length(); i++) {    // loop partial nodes list

#ifdef DEBUG_PRINT
cout << plist(i) << endl;
#endif
      list->push_back(plist(i));
    }

  }
  catch (SpatialException &x) {
    return -2;
  }

  return 0;
}
