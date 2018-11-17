/*
  Name:  int DIFhtmRectRegion, DIFhtmRectRegion2V, DIFhtmRectRegion4V

  Description:
   Intersect a rectangular sky region with the HTM grid returning into the input
   DIF_Region class IDs of fully contained trixels of various depths and partial
   trixels of the highest available depth.
   Note: sides are along the RA/Dec axes.
 
   For circular domains see 'DIFhtmCircleRegion'.

  Parameters:
   (i) DIF_Region &p:  Pointer to the DIF_Region class

  Previously used additional parameters:
  DIFhtmRectRegion
   (i) double cra:     Right Ascension of center (degrees)
   (i) double cde:     Declination of center (degrees)
   (i) double side_ra: Side length along RA (arcmin)
   (i) double side_de: Side length along Dec (arcmin).
                       If omitted then use side_ra (i.e. square area)

  DIFhtmRectRegion2V
   (i) double ra1: Right Ascension of first corner (degrees)
   (i) double de1: Declination of first corner (degrees)
   (i) double ra2: Right Ascension of opposite corner (degrees)
   (i) double de2: Declination of opposite corner (degrees)

  DIFhtmRectRegion4V
   (i) double ra[4]:  Array of corners Right Ascension (degrees)
   (i) double dec[4]: Array of corners Declination (degrees)

  Note:
     Npix = 8 * 4^d  ( => 1 << (2*d+3) )
     ID range = [Npix, 2*Npix - 1]
     For d=6 it is Pix_area ~1.26 deg^2 (8 * 4^6 = 32768 pixels)

  Return 0 on success.


  LN @ INAF-OAS, January 2009                   ( Last change: 17/11/2018 )
*/

#include <iostream>
using namespace std;

#include <vector>
#include "SpatialInterface.h"

#include "dif.hh"


// Input center and the length of the two sides (one length => square)

int DIFhtmRectRegion(DIF_Region &p)
{

  double cra = p.ra1;
  double cde = p.de1;
  double side_ra = p.ra2;
  double side_de = p.de2;

  double hside_ra = side_ra/120.;
  double hside_de = hside_ra;

  if (side_de > 0.)
    hside_de = side_de/120.;

// Check parameters
  while (cra < 0.) cra += 360.;

  if ( (! (  0. <= cra      &&  cra       <  360.))  ||
       (! (-90. <= cde      &&  cde       <=  90.))  ||
       (! (  0. < hside_ra  &&  hside_ra  <= 180.))  ||
       (! (  0. < hside_de  &&  hside_de  <=  90.)) )
    return -1;

  hside_ra /= cos(cde*M_PI/180.);
  if (hside_ra > 180.)
    hside_ra = 180.;

//cout << cra<<" "<<cde << " "<<hside_ra << " "<<hside_de << endl;
// Compute coordinates for two opposite corners (clockwise coords)
  double ra1 = cra - hside_ra;
  double ra2 = cra + hside_ra;
  double de1 = cde - hside_de;
  double de2 = cde + hside_de;

//  if (ra1 < 0) ra1 += 360.;  // Managed in SpatialVector

  p.ra1 = ra1;
  p.ra2 = ra1;
  p.ra3 = ra2;
  p.ra4 = ra2;
  p.de1 = de1;
  p.de2 = de2;
  p.de3 = de2;
  p.de4 = de1;

#ifdef DEBUG_PRINT
cout <<"DIFhtmRectRegion:"<<endl;
cout << p.ra1 <<" "<< p.ra2 <<" "<< p.ra3 << " "<< p.ra4 << endl;
cout << p.de1 <<" "<< p.de2 <<" "<< p.de3 << " "<< p.de4 << endl;
//cout << ra1<<" "<<ra2 << " "<<de1 << " "<<de2 << endl;
#endif

  p.regtype = DIF_REG_4VERT;  // reset region type
  return ( DIFhtmRectRegion4V(p) );
}


// Input RA and Dec of the two opposite corners

int DIFhtmRectRegion2V(DIF_Region &p)
{

  double ra1 = p.ra1;
  double de1 = p.de1;
  double ra2 = p.ra2;
  double de2 = p.de2;

// Order ranges (clockwise coords starting from bottom-right!)
  if (ra1 < ra2) {
    p.ra1 = ra2;
    p.ra3 = ra1;
  } else {
    p.ra1 = ra1;
    p.ra3 = ra2;
  }
  p.ra2 = p.ra3;
  p.ra4 = p.ra1;

  if (de1 < de2) {
    p.de1 = de1;
    p.de3 = de2;
  } else {
    p.de1 = de2;
    p.de3 = de1;
  }
  p.de2 = p.de1;
  p.de4 = p.de3;

#ifdef DEBUG_PRINT
cout <<"DIFhtmRectRegion2V:"<<endl;
cout << p.ra1 <<" "<< p.ra2 <<" "<< p.ra3 << " "<< p.ra4 << endl;
cout << p.de1 <<" "<< p.de2 <<" "<< p.de3 << " "<< p.de4 << endl;
printf("%lf, %lf, %lf, %lf\n",p.ra1, p.ra2, p.ra3, p.ra4);
printf("%lf, %lf, %lf, %lf\n",p.de1, p.de2, p.de3, p.de4);
#endif

  p.regtype = DIF_REG_4VERT;  // reset region type
  return ( DIFhtmRectRegion4V(p) );
}


// Input RA and Dec arrays of the four corners.
// This is the main function and the only one used by DIF.

int DIFhtmRectRegion4V(DIF_Region &p)
{
// No available depth: return
  if (p.params.size() == 0)
    return -1;

  int max_depth = p.params.back();

// Out of range: return here (note that here we reject depth=0)
  if ((max_depth <= 0) || (max_depth > 25))
    return -2;

  double ra[4];
  double dec[4];
  ra[0] = p.ra1;
  ra[1] = p.ra2;
  ra[2] = p.ra3;
  ra[3] = p.ra4;
  dec[0] = p.de1;
  dec[1] = p.de2;
  dec[2] = p.de3;
  dec[3] = p.de4;

  vector<int> depths(p.params);
  vector<long long int> nid_depths;

  ValVec<uint64> plist, flist;  // List results
  unsigned long long int i, j, j0=0;


  try {
// Construct index with max depth
    const SpatialIndex index(max_depth);

    SpatialDomain domain;
    SpatialVector v1(ra[0], dec[0]);
    SpatialVector v2(ra[1], dec[1]);
    SpatialVector v3(ra[2], dec[2]);
    SpatialVector v4(ra[3], dec[3]);
    SpatialConvex cvx(&v1,&v2,&v3,&v4);
    domain.add(cvx);

// Test for just number of trixels
    vector<long long int> nid_part, nid_full;
    domain.intersect(&index,depths,nid_part,nid_full);

#ifdef DEBUG_PRINT
for (i = 0; i < depths.size(); i++)
cout <<"Depth: "<< depths[i] <<"  N full: "<< nid_full[i]
     <<"  N part: "<< nid_part[i] << endl;
#endif


// Domain intersection
    domain.intersect(&index,depths,nid_depths,plist,flist);
    //domain.intersect(&index,plist,flist);

    vector<long long int> *list;

// Returned lists of full nodes at the various depths
    for (i = 0; i < depths.size(); i++) {
#ifdef DEBUG_PRINT
cout <<"DIFhtmRectRegion4V: Depth: "<< depths[i] <<"  N full: "<< nid_depths[i]
     << endl;
#endif
//      vector<long long int>& flist2 = p.flist(depths[i]);
      list = &p.flist(depths[i]);

      for (j = 0; j < nid_depths[i]; j++) {    // loop full nodes list
        list->push_back(flist(j+j0));
#ifdef DEBUG_PRINT
cout << j <<": "<< index.nameById(flist(j+j0)) <<": "<< flist(j+j0) << endl;
#endif
      }
      j0 += j;
    }

// Returned list of partial nodes at the max depth
    list = &p.plist(max_depth);
#ifdef DEBUG_PRINT
cout <<"DIFhtmRectRegion4V: N partial: "<< plist.length() << endl;
#endif
    for (i = 0; i < plist.length(); i++)      // loop partial nodes list
      list->push_back(plist(i));

  }
  catch (SpatialException &x) {
    return -1;
  }

  return 0;
}
