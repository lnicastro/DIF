/* LN modif: 8/10/2014
 
  Note:
   - This version filters multiple matches within the search distance returning only the closest one.
   - If the max number of matches is negative, then set it to the max possible (NxM).
   - See function spherematch2_mm below for multiple matches.

   - Separation returned in arcsec!

  Last change: 23 June 2020 
 */

#include <cstdlib>
#include <cmath>
#include <vector>
#include "chunks.h"
#include <iostream>
#include <algorithm>    // std::sort

using namespace std;

struct multi { 
    unsigned int index;
    unsigned int id1;
    unsigned int id2;
    int d12;
    //float d12;
    //size_t index;
};

// Sorting definitions
struct mm_by_d12 {  // Distance
    bool operator()(multi const &left, multi const &right) {
        return (left.d12 < right.d12);
    }
};


/* These unused
struct mm_by_id1 { 
    bool operator()(multi const &left, multi const &right) { 
        return (left.id1 < right.id1);
    }
};

struct mm_by_id2 { 
    bool operator()(multi const &left, multi const &right) { 
        return (left.id2 < right.id2);
    }
};

struct mm_by_id1_d12 { 
    bool operator()(multi const &left, multi const &right) { 
        return (left.id1 < right.id1 && left.d12 < right.d12);
    }
};

struct mm_by_id2_d12 { 
    bool operator()(multi const &left, multi const &right) { 
        return (left.id2 < right.id2 && left.d12 < right.d12);
    }
};
*/


static long *nra = NULL, ndec;
static double **rabounds = NULL, *decbounds = NULL;
static double raoffset;
static double *xx2 = NULL,*yy2 = NULL,*zz2 = NULL;
static long **nchunk2 = NULL, ***chunklist2 = NULL; 


extern "C"
double separation(double xx1, double yy1, double zz1, double xx2, double yy2, double zz2);

#define FREEVEC(a) {if((a)!=NULL) free((char *) (a)); (a)=NULL;}

static void free_memory()
{
	FREEVEC(xx2);
	FREEVEC(yy2);
	FREEVEC(zz2);
	if(nchunk2!=NULL) 
		unassignchunks(&nchunk2,&chunklist2,nra,ndec);
	if(rabounds!=NULL)
		unsetchunks(&rabounds,&decbounds,&nra,&ndec);
	FREEVEC(nchunk2);
	FREEVEC(chunklist2);
	FREEVEC(rabounds);
	FREEVEC(decbounds);
	FREEVEC(nra);
}



/********************************************************************/

long spherematch2(
   unsigned long npoints1, double *ra1, double *dec1,
   unsigned long npoints2, double *ra2, double *dec2,
   double matchlength, double minchunksize,
   std::vector<long> &match1s, std::vector<long> &match2s, std::vector<float> &distance12s,
   unsigned long *nmatch )
{

  if (npoints1 == 0 || npoints2 == 0) {
cout<<"Spherematch2: Error: npoints1 = "<< npoints1 <<", npoints2 = "<< npoints2 <<endl;
    return -1;
  }

  if (matchlength <= 0 || minchunksize <= 0) {
cout<<"Spherematch2: Error: matchlength = "<< matchlength <<", minchunksize = "<< minchunksize <<endl;
    return -2;
  }

  double myx1, myy1, myz1, currra, sep, rr, dr;  // minsep
  long maxmatch, rachunk, decchunk;
  unsigned long i, j, k, jmax, j1;

  std::vector<long> refcount;
  refcount.clear();

  match1s.clear();
  match2s.clear();
  distance12s.clear();

// Erasing duplicated entries is quite time consuming.
// Use temp vectors and tranfer unique entries after cleaning.
  std::vector<long> match1;
  std::vector<long> match2;
  std::vector<float> distance12;

// Temporary vectors
  multi im;
  std::vector<multi> m;
  m.clear();

/* 1. define chunks */
  setchunks(ra1, dec1, npoints1, minchunksize,
		&rabounds, &decbounds, &nra, &ndec, &raoffset);

/* 2. assign targets to chunks, with minFibreSpacing of leeway */
  assignchunks(ra2, dec2, npoints2, raoffset, matchlength, minchunksize,
		&nchunk2, &chunklist2, rabounds, decbounds, nra, ndec);

/* 3. make x, y, z coords -- compute (x,y,z)1 on the fly - DPF */
  if ( !(xx2 = (double *) malloc(npoints2 * sizeof(double))) ) {
	cerr <<"Error: spherematch2: error allocating memory.\n";
	return 1;
  };
  if ( !(yy2 = (double *) malloc(npoints2 * sizeof(double))) ) {
	cerr <<"Error: spherematch2: error allocating memory.\n";
	return 1;
  };
  if ( !(zz2 = (double *) malloc(npoints2 * sizeof(double))) ) {
	cerr <<"Error: spherematch2: error allocating memory.\n";
	return 1;
  };
 
  for (i = 0; i < npoints2; i++) {
    rr = DEG2RAD*ra2[i];
    dr = DEG2RAD*dec2[i];
    xx2[i] = cos(rr)*cos(dr);
    yy2[i] = sin(rr)*cos(dr);
    zz2[i] = sin(dr);
  } /* end for i */

/* 4. run matching */
  maxmatch = (*nmatch);  /* if nmatch != 0 then fill arrays up to maxmatch */
  (*nmatch) = 0;

  if (maxmatch < 0)  /* if nmatch < 0 then fill arrays up to max number of matches */
    maxmatch = npoints1*npoints2;

  //long ii = 0;

  for (i = 0; i < npoints1; i++) {
    currra = fmod(ra1[i] + raoffset, 360.);
    getchunk(currra, dec1[i], &rachunk, &decchunk, rabounds, decbounds, nra, ndec);
    jmax = nchunk2[decchunk][rachunk];
//if (jmax>0) printf("i, jmax nra ndec  ra1 dec1: %d %d %d %d  %f %f\n", i, jmax, *nra, ndec, ra1[i],dec1[i]);

    if (jmax > 0) {
      rr = DEG2RAD*ra1[i];
      dr = DEG2RAD*dec1[i];
      myx1 = cos(rr)*cos(dr);
      myy1 = sin(rr)*cos(dr);
      myz1 = sin(dr);
      for(j = 0; j < jmax; j++) {
	k = chunklist2[decchunk][rachunk][j];
	sep = separation(myx1, myy1, myz1, xx2[k], yy2[k], zz2[k]);
// This is required to manage multiple matches in ref catalogue
	if (sep <= matchlength && (*nmatch) <= (unsigned long)maxmatch) {  // keep only those within max distance
		j1++;
		im.id1 = i;
		im.id2 = k;
		im.d12 = sep * 3.6e6;  // To mas
		im.index = (*nmatch);
		m.push_back(im);

		match1.push_back(i);
		match2.push_back(k);
		distance12.push_back(sep * 3600.);  // To arcsec

		(*nmatch)++;
	}

      } /* end for j */

      if (j1 > 0) {
	for (j = 0; j < j1; j++)
	  refcount.push_back(j1);
	j1 = 0;
      }

    } /* end if jmax > 0 */

  } /* end for i */
//cout<< "N refcount: "<< refcount.size() << endl;

//for (i = 0; i < *nmatch; i++)
//cout<<"i: "<<i<<" match1: "<<match1[i]<<" match2: "<<match2[i]<<", d: "<< distance12[i] <<endl;

/* 4. clean up after chunks */
  unassignchunks(&nchunk2, &chunklist2, nra, ndec);
  unsetchunks(&rabounds, &decbounds, &nra, &ndec);

/* 6. free memory */
  free_memory();


cout <<"--> spherematch2: total (multi) Xmatch = "<< *nmatch;


#ifdef DEBUG
if (*nmatch > 0) {
  for (i = 0; i < *nmatch; i++)
      cout <<"i: "<< i <<", refcount: "<< refcount[i] <<", id1: "<<m[i].id1<<", id2: "<<m[i].id2<<", d: "<<m[i].d12 <<endl;
}
#endif

  unsigned long n_rm = 0, m_marked = 0, n_unique = 0;

  if (*nmatch == 0) {
	cout << endl;
	return 1;

  } else if (*nmatch == 1) {
	match1s.push_back(match1[0]);
	match2s.push_back(match2[0]);
	distance12s.push_back(distance12[0]);
	cout << endl;

	return 0;
  }


//
// --  More than 1 match: do the 1to1 cleaning job
//

// Array holding the multiple matches
  std::vector<multi> mm;
  mm.clear();


//cout<<"Identifying unique/multiple matches...\n";
// Identify unique and multiple matches
  for (i = 0; i < *nmatch; i++) {
	if (refcount[i] > 1) { // multiple RefObjects for this object
//cout<<"MultiRefCat: "<< i <<": "<< m[i].id1 <<" has "<< refcount[i] << " ref-matches, one is "<<  m[i].id2 <<endl;
		mm.push_back(m[i]);
		continue;
	}
	for (j = 0; j < *nmatch; j++) {
    	  if (m[j].id1 != m[i].id1 && m[j].id2 == m[i].id2) {  // there is another InCat obj matching same RefCat
//cout<<"MultiInCat: "<< i <<": "<< m[j].id2 <<" matches in-cat "<< m[i].id1 <<" and "<< m[j].id1 <<endl;
		mm.push_back(m[i]);
		break;
	  }
	}
	if (j == *nmatch) {  // Unique Incat - RefCat match
		n_unique++;
//cout<<"Unique: "<< m[i].id2 <<"  "<< m[i].id1 << endl;
	}
  }  // end for i


// No need for the original vector: clear it
  std::vector<multi>().swap(m);

  cout <<", "<< n_unique <<" of which unique\n";

// Remove all the InCat objects with a single RefCat counterpart ?

 
#ifdef DEBUG
cout<<"Not unique: "<< mm.size() << endl;
for (i = 0; i < mm.size(); i++)
cout <<"mm[i].id1: "<<mm[i].id1<<", mm[i].id2: "<<mm[i].id2<<", d: "<<mm[i].d12 <<endl;
#endif

//cout<<"Sorting multi-matches vector by distance...\n";
  sort(mm.begin(), mm.end(), mm_by_d12());

//cout <<"Sorted mm:\n";
//for (i = 0; i < mm.size(); i++)
//cout <<"mm[i].id1: "<<m[i].id1<<", mm[i].id2: "<<m[i].id2<<", d: "<<m[i].d12 <<endl;

// Mark duplicated matches with distance > min
  cout <<"--> spherematch2: marking multi-matches to be checked... ";
  for (i = 0; i < mm.size(); i++) {
	if (mm[i].d12 < 0)
	continue;

	for (j = i + 1; j < mm.size(); j++) {
	  if (mm[j].id1 == mm[i].id1 || mm[j].id2 == mm[i].id2) {
		mm[j].d12 = -1;
		m_marked++;
        	continue;
	  }
	}
  }

  cout << m_marked << endl;


// Sorting on distance and checking for negative values does not speed up much things, still...
//cout<<"Sorting multiple matches vector by d12...\n";
  sort(mm.begin(), mm.end(), mm_by_d12());

//for (i = 0; i < mm.size(); i++)
//cout <<"mm[i].id1: "<<mm[i].id1<<", mm[i].id2: "<<mm[i].id2<<", d: "<<mm[i].d12<<endl;

  cout <<"--> spherematch2: multi-matches marking larger distances... ";
  if (m_marked > 0) {
	i = 0;
	while (mm[i].d12 < 0) {

	  if (match1[mm[i].index] == mm[i].id1 && match2[mm[i].index] == mm[i].id2) {
//cout<<"Erase match1: i: "<< i <<", j: "<< j <<",  "<< match1[j] <<" match2: "<< match2[j] <<", d: "<< distance12[j] <<endl<<endl;
		distance12[mm[i].index] = -1;
		n_rm++;
          }
	  i++;
      }  // end while
  }  // end if m_marked > 0

  cout << n_rm <<" to remove... ";

// This is not too different from push_back.
  match1s.reserve(*nmatch - n_rm);
  match2s.reserve(*nmatch - n_rm);
  distance12s.reserve(*nmatch - n_rm);
  j = 0;

// Transfer final unique values into the passed vectors.
  for (i = 0; i < *nmatch; i++) {
	if (distance12[i] >= 0.) {
//cout<< i <<": "<< match1[i] <<" "<< match2[i] <<" "<< distance12[i] << endl;
		match1s[j] = match1[i];
		match2s[j] = match2[i];
		distance12s[j] = distance12[i];
		j++;
	}
  }  // end for i


  *nmatch -= n_rm;

  cout <<"done. Left "<< *nmatch << endl;

//cout<<"Nmatch="<< *nmatch <<"  match1.size() = "<< match1.size() <<", match2.size() = "<< match1.size() <<", distance12.size() = "<< distance12.size()<<endl;

  if (*nmatch > npoints1 || *nmatch > npoints2) {
	cout<<"Spherematch2: Error: more matched objects than input objects:\n"<< 
	"Nmatch="<< *nmatch <<"  npoints1 = "<< npoints1 <<", npoints2 = "<< npoints2 << endl;
	return -3;
  }

// Clear local vector
  std::vector<multi>().swap(mm);
  std::vector<long>().swap(match1);
  std::vector<long>().swap(match2);
  std::vector<float>().swap(distance12);

  return 0;
}


/********************************************************************/


/* LN modif

  Note: this version allows for multiple matches within the search distance.
        However the maximum number of matches is not managed accordingly.

*/

long spherematch2_mm(
   unsigned long   npoints1,
   double *ra1,
   double *dec1,
   unsigned long   npoints2,
   double *ra2,
   double *dec2,
   double matchlength,
   double minchunksize,
   vector<long>   &match1,
   vector<long>   &match2,
   vector<float> &distance12,
   unsigned long   *nmatch )
{

  double myx1,myy1,myz1;
  unsigned long maxmatch, jmax;
  double currra,sep;
  unsigned long i,j,k;
  long rachunk,decchunk;

  match1.clear();
  match2.clear();
  distance12.clear();


/* 1. define chunks */
  setchunks(ra1, dec1, npoints1, minchunksize,
		&rabounds, &decbounds, &nra, &ndec, &raoffset);

/* 2. assign targets to chunks, with minFibreSpacing of leeway */
  assignchunks(ra2, dec2, npoints2, raoffset, matchlength, minchunksize,
		&nchunk2, &chunklist2, rabounds, decbounds, nra, ndec);

/* 3. make x, y, z coords -- compute (x,y,z)1 on the fly - DPF */
  if ( !(xx2 = (double *) malloc(npoints2 * sizeof(double))) ) {
	cerr <<"Error: spherematch2_mm: error allocating memory.\n";
	return 1;
  };
  if ( !(yy2 = (double *) malloc(npoints2 * sizeof(double))) ) {
	cerr <<"Error: spherematch2_mm: error allocating memory.\n";
	return 1;
  };
  if ( !(zz2 = (double *) malloc(npoints2 * sizeof(double))) ) {
	cerr <<"Error: spherematch2_mm: error allocating memory.\n";
	return 1;
  };

  double rr, dr;
  for(i = 0; i < npoints2; i++) {
	rr = DEG2RAD*ra2[i];
	dr = DEG2RAD*dec2[i];
	xx2[i] = cos(rr)*cos(dr);
	yy2[i] = sin(rr)*cos(dr);
	zz2[i] = sin(dr);
  } /* end for i */

/* 4. run matching */
  maxmatch = (*nmatch);  /* if nmatch != 0 then fill arrays up to maxmatch */
  (*nmatch) = 0;
  for (i = 0; i < npoints1; i++) {
	currra = fmod(ra1[i]+raoffset,360.);
	getchunk(currra,dec1[i],&rachunk,&decchunk,rabounds,decbounds,nra,ndec);
	jmax = nchunk2[decchunk][rachunk];
//if (jmax>0) printf("i, jmax %d %d\n", i, jmax);
	if (jmax > 0) {
	  rr = DEG2RAD*ra1[i];
	  dr = DEG2RAD*dec1[i];
	  myx1 = cos(rr)*cos(dr);
	  myy1 = sin(rr)*cos(dr);
	  myz1 = sin(dr);
	  for (j = 0; j < jmax; j++) {
		k = chunklist2[decchunk][rachunk][j];
		sep = separation(myx1, myy1, myz1, xx2[k], yy2[k], zz2[k]);
//printf("*nmatch j k myx1,myy1,myz1,xx2[k],yy2[k],zz2[k]\n %d %d %d %f %f %f %f %f %f \n",
// *nmatch, j, k, myx1,myy1,myz1,xx2[k],yy2[k],zz2[k]);
//printf("sep = %f\n", sep);
		if (sep < matchlength) {
		  if (maxmatch > (*nmatch)) {
			match1.push_back(i);
			match2.push_back(k);
			distance12.push_back(sep * 3600.);
		  }
		  (*nmatch)++;
	       } /* end if sep */
	  } /* end for j */
	} /* end if jmax > 0 */
  } /* end for i */
	 
/* 4. clean up after chunks */
  unassignchunks(&nchunk2, &chunklist2, nra, ndec);
  unsetchunks(&rabounds, &decbounds, &nra, &ndec);

/* 6. free memory */
   free_memory();

  return 0;
}
