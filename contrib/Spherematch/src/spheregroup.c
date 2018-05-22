#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "chunks.h"
#include "friendsoffriends.h"

static long *nra=NULL, ndec;
static double **rabounds=NULL, *decbounds=NULL;
static double raoffset;
static long **nchunk=NULL, ***chunklist=NULL; 
static long *renumbered=NULL;
static double *x=NULL,*y=NULL,*z=NULL;
static long *firstgroup=NULL,*nextgroup=NULL,*multgroup=NULL;
static long ngroups;

#define FREEVEC(a) {if((a)!=NULL) free((char *) (a)); (a)=NULL;}
static void free_memory()
{
	FREEVEC(x);
	FREEVEC(y);
	FREEVEC(z);
	FREEVEC(firstgroup);
	FREEVEC(nextgroup);
	FREEVEC(multgroup);
	FREEVEC(renumbered);
	if(nchunk!=NULL) 
		unassignchunks(&nchunk,&chunklist,nra,ndec);
	if(rabounds!=NULL)
		unsetchunks(&rabounds,&decbounds,&nra,&ndec);
}


/********************************************************************/
long spheregroup
  (int      argc,
   void *   argv[])
{
   long    npoints;
   double    *  ravec;
   double    *  decvec;
   double    linklength;
   double    minchunksize;
	 long *ingroup;

	 long i,j,iclump;
	 long retval=1;

   /* 0. allocate pointers from IDL */
   npoints = *((long *)argv[0]);
   ravec = (double *)argv[1];
   decvec = (double *)argv[2];
   linklength = *(double *)argv[3];
   minchunksize = *(double *)argv[4];
   ingroup = (long *)argv[5];

	 /* 1. define chunks */
	 setchunks(ravec,decvec,npoints,minchunksize,&rabounds,
						 &decbounds,&nra,&ndec,&raoffset);

	 /* 2. assign targets to chunks, with minFibreSpacing of leeway */
	 assignchunks(ravec,decvec,npoints,raoffset,linklength,
								minchunksize,&nchunk,&chunklist,rabounds,decbounds,
								nra,ndec);

	 /* 3. make x, y, z coords */
	 x=(double *) malloc(npoints*sizeof(double));
	 y=(double *) malloc(npoints*sizeof(double));
	 z=(double *) malloc(npoints*sizeof(double));
	 for(i=0;i<npoints;i++) {
		 x[i]=cos(DEG2RAD*ravec[i])*cos(DEG2RAD*decvec[i]);
		 y[i]=sin(DEG2RAD*ravec[i])*cos(DEG2RAD*decvec[i]);
		 z[i]=sin(DEG2RAD*decvec[i]);
	 } /* end for i */

	 /* 4. run fof */
	 firstgroup=(long *) malloc(npoints*sizeof(long));
	 multgroup=(long *) malloc(npoints*sizeof(long));
	 nextgroup=(long *) malloc(npoints*sizeof(long));
	 if(!friendsoffriends(x,y,z,npoints,linklength,nchunk,chunklist,
												nra,ndec,firstgroup,multgroup,nextgroup,
												ingroup,&ngroups)) {
		 printf("friendsoffriends returned error in spheregroup()\n");
		 free_memory();
		 return(0);
	 } /* end if */

	 /* 4. clean up after chunks */
	 unassignchunks(&nchunk,&chunklist,nra,ndec);
	 unsetchunks(&rabounds,&decbounds,&nra,&ndec);

	 /* 5a. renumber the groups in order of appearance in list */
	 renumbered=(long *) malloc(npoints*sizeof(long));
	 for(i=0;i<npoints;i++) 
		 renumbered[i]=0;
	 iclump=0;
	 for(i=0;i<npoints;i++) {
		 if(!renumbered[i]) {
			 for(j=firstgroup[ingroup[i]];j!=-1;j=nextgroup[j]) {
				 ingroup[j]=iclump;
				 renumbered[j]=1;
			 } /* end for j */
			 iclump++;
		 } /* end if */
	 } /* end for i */
	 FREEVEC(renumbered);
	 
	 /* 5b. reset the index lists */
	 for(i=0;i<npoints;i++) 
		 firstgroup[i]=-1;
	 for(i=npoints-1;i>=0;i--) {
		 nextgroup[i]=firstgroup[ingroup[i]];
		 firstgroup[ingroup[i]]=i;
	 } /* end for i */
	 
	 /* 5c. reset the multiplicities */
	 for(i=0;i<ngroups;i++) {
		 multgroup[i]=0;
		 for(j=firstgroup[i];j!=-1;j=nextgroup[j]) {
			 multgroup[i]++;
		 } /* end for j */
	 } /* end for i */
	 
	 /* 6. free memory */
	 free_memory();
   return retval;
}

/******************************************************************************/

