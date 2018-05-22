/* LN modif: 8/10/2014
 
  Note:
   - This version filters multiple matches within the search distance returning only the closest one.
   - If the max number of matches is negative, then set it to the max possible (NxM).
   - See function spherematch2_mm below for multiple matches.

  Last change: 17 March 2015 
 */

#include <cstdlib>
#include <cmath>
#include <vector>
#include "chunks.h"
#include <iostream>
#include <algorithm>    // std::sort

using namespace std;

struct data { 
    long id;
    size_t index;
};

struct by_id { 
    bool operator()(data const &left, data const &right) { 
        return (left.id < right.id);
    }
};
struct by_index { 
    bool operator()(data const &left, data const &right) { 
        return (left.index < right.index);
    }
};


static long *nra=NULL, ndec;
static double **rabounds=NULL, *decbounds=NULL;
static double raoffset;
static double *xx2=NULL,*yy2=NULL,*zz2=NULL;
static long **nchunk2=NULL, ***chunklist2=NULL; 


extern "C"
double separation(double xx1, double yy1, double zz1, double xx2, double yy2, double zz2);

#define FREEVEC(a) {if((a)!=NULL) free((char *) (a)); (a)=NULL;}

static void free_memory()
{
	xx2=NULL;
	yy2=NULL;
	zz2=NULL;
	if(nchunk2!=NULL) 
		unassignchunks(&nchunk2,&chunklist2,nra,ndec);
	if(rabounds!=NULL)
		unsetchunks(&rabounds,&decbounds,&nra,&ndec);
	nchunk2=NULL;
	chunklist2=NULL;
	rabounds=NULL;
	decbounds=NULL;
	nra=NULL;
}


/********************************************************************/

long spherematch2(
   long npoints1, double *ra1, double *dec1,
   long npoints2, double *ra2, double *dec2,
   double matchlength, double minchunksize,
   vector<long> &match1, vector<long> &match2, vector<double> &distance12,
   long *nmatch )
{

  if (npoints1 <= 0 || npoints2 <= 0) {
cout<<"Spherematch2: Error: npoints1 = "<< npoints1 <<", npoints2 = "<< npoints2 <<endl;
    return -1;
  }

  if (matchlength <= 0 || minchunksize <= 0) {
cout<<"Spherematch2: Error: matchlength = "<< matchlength <<", minchunksize = "<< minchunksize <<endl;
    return -2;
  }

  double myx1, myy1, myz1, currra, sep, minsep, rr, dr;
  long maxmatch, jmax, i, j, k, i1, k1, rachunk, decchunk, retval=1;

  match1.clear();
  match2.clear();
  distance12.clear();

  std::vector<long> id1, id2;
  std::vector<double> d12;
  id1.clear();
  id2.clear();
  d12.clear();

/* 1. define chunks */
  setchunks(ra1,dec1,npoints1,minchunksize,&rabounds,&decbounds,&nra,&ndec,&raoffset);

/* 2. assign targets to chunks, with minFibreSpacing of leeway */
  assignchunks(ra2,dec2,npoints2,raoffset,matchlength,minchunksize,&nchunk2,&chunklist2,rabounds,decbounds,nra,ndec);

/* 3. make x, y, z coords -- compute (x,y,z)1 on the fly - DPF */
  xx2 = (double *) malloc(npoints2 * sizeof(double));
  yy2 = (double *) malloc(npoints2 * sizeof(double));
  zz2 = (double *) malloc(npoints2 * sizeof(double));
 
  for (i=0; i<npoints2; i++) {
    rr = DEG2RAD*ra2[i];
    dr = DEG2RAD*dec2[i];
    xx2[i]=cos(rr)*cos(dr);
    yy2[i]=sin(rr)*cos(dr);
    zz2[i]=sin(dr);
  } /* end for i */

/* 4. run matching */
  maxmatch = (*nmatch);  /* if nmatch != 0 then fill arrays up to maxmatch */
  (*nmatch) = 0;

  if (maxmatch < 0)  /* if nmatch < 0 then fill arrays up to max number of matches */
    maxmatch = npoints1*npoints2;

  for (i=0; i<npoints1; i++) {
//printf("i= %d %f %f\n", i, ra1[i],dec1[i]);
    currra = fmod(ra1[i]+raoffset, 360.);
    getchunk(currra,dec1[i],&rachunk,&decchunk,rabounds,decbounds,nra,ndec);
    jmax = nchunk2[decchunk][rachunk];
//if (jmax>0) printf("i, jmax nra ndec  ra1 dec1: %d %d %d %d  %f %f\n", i, jmax, *nra, ndec, ra1[i],dec1[i]);
    if (jmax > 0) {
      rr = DEG2RAD*ra1[i];
      dr = DEG2RAD*dec1[i];
      myx1 = cos(rr)*cos(dr);
      myy1 = sin(rr)*cos(dr);
      myz1 = sin(dr);
      for(j=0; j<jmax; j++) {
        k = chunklist2[decchunk][rachunk][j];
        sep = separation(myx1,myy1,myz1,xx2[k],yy2[k],zz2[k]);

        if (j == 0) {
          minsep = sep;
          i1 = i;
          //j1 = 0;
          k1 = k;
        } else {
          if (sep<minsep) {
            minsep = sep;
            i1 = i;
            k1 = k;
          }
        }

      } /* end for j */


      if (minsep < matchlength) {
        if (maxmatch > (*nmatch)) {
          if (jmax > 1) {  // keep only those within max distance
            id1.push_back(i1);
            id2.push_back(k1);
            d12.push_back(minsep);
//cout<<"jmax, i, k, minsep: "<<jmax<<"  "<<i1<<" "<<k1<<" "<<minsep*3600<<endl;
          }
          match1.push_back(i1);
          match2.push_back(k1);
          distance12.push_back(minsep);
        }
        (*nmatch)++;
      }

    } /* end if jmax>0 */

  } /* end for i */

// Debug: list for more than 1 object within given radius
  //long nmul = id1.size();

#ifdef DEBUG
  if (nmul > 0) {
    for (i=0; i<nmul; i++)
      cout <<id1[i]<<" "<<id2[i]<<" "<<d12[i]*3600 <<endl;
  }
#endif

/* 4. clean up after chunks */
  unassignchunks(&nchunk2,&chunklist2,nra,ndec);
  unsetchunks(&rabounds,&decbounds,&nra,&ndec);

/* 6. free memory */
  free_memory();


  //if (match2.size() > 1) {
  if (*nmatch > 1) {

    long ndup=0, n = *nmatch;
    std::vector<data> m2(n);  // temporary array

    for (i=0; i<n; i++) {
      m2[i].index = i;
      m2[i].id = match2[i];
//cout <<"i: "<<i<<" match1[i]: "<<match1[i]<<" m2[i].id: "<<m2[i].id<<" d: "<<distance12[i]*3600 <<endl;
    }

    sort(m2.begin(), m2.end(), by_id());

    long ij=0, dup_id=0;
    double dmin=361;

    for (i=1; i<n; i++) {
// Duplicated reference objects
      if (m2[i-1].id == m2[i].id || m2[i].id == dup_id) {
        if (dup_id == -1) {
          ij = i - 1;
          dmin = distance12[m2[ij].index];
          dup_id = m2[i].id;
        }
        ndup++;

        if (distance12[m2[i].index] < dmin)  // following object is closer
        {
          dmin = distance12[m2[i].index];
          m2[ij].id = -1;
          ij = i;
/*
          for (j=0; j<nmul; j++) {

            if (id1[j] == match1[m2[i1].index] && id2[j] != m2[i].id) {

              match2[m2[i1].index] = id2[j];
              distance12[m2[i1].index] = d12[j];
              break;
            }
          }
          if (j == nmul) m2[i1].id = -1;
*/

/* Example (sort gives random order):
  172 172 0.0100000
  172 173 0.0090536
  173 172 0.0110439
  173 173 0.0100000

  172 172 0.0100000
  173 172 0.0110439
  173 173 0.0100000
  172 173 0.0090536
which based on min distance will select
  173 173 0.0100000
  172 173 0.0090536
*/
        } else {
          m2[i].id = -1;

/* Example:
   id1                             id2
   70   0.8224558   -1.0018456     70   0.8224558   -1.0018428    0.0100000
   70   0.8224558   -1.0018456     71   0.8224556   -1.0018431    0.0090554
   71   0.8224556   -1.0018458     70   0.8224558   -1.0018428    0.0110453
   71   0.8224556   -1.0018458     71   0.8224556   -1.0018431    0.0100000

which based on min distance will select
  match1  match2
  70      71     0.0090554
  71      71     0.0100000
so 71 is a duplicated ref. catalogue ID
*/
/*
          i1 = i;
          for (j=0; j<nmul; j++) {

            if (id1[j] == match1[m2[i1].index] && id2[j] != m2[i].id) {
// 71 && 70 
              match2[m2[i1].index] = id2[j];
// 71 -> 70
              distance12[m2[i1].index] = d12[j];
              break;
            }
          }
          if (j == nmul) m2[i].id = -1;
*/
        }  // end else

      } else {  // end if
        ij = 1;
        dup_id = -1;
        dmin = 361;
      }

    }  // end for i<n

    if (ndup > 0) {
      sort(m2.begin(), m2.end(), by_index());
      i1=0;
      for (i=0; i<n; i++) {
        if (m2[i].id == -1) {
          match1.erase(match1.begin()+m2[i].index -i1);
          match2.erase(match2.begin()+m2[i].index -i1);
          distance12.erase(distance12.begin()+m2[i].index -i1);
          (*nmatch)--;
          i1++;
        }
      }
    }

    if (*nmatch > npoints1 || *nmatch > npoints2) {
cout<<"Spherematch2: Error: more matched objects than input objects:\n"<<
      "Nmatch="<<*nmatch<<"  npoints1 = "<< npoints1 <<", npoints2 = "<< npoints2 <<endl;
      return -3;
    }


  } // if *nmatch > 1


   return retval;
}
/******************************************************************************/

/* LN modif

  Note: this version allows for multiple matches within the search distance.
        However the maximum number of matches is not managed accordingly.

 */

/********************************************************************/

long spherematch2_mm(
   long   npoints1,
   double *ra1,
   double *dec1,
   long   npoints2,
   double *ra2,
   double *dec2,
   double matchlength,
   double minchunksize,
   vector<long>   &match1,
   vector<long>   &match2,
   vector<double> &distance12,
   long   *nmatch )
{

  double myx1,myy1,myz1;
  long maxmatch, jmax;
  double currra,sep;
  long i,j,k,rachunk,decchunk;
  long retval=1;

  match1.clear();
  match2.clear();
  distance12.clear();


	 /* 1. define chunks */
	 setchunks(ra1,dec1,npoints1,minchunksize,&rabounds,
						 &decbounds,&nra,&ndec,&raoffset);

	 /* 2. assign targets to chunks, with minFibreSpacing of leeway */
	 assignchunks(ra2,dec2,npoints2,raoffset,matchlength,minchunksize,&nchunk2,
		      &chunklist2,rabounds,decbounds,nra,ndec);

	 /* 3. make x, y, z coords -- compute (x,y,z)1 on the fly - DPF */
	 xx2=(double *) malloc(npoints2 * sizeof(double));
	 yy2=(double *) malloc(npoints2 * sizeof(double));
	 zz2=(double *) malloc(npoints2 * sizeof(double));
         double rr, dr;
	 for(i=0;i<npoints2;i++) {
                 rr = DEG2RAD*ra2[i];
                 dr = DEG2RAD*dec2[i];
		 xx2[i]=cos(rr)*cos(dr);
		 yy2[i]=sin(rr)*cos(dr);
		 zz2[i]=sin(dr);
	 } /* end for i */

	 /* 4. run matching */
	 maxmatch = (*nmatch);  /* if nmatch != 0 then fill arrays up to maxmatch */
	 (*nmatch)=0;
	 for(i=0;i<npoints1;i++) {
	   currra=fmod(ra1[i]+raoffset,360.);
	   getchunk(currra,dec1[i],&rachunk,&decchunk,rabounds,decbounds,nra,ndec);
	   jmax=nchunk2[decchunk][rachunk];
//if (jmax>0) printf("i, jmax %d %d\n", i, jmax);
	   if(jmax>0) {
             rr = DEG2RAD*ra1[i];
             dr = DEG2RAD*dec1[i];
	     myx1=cos(rr)*cos(dr);
	     myy1=sin(rr)*cos(dr);
	     myz1=sin(dr);
	     for(j=0;j<jmax;j++) {
	       k=chunklist2[decchunk][rachunk][j];
	       sep=separation(myx1,myy1,myz1,xx2[k],yy2[k],zz2[k]);
//printf("*nmatch j k myx1,myy1,myz1,xx2[k],yy2[k],zz2[k]\n %d %d %d %f %f %f %f %f %f \n",
// *nmatch, j, k, myx1,myy1,myz1,xx2[k],yy2[k],zz2[k]);
//printf("sep = %f\n", sep);
	       if(sep<matchlength) {
		 if(maxmatch>(*nmatch)) {
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
	 unassignchunks(&nchunk2,&chunklist2,nra,ndec);
	 unsetchunks(&rabounds,&decbounds,&nra,&ndec);

	 /* 6. free memory */
	 free_memory();

   return retval;
}

/******************************************************************************/
