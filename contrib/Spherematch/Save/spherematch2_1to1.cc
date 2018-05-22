/* LN modif: 8/10/2014
 
  Note: this version filters multiple matches within the search distance
        returning only the closest one.

 */

#include <cstdlib>
#include <cmath>
#include <vector>
#include "chunks.h"
#include <iostream>

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


static long *nra=NULL, ndec;
static double **rabounds=NULL, *decbounds=NULL;
static double raoffset;
static double *xx2=NULL,*yy2=NULL,*zz2=NULL;
static long **nchunk2=NULL, ***chunklist2=NULL; 


extern "C"
double separation(double xx1, double yy1, double zz1, double xx2, double yy2,
								double zz2);

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
  double currra,sep, minsep;
  long i,j,k,rachunk,decchunk;
  long retval=1;

  match1.clear();
  match2.clear();
  distance12.clear();

  std::vector<long> id1, id2;
  std::vector<double> d12;
  id1.clear();
  id2.clear();
  d12.clear();

  long i1, k1;

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
//printf("i= %d %f %f\n", i, ra1[i],dec1[i]);
	   currra=fmod(ra1[i]+raoffset,360.);
	   getchunk(currra,dec1[i],&rachunk,&decchunk,rabounds,decbounds,nra,ndec);
	   jmax=nchunk2[decchunk][rachunk];
//if (jmax>0) printf("i, jmax nra ndec  ra1 dec1: %d %d %d %d  %f %f\n", i, jmax, *nra, ndec, ra1[i],dec1[i]);
	   if(jmax>0) {
             rr = DEG2RAD*ra1[i];
             dr = DEG2RAD*dec1[i];
	     myx1=cos(rr)*cos(dr);
	     myy1=sin(rr)*cos(dr);
	     myz1=sin(dr);
	     for(j=0;j<jmax;j++) {
	       k=chunklist2[decchunk][rachunk][j];
	       sep=separation(myx1,myy1,myz1,xx2[k],yy2[k],zz2[k]);

               if (j==0) {
                 minsep = sep;
                 i1=i;
                 k1=k;
               } else {
                 if (sep<minsep) {
                   minsep=sep;
                   i1 = i;
                   k1 = k;
                 }
               }
// Debug: this is for debugging but could be used for other purposes
////if (jmax>1 && sep<matchlength){
if (jmax>1){
  id1.push_back(i);
  id2.push_back(k);
  d12.push_back(sep*3600);
}

//printf("*nmatch j k myx1,myy1,myz1,xx2[k],yy2[k],zz2[k]\n %d %d %d %f %f %f %f %f %f \n",
// *nmatch, j, k, myx1,myy1,myz1,xx2[k],yy2[k],zz2[k]);
//printf("sep = %f\n", sep);
	     } /* end for j */

             if(minsep<matchlength) {
               if(maxmatch>(*nmatch)) {
                  match1.push_back(i1);
                  match2.push_back(k1);
                  distance12.push_back(minsep);
               }
               (*nmatch)++;
             }

           } /* end if jmax>0 */

         } /* end for i */

// Debug: list for more than 1 object within given radius
  for (size_t i=0; i<id1.size(); i++)
    cout <<id1[i]<<" "<<id2[i]<<" "<<d12[i] <<endl;

	 /* 4. clean up after chunks */
	 unassignchunks(&nchunk2,&chunklist2,nra,ndec);
	 unsetchunks(&rabounds,&decbounds,&nra,&ndec);

	 /* 6. free memory */
	 free_memory();


  if (match2.size() > 1) {

    long n = match2.size();
    std::vector<data> idu(n);

    for (i=0; i<n; i++) {
      idu[i].index = i;
      idu[i].id = match2[i];
    }

    sort(idu.begin(), idu.end(), by_id());

    for (i=1; i<n; i++) {
// Duplicated reference objects
      if (idu[i-1].id == idu[i].id) {
        if ( distance12[idu[i].index]<distance12[idu[i-1].index] )
          idu[i-1].id = -1;
        else
          idu[1].id = -1;

cout<<"Spherematch2: duplicated reference entry: "<< idu[i].id <<" "<<ra2[idu[i].id]<<" "<<dec2[idu[i].id]
    <<" d1="<< distance12[idu[i-1].index]*3600 <<" d2="<<distance12[idu[i].index]*3600 <<endl;
      } 
    }

    i1=0;
    for (i=0; i<n; i++) {
      if (idu[i].id == -1) {
        match1.erase(match1.begin()+idu[i].index -i1);
        match2.erase(match2.begin()+idu[i].index -i1);
        distance12.erase(distance12.begin()+idu[i].index -i1);
        (*nmatch)--;
        i1++;
      }
    }

  } // if match2.size() > 1


   return retval;
}

/******************************************************************************/
