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

struct multi { 
    long id1;
    long id2;
    double d12;
    size_t index;
};

struct m2data { 
    long id;
    size_t index;
};

struct m_by_id2 { 
    bool operator()(multi const &left, multi const &right) { 
        return (left.id2 < right.id2 && left.d12<right.d12);
    }
};

struct m2_by_id { 
    bool operator()(m2data const &left, m2data const &right) { 
        return (left.id < right.id);
    }
};
struct m2_by_index { 
    bool operator()(m2data const &left, m2data const &right) { 
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
  long maxmatch, jmax, i, j, k, i1, k1, rachunk, decchunk, retval=1;  //j1

  match1.clear();
  match2.clear();
  distance12.clear();

  //std::vector<long> id1, id2;
  //std::vector<double> d12;
  multi im;
  std::vector<multi> m;
  m.clear();
  //id1.clear();
  //id2.clear();
  //d12.clear();

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

  size_t ii = 0;
  int j1;

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
          j1 = 0;
          k1 = k;
        } else {
          if (sep<minsep) {
            minsep = sep;
            i1 = i;
            k1 = k;
          }
        }

// This is required to manage multiple matches in ref catalogue
        if (jmax > 1 && sep <= matchlength) {  // keep only those within max distance

            im.id1=i;
            im.id2=k;
            im.d12=sep;
            im.index=ii;
            m.push_back(im);
            ii++;
            j1++;
cout<<"***jmax, j1,  i, k, j, sep, minsep: "<<jmax<<" "<<j1<<"  "<<i<<" "<<k<<" "<<j<<" "<<sep*3600<<" "<<minsep*3600<<endl;
        }

//printf("*nmatch j k myx1,myy1,myz1,xx2[k],yy2[k],zz2[k]\n %d %d %d %f %f %f %f %f %f \n",
// *nmatch, j, k, myx1,myy1,myz1,xx2[k],yy2[k],zz2[k]);
//printf("sep = %f\n", sep);

      } /* end for j */

// Manage multi-matches: remove single matches within max distance
      if (j1 == 1) {
        m.pop_back(); 
        ii--;
        //id1.pop_back(); 
        //id2.pop_back(); 
        //d12.pop_back(); 
      }
//if (j1>0) {
//cout<<"***j1: "<<j1<<endl;
      j1 = 0;
//}

      if (minsep < matchlength) {
        if (maxmatch > (*nmatch)) {
//if (jmax > 1)
//cout<<"jmax, i1, k1, minsep: "<<jmax<<"  "<<i1<<" "<<k1<<" "<<minsep*3600<<endl;

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
  long nmul = m.size();

#ifdef DEBUG
  if (nmul > 0) {
    for (i=0; i<nmul; i++)
      cout <<m[i].index<<" "<<m[i].id1<<" "<<m[i].id2<<" "<<m[i].d12*3600 <<endl;
      //cout <<index[i]<<" "<<id1[i]<<" "<<id2[i]<<" "<<d12[i]*3600 <<endl;
  }
#endif

/* 4. clean up after chunks */
  unassignchunks(&nchunk2,&chunklist2,nra,ndec);
  unsetchunks(&rabounds,&decbounds,&nra,&ndec);

/* 6. free memory */
  free_memory();


cout<<"Nmatch="<<*nmatch<<endl;

  //if (match2.size() > 1) {
  if (*nmatch > 1) {

    long ndup=0, n = *nmatch;
    std::vector<m2data> m2(n);  // temporary array

    for (i=0; i<n; i++) {
      m2[i].index = i;
      m2[i].id = match2[i];
//cout <<"i: "<<i<<"  match1[i]: "<<match1[i]<<" m2[i].id: "<<m2[i].id<<" d: "<<distance12[i]*3600 <<endl;
    }

    sort(m2.begin(), m2.end(), m2_by_id());

/*
cout<<"\nTemp array after ID sorting:\n";
for (i=0; i<n; i++)
cout <<"i: "<<i<<" m2[i].index: "<<m2[i].index<<" match1[m2[i].index]: "<<match1[m2[i].index]<<" m2[i].id: "<<m2[i].id<<" d: "<<distance12[m2[i].index]*3600<<endl;
cout<<endl;
*/

    i=1;
    while (i<n) {
      if (m2[i].id != m2[i-1].id) {
        for (int ik = 0; ik<m.size(); ik++) {
          if (m[ik].id2 == m2[i-1].id) m[ik].id2 = -1;
}
      } else i++;
i++;
    }


    long ij=0, dup_id=-1;
    double dmin=361;

    for (i=1; i<n; i++) {
      //if (m2[i-1].id == -1) continue;
//cout<<"ij, i, m2[ij].id, m2[i].id, dup_id: "<<ij<<" "<<i<<" "<<m2[ij].id<<" "<<m2[i].id<<" "<<dup_id<<endl;

// Duplicated reference objects
      if (m2[ij].id == m2[i].id || m2[i].id == dup_id) {
        if (dup_id == -1) {
          ij = i - 1;
          dmin = distance12[m2[ij].index];
          dup_id = m2[i].id;
        }
        ndup++; 

cout<<"\n***i: "<<i<<" ndup="<<ndup<<" refcat dup_id="<<dup_id
              <<" incat ids: "<<match1[m2[ij].index]<<", "<<match1[m2[i].index]<<endl;
cout<<"Spherematch2: duplicated reference entry: m2[i].id="<< m2[i].id <<"  "<<ra2[m2[i].id]<<" "<<dec2[m2[i].id]
    <<" d1="<< distance12[m2[ij].index]*3600 <<" d2="<< distance12[m2[i].index]*3600 <<endl;

        if (distance12[m2[i].index] < dmin)  // following object is closer
        {
cout<<"Following duplicated object is closer.\n";
          dmin = distance12[m2[i].index];
          //m2[ij].id = -1;
          //ij = i;
          for (j=0; j<nmul; j++) {

            if (m[j].id1 == match1[m2[ij].index] && m[j].id2 != m2[i].id && m[j].id2 != -1) {

cout<<"Swapping ids: Old refcat id="<<match2[m2[ij].index]<<" d="<<distance12[m2[ij].index]*3600;
              match2[m2[ij].index] = m[j].id2;
              distance12[m2[ij].index] = m[j].d12;
cout<<" New refcat id="<<m[j].id2<<" d="<<m[j].d12*3600<<endl;
              break;
            }
          }
          if (j == nmul) { m2[ij].id = -1; ij = i;}


        } else {
cout<<"Previous duplicated object is closer.\n";

          for (j=0; j<nmul; j++) {

            if (m[j].id1 == match1[m2[i].index] && m[j].id2 != m2[i].id && m[j].id2 != -1) {
// 71 && 70 
cout<<"Swapping ids: Old refcat id="<<match2[m2[i].index]<<" d="<<distance12[m2[i].index]*3600;
              match2[m2[i].index] = m[j].id2;
// 71 -> 70
              distance12[m2[i].index] = m[j].d12;
//cout<<" New refcat id="<<m[j].id2<<" d="<<m[j].d12*3600<<endl;
cout<<" New refcat id="<<m[j].id2<<" d="<<m[j].d12*3600<<endl;
              break;
            }
          }
          if (j == nmul) {
            m2[i].id = -1;
          }
            //m2[i].id = -1;
        }

cout<<"dmin="<<dmin*3600<<" dcur="<< distance12[m2[i].index]*3600<<endl;
      } else {  // end if dup
        ij = i;
        dup_id = -1;
        dmin = 361;
      }
    }  // end for i<n


    if (ndup > 0) {
      sort(m2.begin(), m2.end(), m2_by_index());
      i1=0;
      for (i=0; i<n; i++) {
        if (m2[i].id == -1) {
cout<<"erase i: "<<i<<" i1: "<<i1<<" m2[i].index: "<<m2[i].index<<" m2[i].index - i1: "<<m2[i].index - i1 <<endl;
cout<<"match1: "<<match1[m2[i].index -i1]<<" match2: "<<match2[m2[i].index -i1] <<endl;
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
