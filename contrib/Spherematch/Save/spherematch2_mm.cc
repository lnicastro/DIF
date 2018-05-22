/* LN modif

  Note: this version allows for multiple matches within the search distance.
        However the maximum number of matches is not managed accordingly.

 */

#include <cstdlib>
#include <cmath>
#include <vector>
#include "chunks.h"

using namespace std;


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
