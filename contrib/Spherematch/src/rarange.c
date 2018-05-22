#include <stdio.h>
#include <string.h>
#include <math.h>
#include "chunks.h"

/*
 * This file deals with the fact that I put things between 0. and 360. in ra. 
 * raoffset returns the offset which yields the smallest raRange, and 
 * also returns that range.
 *
 * Michael Blanton
 * 8/2000 */

#define NRA 6
#define EPS 1.e-5

long 
rarange(double ra[],         /* degrees */
				long nPoints,
				double minSize,       /* degrees */
				double *raRangeMin,
				double *raOffset)
{
	long result;
	long j;
	double raMax,raMin,raRange;

	/* Find ra offset which minimizes the range
	 * in ra (this should take care of the case
	 * that ra crosses zero in some parts */
	(*raRangeMin)=361.; 
	(*raOffset)=0.;
	for(j=0;j<NRA;j++) {
		
		/* get range */
		result=getraminmax(ra,360.*(double)j/(double)NRA,nPoints,&raMin,&raMax);
		if(!result) return(result);
		raRange=raMax-raMin;

		/* check if it is minimum, but protect against roundoff error; 
		 * and don't change it if we are going to be too close to the edge 
		 * anyway */
		if(2.*(raRange-(*raRangeMin))/(raRange+(*raRangeMin))<-EPS 
			 && raMin>minSize
			 && raMax<360.-minSize) {
			(*raRangeMin)=raRange;
			(*raOffset)=360.*(double)j/(double)NRA;
		} /* end if */
	} /* end for j */

	return(1);
} /* end tiRaOffset */ 

long
getraminmax(double ra[], 
						double raOffset,
						long nPoints, 
						double *raMin, 
						double *raMax) 
{
	double currRa;
	long i;
	
	(*raMax)=-1.;
	(*raMin)=361.;
	for(i=0;i<nPoints;i++) {
		currRa=fmod(ra[i]+raOffset,360.);
		(*raMin)=(currRa<(*raMin)) ? currRa : (*raMin);
		(*raMax)=(currRa>(*raMax)) ? currRa : (*raMax);
	} /* end for i */
	
	return(1);
} /* end tiGetRaMinMax */
