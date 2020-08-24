/* LN modif: 8/10/2014
 
  Note:
   - This version filters multiple matches within the search distance returning only the closest one.
   - If the max number of matches is negative, then set it to the max possible (NxM).
   - See function spherematch2_mm below for multiple matches.

  Last change: 21 June 2020 
 */

#include <cstdlib>
#include <cmath>
#include <vector>
#include "chunks.h"
#include <iostream>
#include <algorithm>    // std::sort

using namespace std;

struct multi { 
    long id1;
    long id2;
    double d12;
};


struct m_by_id2 { 
    bool operator()(multi const &left, multi const &right) { 
        return (left.id2 < right.id2 && left.d12 < right.d12);
    }
};

struct m_by_d12 {
    bool operator()(multi const &left, multi const &right) {
        return (left.d12 < right.d12);
    }
};


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
   vector<long> &match1s, vector<long> &match2s, vector<double> &distance12s,
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
  long maxmatch, rachunk, decchunk, retval=1;
  unsigned long i, j, k, jmax, j1; //, i1, k1;

  std::vector<long> refcount;
  refcount.clear();

  match1s.clear();
  match2s.clear();
  distance12s.clear();

// Erasing duplicated entries is quite time consuming.
// Use temp vectors and tranfer unique entries after cleaning.
  std::vector<long> match1;
  std::vector<long> match2;
  std::vector<double> distance12;

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
  xx2 = (double *) malloc(npoints2 * sizeof(double));
  yy2 = (double *) malloc(npoints2 * sizeof(double));
  zz2 = (double *) malloc(npoints2 * sizeof(double));
 
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
    currra = fmod(ra1[i]+raoffset, 360.);
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
/*
	if (j == 0) {
	  minsep = sep;
          //i1 = i;
          //j1 = 0;
          //k1 = k;
	} else {
	  if (sep < minsep) {
	    minsep = sep;
            //i1 = i;
            //k1 = k;
	  }
	}
*/
// This is required to manage multiple matches in ref catalogue
        //if (jmax > 1 && sep <= matchlength) {  // keep only those within max distance
	if (sep <= matchlength && (*nmatch) <= (unsigned long)maxmatch) {  // keep only those within max distance
		j1++;
		im.id1 = i;
		im.id2 = k;
		im.d12 = sep;
		m.push_back(im);

		match1.push_back(i);
		match2.push_back(k);
		distance12.push_back(sep);

		(*nmatch)++;
	}

      } /* end for j */


/*
// Manage multi-matches: remove single matches within max distance
      if (j1 == 1) {
//cout<<"Removing "<<m[ii-1].id1<<" "<< m[ii-1].id2<<" "<< m[ii-1].d12*3600<<endl;
        m.pop_back(); 
        ii--;
      }
*/
//cout<<"i1, k1: "<< i1 <<", "<< k1 << endl<<endl;


    } /* end if jmax>0 */
      if (j1 > 0) {
	for (j = 0; j < j1; j++)
	  refcount.push_back(j1);
	j1 = 0;
      }

  } /* end for i */
//cout<< "N refcount: "<< refcount.size() << endl;

//for (i = 0; i < *nmatch; i++)
//cout<<"i: "<<i<<" match1: "<<match1[i]<<" match2: "<<match2[i]<<", d: "<< distance12[i]*3600 <<endl;

/* 4. clean up after chunks */
  unassignchunks(&nchunk2, &chunklist2, nra, ndec);
  unsetchunks(&rabounds, &decbounds, &nra, &ndec);

/* 6. free memory */
  free_memory();


//
// Debug: list for more than 1 object within given radius
//  unsigned long nmul = m.size();
//cout<<"nmul="<<nmul<<endl;

cout<<"Total (multi) Nmatch = "<< *nmatch << endl;


#ifdef DEBUG
if (*nmatch > 0) {
  for (i = 0; i < *nmatch; i++)
      cout <<"i: "<< i <<", refcount: "<< refcount[i] <<", id1: "<<m[i].id1<<", id2: "<<m[i].id2<<", d: "<<m[i].d12*3600 <<endl;
}
#endif

  unsigned long n_rm = 0, ij = 0, m_marked = 0, n_unique = 0;

  if (*nmatch > 1) {

// Array with just the multiple matches
    std::vector<multi> mm;
    mm.clear();


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
    }


// Clear old vector
//m.clear();
    std::vector<multi>().swap(m);

// Remove all the InCat objects with a single RefCat counterpart ?

    if (n_unique > 0)
	cout <<"--> "<< n_unique <<" unique matches found\n";

 
#ifdef DEBUG
cout<<"Not unique: "<< mm.size() << endl;
for (i = 0; i < mm.size(); i++)
cout <<"mm[i].id1: "<<mm[i].id1<<", mm[i].id2: "<<mm[i].id2<<", d: "<<mm[i].d12*3600<<endl;
#endif

//sort(m.begin(), m.end(), m_by_d12());
    sort(mm.begin(), mm.end(), m_by_d12());

//cout <<"Sorted mm:\n";
//for (i = 0; i < mm.size(); i++)
//cout <<"mm[i].id1: "<<m[i].id1<<", mm[i].id2: "<<m[i].id2<<", d: "<<m[i].d12*3600<<endl;


// Mark duplicated matches with distance > min
    for (i = 0; i < mm.size(); i++) {
	if (mm[i].d12 == -1)
 	continue;

	for (ij = i + 1; ij < mm.size(); ij++) {

	  if (mm[ij].id1 == mm[i].id1 || mm[ij].id2 == mm[i].id2) {
		mm[ij].d12 = -1;
		m_marked++;
        	continue;
	  }
	}
    }

//cout <<"Marked ("<< m_marked <<") m:\n";

    if (m_marked > 0) {

	for (i = 0; i < mm.size(); i++) {

	if (mm[i].d12 == -1) {

	  for (ij = 0; ij < match1.size(); ij++) {

	    if (match1[ij] == mm[i].id1 && match2[ij] == mm[i].id2) {
//cout<<"Erase match1: "<< match1[ij] <<" match2: "<< match2[ij] <<", d: "<< distance12[ij]*3600 <<endl<<endl;

// This is quite time consuming. Better copy unmarked values into the passed vectors.
              //match1.erase(match1.begin() + ij);
              //match2.erase(match2.begin() + ij);
              //distance12.erase(distance12.begin() + ij);
              //(*nmatch)--;
		distance12[ij] = -1;
		n_rm++;
		continue;

            }

          }  // end for ij

        }  // mm[i].d12 < 0

      }  // end for i

    }  // end if m_marked > 0


// Transfer final unique values into the passed vectors.
    for (i = 0; i < *nmatch; i++) {
	if (distance12[i] >= 0.) {
//cout<< i <<": "<< match1[i] <<" "<< match2[i] <<" "<< distance12[i] << endl;
		match1s.push_back(match1[i]);  
		match2s.push_back(match2[i]);  
		distance12s.push_back(distance12[i]);  
	}
    }


    *nmatch -= n_rm;

cout<<"--> "<< n_rm <<" duplicated entries removed\n";

//cout<<"Nmatch="<< *nmatch <<"  match1.size() = "<< match1.size() <<", match2.size() = "<< match1.size() <<", distance12.size() = "<< distance12.size()<<endl;

    if (*nmatch > npoints1 || *nmatch > npoints2) {
	cout<<"Spherematch2: Error: more matched objects than input objects:\n"<< 
	"Nmatch="<< *nmatch <<"  npoints1 = "<< npoints1 <<", npoints2 = "<< npoints2 << endl;
	return -3;
    }

    std::vector<multi>().swap(mm);
  } // if *nmatch > 1


  return retval;
}



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
   vector<double> &distance12,
   unsigned long   *nmatch )
{

  double myx1,myy1,myz1;
  unsigned long maxmatch, jmax;
  double currra,sep;
  unsigned long i,j,k;
  long rachunk,decchunk;
  long retval=1;

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
	 xx2=(double *) malloc(npoints2 * sizeof(double));
	 yy2=(double *) malloc(npoints2 * sizeof(double));
	 zz2=(double *) malloc(npoints2 * sizeof(double));
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
	 (*nmatch)=0;
	 for(i = 0; i < npoints1; i++) {
	   currra = fmod(ra1[i]+raoffset,360.);
	   getchunk(currra,dec1[i],&rachunk,&decchunk,rabounds,decbounds,nra,ndec);
	   jmax = nchunk2[decchunk][rachunk];
//if (jmax>0) printf("i, jmax %d %d\n", i, jmax);
	   if(jmax > 0) {
             rr = DEG2RAD*ra1[i];
             dr = DEG2RAD*dec1[i];
	     myx1 = cos(rr)*cos(dr);
	     myy1 = sin(rr)*cos(dr);
	     myz1 = sin(dr);
	     for(j = 0; j < jmax; j++) {
	       k = chunklist2[decchunk][rachunk][j];
	       sep = separation(myx1, myy1, myz1, xx2[k], yy2[k], zz2[k]);
//printf("*nmatch j k myx1,myy1,myz1,xx2[k],yy2[k],zz2[k]\n %d %d %d %f %f %f %f %f %f \n",
// *nmatch, j, k, myx1,myy1,myz1,xx2[k],yy2[k],zz2[k]);
//printf("sep = %f\n", sep);
	       if(sep < matchlength) {
		 if(maxmatch > (*nmatch)) {
		   match1.push_back(i);
		   match2.push_back(k);
		   distance12.push_back(sep);
		 }
		 (*nmatch)++;
	       } /* end if */
	     } /* end for j */
	   } /* end if jmax>0 */
	 } /* end for i */
	 
	 /* 4. clean up after chunks */
	 unassignchunks(&nchunk2, &chunklist2, nra, ndec);
	 unsetchunks(&rabounds, &decbounds, &nra, &ndec);

	 /* 6. free memory */
	 free_memory();

   return retval;
}
