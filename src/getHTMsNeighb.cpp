/*
  Name: int getHTMsNeighb, int getHTMsNeighb1

  Description:
   Return the HTM trixel IDs, at the same or higher depth, of neighboring
   pixels of a given pixel ID.
   Result contains a variable Nr. of trixels (sorted in ascending order).
   getHTMsNeighb1 always create a new htmInterface base and destroy it on exit.

  Parameters:
 ( (i) char*& saved: if not NULL then re-use existing SpatialIndex )
   (i) int depth:  Depth level of map in the range [0, 25]
   (i) unsigned long long int id: Pixel ID
   (i) int odepth:  Depth level of map for the border trixel [0, 25]

   (o) vector<long long int>& idn: HTM IDs of neighbors ()

  Note:
    If depth is not in the allowed range then set all ID to -1 and return.
    For coordinates multiple of 90 deg. the number of neighbors can be less
    than 12 (tipically 10).
    If depth is not in the allowed range then return an empty list
    (and not 0 code).

  Return 0 on success.


  LN@IASF-INAF, November 2013                      Last change: 15/10/2015
*/

using namespace std;

#include <vector>
#include "SpatialInterface.h"

/* degrees to radians */
static const double DEG2RAD = 1.74532925199432957692369E-2;


int getHTMNeighb(char*& saved, int depth, unsigned long long int id,
                 vector<unsigned long long int>& idn);

void cleanHTMsUval(char*& saved, char*& osaved)
{
  if (saved)
    delete (SpatialIndex*) saved;
  if (osaved)
    delete (SpatialIndex*) osaved;
}


// See SpatialIndex.cpp
bool vec_isInside(const SpatialVector & v, const SpatialVector & v0,
               const SpatialVector & v1, const SpatialVector & v2)
{
  if( (v0 ^ v1) * v < -gEpsilon) return false;
  if( (v1 ^ v2) * v < -gEpsilon) return false;
  if( (v2 ^ v0) * v < -gEpsilon) return false;
  return true;
}



int getHTMsNeighb(char*& saved, char*& osaved, int depth, unsigned long long int id, int odepth,
                 vector<unsigned long long int>& idn)
{

// Depth in allowed range
  if ((depth < 0) || (depth > 25) || (odepth < 0) || (odepth > 25))
    return -1;

// ID in allowed range
  long long unsigned int npix = (1 << (2*depth+3));
  if (id < npix || id > 2*npix-1)
    return -2;

  ValVec<uint64> plist, flist;
  unsigned long long int this_id=0;
  size_t i, j, n, nm;

  int depth_diff = odepth-depth;
  nm = 1<<depth_diff;

  if (depth_diff < 0)
    return -3;
  else if (depth_diff == 0)
    return getHTMNeighb(saved, depth, id, idn);


//int depth_ratio = 1 << 2*depth_diff;
//cout<<"depth_ratio="<<depth_ratio<<" nm="<<nm<<endl;
//cout<<(depth_ratio/4)*3 + 3*6<<endl;

  SpatialIndex *index, *oindex;

  idn.clear();
//idn.resize(1);

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
    SpatialVector vm, p;
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
      p = (vi[0] ^ vm);
//p.show();

// Modulus -> ~triangle side; divide by 8 to use a smaller radius.
//cout <<"p.length="<<p.length()/DEG2RAD<<endl;
      r = cos(p.length() / 8.);
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
    idn.push_back(plist(0));

    j = 1;
    for (i=1; i<plist.length(); i++) {
      this_id = plist(i);
      if (this_id != idn[j-1]) {
        idn.push_back(plist(i));
        //idn[j] = this_id;
        j++;
      }
    }

// Remove inner trixels
    n = 0;
    for (i=0; i<j; i++) {
      oindex->nodeVertex(idn[i], vo[0], vo[1], vo[2]);
      p = vo[0] + vo[1] + vo[2];
      if (! vec_isInside(p, v[0], v[1], v[2])) {
        //this_id = idn[i];
        idn[n] = idn[i];
        n++;
      }

    }

    idn.resize(n);

  }
  catch (SpatialException &x) {
//cout << "Error. SpatialException: " << x.what() << endl;
    return -2;
  }

  return 0;
}


int getHTMsNeighb1(int depth, unsigned long long int id, int odepth,
                   vector<unsigned long long int>& idn)
{
  char* saved = NULL;
  char* osaved = NULL;
  int ret = getHTMsNeighb(saved, osaved, depth, id, odepth, idn);
  cleanHTMsUval(saved, osaved);
  return ret;
}
