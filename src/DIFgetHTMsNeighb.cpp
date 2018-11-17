/*
  Name: int DIFgetHTMsNeighb, int DIFgetHTMsNeighb1

  Description:
   Return into the input DIF_Region class the HTM trixel IDs, at the same
   or higher depth, of neighboring pixels of a given pixel ID.
   Result contains a variable Nr. of trixels (sorted in ascending order).
   getHTMsNeighb1 always create a new htmInterface base and destroy it on exit.

  Parameters:
 ( (i) char*& saved: if not NULL then re-use existing SpatialIndex )
   (i) int depth:  Depth level of map in the range [0, 25]
   (i) long long int id: Pixel ID
   (i) int odepth:  Depth level of map for the border trixel [0, 25]

  DIFgetHTMsNeighb1:
   (i) DIF_Region &p: Pointer to the DIF_Region class

  Note:
    If depth is not in the allowed range then set all ID to -1 and return.
    For coordinates multiple of 90 deg. the number of neighbors can be less
    than 12 (tipically 10).
    If depth is not in the allowed range then return an empty list
    (and not 0 code).

  Return 0 on success.


  LN @ INAF-OAS, November 2013                      Last change: 17/11/2018
*/

#include <iostream>
using namespace std;

#include <vector>
#include "SpatialInterface.h"

#include "dif.hh"

/* degrees to radians */
static const double DEG2RAD = 1.74532925199432957692369E-2;


bool vec_isInside(const SpatialVector & v, const SpatialVector & v0,
               const SpatialVector & v1, const SpatialVector & v2);
//void cleanHTMsUval(char*& saved, char*& osaved)
//{
//  if (saved)
//    delete (SpatialIndex*) saved;
//  if (osaved)
//    delete (SpatialIndex*) osaved;
//}



int DIFgetHTMsNeighb(char*& saved, char*& osaved, DIF_Region &p, int depth, long long int id, int odepth)
{

  ValVec<uint64> plist, flist;
  long long int this_id=0;
  size_t i, j, n, nm;

// No available depth?
  if (p.params.size() == 0)
    return -1;
// Use the smaller pixel (if more than one depth)
 //int depth = p.params[0];
 // int depth = p.params.back();

// Out of range: return here
  if ((depth < 0) || (depth > 25) || (odepth < 0) || (odepth > 25))
    return -2;

// ID in allowed range
  long long unsigned int npix = (1 << (2*depth+3));
  if (id < npix || id > 2*npix-1)
    return -3;

  int depth_diff = p.outdepth-depth;
  nm = 1<<depth_diff;

  if (depth_diff < 0)
    return -4;
  //else if (depth_diff == 0)
    //return DIFgetHTMNeighb(saved, depth, id, idn);  // not implemented

  SpatialIndex *index, *oindex;

  vector<long long int> &list = p.flist(p.outdepth);
  list.clear();
//list.resize(1);

  try {
    if (! saved) {
      index = new SpatialIndex(depth);  // generate htm interfaces
      oindex = new SpatialIndex(odepth);
      saved = (char*) index;
      osaved = (char*) oindex;

    } else {
      index = (SpatialIndex*) saved;
      oindex = (SpatialIndex*) osaved;
    }

//uint64 id;
//id = htm->lookupID(ra, dec);

// Vertices of central trixel
    static ValVec<SpatialVector> v, vi, vo;
    v.at(3);
    vi.at(nm+1);
    vo.at(nm+1);
    ValVec<SpatialConvex> cvx;
    cvx.at((nm+1)*3);

    double r=1.;
    SpatialVector vm, pm;
    static SpatialConstraint constr;
    SpatialDomain domain;    // initialize empty domain

    index->nodeVertex(id, v[0], v[1], v[2]);


  for (int is=0; is<3; is++) {

   if (is == 0) {
// Side 0
    vi[0] = v[1];
    vi[1] = v[1] + v[2]; vi[1].normalize();
    vi[2] = v[2];
   } else if (is == 1) {
// Side 1
    vi[0] = v[2];
    vi[1] = v[2] + v[0]; vi[1].normalize();
    vi[2] = v[0];
   } else {
// Side 2
    vi[0] = v[0];
    vi[1] = v[0] + v[1]; vi[1].normalize();
    vi[2] = v[1];
   }

//vo[0] = vi[0];
//vo[nm/2] = vi[1];
//vo[nm] = vi[2];

//m=nm/2;
// Only if depth diff. > 1, i.e. more than 1 vertex on a side
    n = 2;
    while (n < nm) {
      for (i=1; i<=n; i++) {
        vo[(i-1)*2 + 1] = vi[i-1] + vi[i]; vo[(i-1)*2 + 1].normalize();
        vo[i*2] = vi[i];
//cout<<m*i - m/2 <<endl;
//[m*i - m/2]
      }
      for (i=1; i<=2*n; i++)
        vi[i] = vo[i];

      n *=2;
//m /=2;
    }
   
    if (is == 0) {
// Cross product
      vm = (vi[0] + vi[1]); vm.normalize();
      pm = (vi[0] ^ vm);
//p.show();

// Modulus -> ~triangle side; divide by 8 to use a smaller radius.
//cout <<"p.length="<<p.length()/DEG2RAD<<endl;
      r = cos(pm.length() / 8.);
    }

// Skip second vertex of parent trixel. Will be first of next side.
    for (i=0; i<nm; i++) {
//cout << i << ": RA, Dec: " << vi[i].ra() << "  " << vi[i].dec() << endl;
      constr.setVector(vi[i]);
      constr.setDistance(r);
      cvx[i+is*nm].add(constr);
      domain.add(cvx[i+is*nm]);
    }

  }  // end for is


    domain.intersect(oindex,plist,flist);

//cout<<"partial: "<<plist.length()<<" full: "<<flist.length()<<endl;

// Array already sorted: it is easy to remove duplicated entries
    list.push_back(plist(0));

    j = 1;
    for (i=1; i<plist.length(); i++) {
      this_id = plist(i);
      if (this_id != list[j-1]) {
        list.push_back(plist(i));
        //list[j] = this_id;
        j++;
      }
    }

// Remove inner trixels
    n = 0;
    for (i=0; i<j; i++) {
      oindex->nodeVertex(list[i], vo[0], vo[1], vo[2]);
      pm = vo[0] + vo[1] + vo[2];
      if (! vec_isInside(pm, v[0], v[1], v[2])) {
        //this_id = list[i];
        list[n] = list[i];
        n++;
      }

    }

    list.resize(n);

#ifdef DEBUG_PRINT
for (i = 0; i < list.size(); i++)
cout <<"ID "<<i<<": "<< list[i] <<endl;
#endif

  }
  catch (SpatialException &x) {
//cout << "Error. SpatialException: " << x.what() << endl;
    return -2;
  }

  return 0;
}


int DIFgetHTMsNeighb1(DIF_Region &p)
{
  char* saved = NULL;
  char* osaved = NULL;
  int ret = DIFgetHTMsNeighb(saved, osaved, p, p.indepth, p.refpix, p.outdepth);
  cleanHTMsUval(saved, osaved);
  return ret;
}
