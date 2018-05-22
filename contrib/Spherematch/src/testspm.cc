/*
   Testing cleaning loop in function spherematch2.

Compile: g++ -o testspm testspm.cc ../lib/libspheregroup.a -DDEBUG

Example queries to create text files:

Input catalogue:
  SELECT DISTINCT htmid_6,RAmas/3.6e6,DECmas/3.6e6 FROM UCAC WHERE htmID_6=45503
  UNION ALL
  SELECT DISTINCT htmid_6,RAmas/3.6e6,DECmas/3.6e6 FROM UCAC_htm_14 WHERE DIF_sNeighb(6,45503,14)
  into outfile '/tmp/u45503.lis'

Reference catalogue:
  SELECT ramas/3.6e6,decmas/3.6e6 from scratch.TYCHO2 WHERE htmid_6=45503
  into outfile '/tmp/t45503.lis';


  Last change: 22 March 2015 
 */

#include <cstdlib>
#include <cmath>
#include <vector>
#include "chunks.h"
#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>    // std::sort
#include <iomanip>

using namespace std;

struct multi { 
    long id1;
    long id2;
    double d12;
    size_t index;
    multi(long i1, long i2, double d, size_t i): id1(i1), id2(i2), d12(d), index(i) {}

    bool operator < (const multi &m) const
    {
      //return ((id2 < m.id2) && (d12 < m.d12));
      if (id2==m.id2) return( d12 < m.d12 ); else return ( id2 < m.id2 );
    }
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
  long maxmatch, jmax, i, j, k, i1, k1, rachunk, decchunk, retval=1;

  match1.clear();
  match2.clear();
  distance12.clear();

  //std::vector<long> id1, id2;
  //std::vector<double> d12;
multi *im;
  //im = new multi(0,0,0,0);
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
  long ndup_in=0, ndup_ref=0, j1;

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

//if (ii!=0) cout<<"PIPPO: i, ii, id1, sep: "<<i<<" "<<ii<<" "<<m[ii-1].id1<<" "<< m[ii-1].d12*3600<<endl;
          //if (ii==0) {
im = new multi(i,k,sep,ii);
            //im.id1=i;
            //im.id2=k;
            //im.d12=sep;
            //im.index=ii;

            m.push_back(*im);
            if (ii == 0) ndup_in = 1;    
            else if (i != m[ii-1].id1) ndup_in++;
            ii++;
            j1++;
          //} else {
            //if (i != m[ii-1].id1) {
              //im.id1=i;
              //im.id2=k;
              //im.d12=sep;
              //im.index=ii;
              //m.push_back(im);
              //ii++;
              //j1++;
            //} else {
              //if (i == m[ii-1].id1 && sep < m[ii-1].d12) {
//cout<<"Duplicated input object is closer\n";
//cout<<"***index, id1,  id2_old, id2_new, sep_old, sep_new: "<<
//m[ii-1].index<<" "<<m[ii-1].id1<<"  "<< m[ii-1].id2<<" "<< k <<" "<< m[ii-1].d12*3600<<" "<<sep*3600<<endl;
                ////m[ii-1].id2 = k;
                //m[ii-1].d12 = sep;
              //}
            //}
          //}
cout<<"***jmax, j, j1,  i, k, j,  sep, minsep: "<<jmax<<" "<<j<<" "<<j1<<"  "<<i<<" "<<k<<"  "<<sep*3600<<" "<<minsep*3600<<endl;
        }

//printf("*nmatch j k myx1,myy1,myz1,xx2[k],yy2[k],zz2[k]\n %d %d %d %f %f %f %f %f %f \n",
// *nmatch, j, k, myx1,myy1,myz1,xx2[k],yy2[k],zz2[k]);
//printf("sep = %f\n", sep);

      } /* end for j */

// Manage multi-matches: remove single matches within max distance
      if (j1 == 1) {
cout<<"Removing "<<m[ii-1].index<<"  "<<m[ii-1].id1<<" "<< m[ii-1].id2<<" "<< m[ii-1].d12*3600<<endl;
        m.pop_back(); 
        ii--;
      }
//if (j1>0) {
//cout<<"***j1: "<<j1<<endl;
      j1 = 0;
//}

      if (minsep < matchlength) {
        if (maxmatch > (*nmatch)) {
if (jmax > 1)
cout<<"ndup_in, jmax, i1, k1, minsep: "<<ndup_in<<" "<<jmax<<"  "<<i1<<" "<<k1<<" "<<minsep*3600<<endl;

          match1.push_back(i1);
          match2.push_back(k1);
          distance12.push_back(minsep);
        }
        (*nmatch)++;
      }

    } /* end if jmax>0 */

  } /* end for i */

/* 4. clean up after chunks */
  unassignchunks(&nchunk2,&chunklist2,nra,ndec);
  unsetchunks(&rabounds,&decbounds,&nra,&ndec);

/* 6. free memory */
  free_memory();


cout<<"Nmatch="<<*nmatch<<endl;

//
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

  //if (match2.size() > 1) {
  if (*nmatch > 1) {

    unsigned long ij=0;
    long ndup=0, dup_id=-1, n = *nmatch;
    double dmin=361;
    std::vector<m2data> m2(n);  // temporary array

    for (i=0; i<n; i++) {
      m2[i].index = i;
      m2[i].id = match2[i];
cout <<"i: "<<i<<"  match1[i]: "<<match1[i]<<" m2[i].id: "<<m2[i].id<<" d: "<<distance12[i]*3600 <<endl;
    }

    sort(m2.begin(), m2.end(), m2_by_id());


for (i=0; i<m.size(); i++)
cout <<"i: "<<i<<" id1: "<<m[i].id1<<" id2: "<<m[i].id2<<" d21: "<<m[i].d12*3600<<endl;

    //sort(m.begin(), m.end(), m_by_id2());
    sort(m.begin(), m.end());

cout<<"\nDup. array after ID2 sorting:\n";
for (i=0; i<m.size(); i++)
cout <<"i: "<<i<<" id1: "<<m[i].id1<<" id2: "<<m[i].id2<<" d21: "<<m[i].d12*3600<<endl;


cout<<"\nTemp array after ID sorting:\n";
for (i=0; i<n; i++)
cout <<"i: "<<i<<" index: "<<m2[i].index<<" match1["<<m2[i].index<<"]: "<<match1[m2[i].index]<<" m2[i].id: "<<m2[i].id<<" d: "<<distance12[m2[i].index]*3600<<endl;
cout<<endl;


    i=1;
    while (i<n) {
// u64560.lis t64560.lis
//if (m2[i].id>=127) cout <<"i="<<i<<"  "<<m2[i-1].id<<" "<<m2[i].id<<endl;
      if (m2[i].id != m2[i-1].id) {
        for (ij = 0; ij<m.size(); ij++) {
//if (m2[i].id>=127) cout <<"  i="<<i<<"  "<<m[ij].id2<<" "<<m2[i-1].id<<endl;
// Marking is cheaper than erasing
          if (m[ij].id2 == m2[i-1].id)
{
cout <<"ij="<<ij<<"  i="<<i<<"  "<<m2[i-1].id<<" "<<m2[i].id<<endl;
cout <<"  ndup="<<ndup<<" ij="<<ij<<"  "<<m[ij].id2<<endl;
cout<<m[0].id2<<endl;
 m[ij].id2 = -1;
          ndup++;
}
        }
      for (ij=0; ij<m.size(); ij++) {
        if (m[ij].id2 == -1) {
          m.erase(m.begin()+ij);
          ij++;
        }
      }

      } else {
// Skip multiple ids
        dup_id = m2[i].id;
        i++;
        while (ii<n && m2[i].id == dup_id)
          i++;
      }
      i++;
    }
/*
  i1 = 0;
      for (ij=0; ij<m.size(); ij++) {
        if (m[ij].id2 == -1) {
          m.erase(m.begin()+ij -i1);
          ij++;
          i1++;
        }
      }
*/

    //if (m.size() < nmul) {

cout<<"Duplicated list cleaned. From "<<nmul<<" to "<<m.size()<<endl;
      nmul = m.size();

      for (i=0; i<nmul; i++)
        cout <<m[i].index<<" "<<m[i].id1<<" "<<m[i].id2<<" "<<m[i].d12*3600<<endl;
    //} else
//cout<<"Duplicated list unchanged.\n";

  long m_marked=0, got_i=-1, got_id=-1;
  double got_d=0;

    ij = 0;
    ndup = 0;
    dup_id = -1;
    for (i=1; i<n; i++) {

// This seems not correct for cases like this:
// id1 id2 d
// 20  32  0.5
// 21  32  0.3
// 22  33  0.3
// 23  33  0.5

      //if (m2[i-1].id == -1) continue;
cout<<"ij, i, m2[ij].id, m2[i].id, dup_id: "<<ij<<" "<<i<<" "<<m2[ij].id<<" "<<m2[i].id<<" "<<dup_id<<endl;

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
    <<" d1="<< distance12[m2[ij].index]*3600 <<" d2="<< distance12[m2[i].index]*3600<<endl;

        if (distance12[m2[i].index] < dmin)  // following object is closer
        {
cout<<"Following duplicated object is closer.\n";
          dmin = distance12[m2[i].index];
          //m2[ij].id = -1;
          //ij = i;
          for (j=0; j<nmul; j++) {

cout<<"j: "<<j<<" id1[j]="<<m[j].id1<<"  match1["<<m2[ij].index<<"]="<<match1[m2[ij].index]
           <<" id2["<<j<<"]="<<m[j].id2<<" d="<<m[j].d12*3600<<endl;

            //if (m[j].id1 == match1[m2[ij].index] && m[j].id2 != m2[i].id && m[j].id2 != -1) {
            if ((m[j].id1 == match1[m2[ij].index]) && (m[j].id2 != -1) && (m[j].id2 != m2[i].id)) {

if (m[j].id2 == got_id) {
  if (got_d < distance12[m2[i].index]) continue;
  else {
    m2[got_i].id = -1;
    m_marked++;
  }
}
cout<<"Swapping ids: Old refcat id="<<match2[m2[ij].index]<<" d="<<distance12[m2[ij].index]*3600;
              match2[m2[ij].index] = m[j].id2;
              distance12[m2[ij].index] = m[j].d12;
got_i = ij;
got_id = m[j].id2;
got_d  = m[j].d12;
cout<<" New id="<<m[j].id2<<" d="<<m[j].d12*3600<<endl;
              m[j-1].id2 = -1;
              break;
            }
          }
          if (j == nmul) {
cout<<" Refcat ID unchanged. Ignore m2["<<ij<<"].id: "<<m2[ij].id<<" d="<<distance12[m2[ij].index]*3600<<endl;
            m2[ij].id = -1;
            m_marked++;
          }
          ij = i;

        } else {
cout<<"Previous duplicated object is closer.\n";

          for (j=0; j<nmul; j++) {

cout<<"j: "<<j<<" id1[j]="<<m[j].id1<<"  match1["<<m2[i].index<<"]="<<match1[m2[i].index]
           <<" id2["<<j<<"]="<<m[j].id2<<" d="<<m[j].d12*3600<<endl;
 
            //if (m[j].id1 == match1[m2[i].index] && m[j].id2 != m2[i].id && m[j].id2 != -1) {
            if ((m[j].id1 == match1[m2[i].index]) && (m[j].id2 != -1) && (m[j].id2 != m2[i].id)) {
if (m[j].id2 == got_id) {
  if (got_d < distance12[m2[i].index]) continue;
  else {
    m2[got_i].id = -1;
    m_marked++;
  }
}
// 71 && 70 
cout<<"Swapping ids: Old refcat id="<<match2[m2[i].index]<<" d="<<distance12[m2[i].index]*3600;
              match2[m2[i].index] = m[j].id2;
// 71 -> 70
              distance12[m2[i].index] = m[j].d12;
got_i = ij;
got_id = m[j].id2;
got_d  = m[j].d12;
cout<<" New id="<<m[j].id2<<" d="<<m[j].d12*3600<<endl;
              break;
            }
          }
          if (j == nmul) {
cout<<" Refcat ID unchanged. Ignore m2["<<i<<"].id: "<<m2[i].id<<" d="<<distance12[m2[i].index]*3600<<endl;
            m2[i].id = -1;
            m_marked++;
          }
        }

cout<<"dmin="<<dmin*3600<<" dcur="<< distance12[m2[i].index]*3600<<endl;
      } else {  // end if dup
        ij = i;
        dup_id = -1;
        dmin = 361;
        got_id = -1;
      }
    }  // end for i<n


/*
    for (i=1; i<n; i++) {
cout<<"i: "<< i <<"  m1[i-1]="<<match1[m2[i-1].index] <<" m1[i]="<<match1[m2[i].index]
                <<"  m2[i-1].id: "<<m2[i-1].id<<"  m2[i].id: "<<m2[i].id<<endl;
    //<<"match1[m2[i-1].index]: "<<match1[m2[i-1].index] <<" match1[m2[i].index]: "<<match1[m2[i].index]<<endl
    //<<"distance12[m2[i-1].index]: "<<distance12[m2[i-1].index] <<" distance12[m2[i].index]: "<<distance12[m2[i].index]<<endl;
// Duplicated reference objects
      //if (m2[i-1].id == m2[i].id || m2[i-1].id == -1) {
      if (m2[i-1].id == -1)
{
cout<<"\n*i: "<<i<<" ndup="<<ndup<<" dup_id="<<dup_id<<"  m2[ij].id: "<<m2[ij].id<<"  m2[i].id: "<<m2[i].id
              <<" match1[ij]: "<<match1[ij]<<" match1[i]: "<<match1[i]<<endl;
 continue;  // following object already marked
}
      if (m2[ij].id == m2[i].id || m2[i].id == dup_id) {
      //if (m2[i-1].id == m2[i].id || m2[i].id == dup_id) {
        dup_id = m2[i].id;
        ndup++;
cout<<"\n***i: "<<i<<" ndup="<<ndup<<" dup_id="<<dup_id<<"  m2[ij].id: "<<m2[ij].id<<"  m2[i].id: "<<m2[i].id
              <<" match1[m2[ij].index]: "<<match1[m2[ij].index]<<" match1[m2[i].index]: "<<match1[m2[i].index]<<endl;
cout<<"Spherematch2: duplicated reference entry: m2[i].id="<< m2[i].id <<"  "<<ra2[m2[i].id]<<" "<<dec2[m2[i].id]
    <<" d1="<< distance12[m2[ij].index]*3600 <<" d2="<< distance12[m2[i].index]*3600 <<endl;

        if (distance12[m2[i].index] <= distance12[m2[ij].index] && distance12[m2[i].index] < dmin)  // following object is closer
        {
          dmin = distance12[m2[i].index];
          ij = i;
          i1 = ij;  //i-1;
          for (j=0; j<nmul; j++) {

cout<<"j: "<<j<<" id1[j]="<<id1[j]<<" m2[i1].index="<<m2[i1].index<<" match1[m2[i1].index]="<<match1[m2[i1].index]
           <<" id2[j]="<<id2[j]<<" m2[i].id="<<m2[i].id<<endl;

            if (id1[j] == match1[m2[i1].index] && id2[j] != m2[i].id) {

cout<<"Old match2[m2[i1].index]="<<match2[m2[i1].index]<<" distance12[m2[i1].index]="<<distance12[m2[i1].index]*3600<<endl;
              match2[m2[i1].index] = id2[j];
              distance12[m2[i1].index] = d12[j];
cout<<"New match2[m2[i1].index]="<<id2[j]<<" distance12[m2[i1].index]="<<d12[j]*3600<<endl;
              break;
            }
          }
          if (j == nmul) {
            m2[i1].id = -1;
            //if (m2[i].id == dup_id) m2[ij].id = -1;
          }
          //m2[i1].id = -1;

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

/*
        } else {
          if (distance12[m2[i-1].index] < dmin) {
            dmin = distance12[m2[i-1].index];
            ij = i-1;
          }
*/


/* Example:
   id1                             id2
   70   0.8224558   -1.0018456     70   0.8224558   -1.0018428    0.0100000
   70   0.8224558   -1.0018456     71   0.8224556   -1.0018431    0.0090554
   71   0.8224556   -1.0018458     70   0.8224558   -1.0018428    0.0110453
   71   0.8224556   -1.0018458     71   0.8224556   -1.0018431    0.0100000

70 70 0.0100000
71 70 0.0110453
70 71 0.0090554
71 71 0.0100000

which based on min distance will select
  match1  match2
  70      71     0.0090554
  71      71     0.0100000
so 71 is a duplicated ref. catalogue ID
*/

/*
          i1 = i;
          for (j=0; j<nmul; j++) {
cout<<"j: "<<j<<" id1[j]="<<id1[j]<<" m2[i1].index="<<m2[i1].index<<" match1[m2[i1].index]="<<match1[m2[i1].index]
           <<" id2[j]="<<id2[j]<<" m2[i].id="<<m2[i].id<<endl;

            if (id1[j] == match1[m2[i1].index] && id2[j] != m2[i].id) {
// 71 && 70 
cout<<"Old match2[m2[i1].index]="<<match2[m2[i1].index]<<" distance12[m2[i1].index]="<<distance12[m2[i1].index]*3600<<endl;
              match2[m2[i1].index] = id2[j];
// 71 -> 70
              distance12[m2[i1].index] = d12[j];
cout<<"New match2[m2[i1].index]="<<id2[j]<<" distance12[m2[i1].index]="<<d12[j]*3600<<endl;
              break;
            }
          }
          if (j == nmul) {
            m2[i].id = -1;
            //if (m2[i].id == dup_id) m2[ij].id = -1;
          }
        }  // end else

        //dup_id = m2[i].id;

cout<<"dmin="<<dmin*3600<<" distance12[m2[i1].index]="<< distance12[m2[i1].index]*3600<<"  m2[ij].id: "<<m2[ij].id<<"  m2[i].id: "<<m2[i].id<<endl;
      } else {  // end if
        dmin = 361;
      }

    }  // end for i<n
*/


    if (m_marked > 0) {
      sort(m2.begin(), m2.end(), m2_by_index());
      i1=0;
cout<<"Cleaning match list ("<<m_marked<<" marked)..."<<endl;
      for (i=0; i<n; i++) {
        if (m2[i].id == -1) {
// Marking is cheaper than erasing
//match1[m2[i].index] = -1;
//match2[m2[i].index] = -1;
//distance12[m2[i].index] = -1;
cout<<i1<<": erase i: "<<i<<" m2[i].index: "<<m2[i].index<<" match1: "<<match1[m2[i].index -i1]<<" match2: "<<match2[m2[i].index -i1] <<endl;
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



int main (int argc, char *argv[])
{

  double minchunksize, min_dist=-1., matchlength=1./3600,
         *ra1=NULL, *de1=NULL, *ra2=NULL, *de2=NULL;
  unsigned long i, nrows, nr1, nr2, *id1=NULL;
  long nmatch;
  string row, list1, list2;

  vector<double> distance12;
  vector<long> match1, match2;

  std::ifstream ifs;

  cout << setiosflags(ios::fixed);
  cout << setprecision(7);


  if (argc<3)
    exit(0);

  list1 = string(*++argv);
  list2 = string(*++argv);

  if (argc > 3)
    sscanf(*++argv,"%lf",&min_dist);

  if (min_dist >= 0.) {
    matchlength = min_dist/3600;
  } else
    min_dist = 1.;

  ifs.open(list1.c_str(), ios_base::in);
  //std::ifstream ifs(list1.c_str());

//stringstream strStream;
//strStream << inFile.rdbuf();//read the file
//string str = strStream.str();//str holds the content of the file

// new lines will be skipped unless we stop it from happening:    
    ifs.unsetf(std::ios_base::skipws);
// count the newlines with an algorithm specialized for counting:
    nrows = std::count(
        std::istream_iterator<char>(ifs),
        std::istream_iterator<char>(), 
        '\n');

    cout << list1 <<": lines: " << nrows <<"\n";
//length = ifs.tellg();           // report location (this is the length)

  ifs.clear();
  ifs.seekg(0, std::ios::beg);    // go back to the beginning

  nr1 = nrows;

  if (!(id1 = (unsigned long *) malloc(nr1 * sizeof(unsigned long)))) exit(-1);
  if (!(ra1 = (double *) malloc(nr1 * sizeof(double)))) exit (-1);
  if (!(de1 = (double *) malloc(nr1 * sizeof(double)))) exit(-1);
//  if (t.use_master_id1) {
//    if (!(mt1 = (unsigned long *) realloc(mt1, nr1 * sizeof(unsigned long)))) exit(-1);
//    if (!(rn1 = (unsigned long *) realloc(rn1, nr1 * sizeof(unsigned long)))) exit(-1);
//  }

  i = 0;
  while(std::getline(ifs, row)){
   //std::getline(ifs, row);
   //ifs >> id1[i] >> ra1[i] >> de1[i];
//cout << row<<endl;
    sscanf(row.c_str(),"%lu %lf %lf",&id1[i],&ra1[i],&de1[i]);
//cout <<i<<"  "<< id1[i] <<" "<< ra1[i] <<" "<< de1[i] <<endl;
   i++;
 }

  ifs.close();
  //cout <<list1<<" done\n";

  ifs.open(list2.c_str());

    ifs.unsetf(std::ios_base::skipws);
    nrows = std::count(
        std::istream_iterator<char>(ifs),
        std::istream_iterator<char>(), 
        '\n');

    cout << list2 <<": lines: " << nrows <<"\n";
 
  ifs.clear();
  ifs.seekg(0, std::ios::beg);    // go back to the beginning

  nr2 = nrows;

  if (!(ra2 = (double *) malloc(nr2 * sizeof(double)))) exit (-1);
  if (!(de2 = (double *) malloc(nr2 * sizeof(double)))) exit(-1);

  i = 0;
  while(std::getline(ifs, row)){
    sscanf(row.c_str(),"%lf %lf",&ra2[i],&de2[i]);
//cout <<i<<"  "<< ra2[i] <<" "<< de2[i] <<endl;
   i++;
 }

  ifs.close();
  //cout <<list2<<" done\n";


// This could be parametrized
  minchunksize = matchlength * 10;
  nmatch = nr1*nr2;

  spherematch2(nr1, ra1, de1, nr2, ra2, de2, matchlength, minchunksize,
               match1, match2, distance12, &nmatch);

  if (nmatch == 0) {
    cout << "no match at a separation of "<< min_dist <<" arcsec"<< endl;
    exit(0);
  }


  cout <<"\nFound "<< nmatch <<" matches at max sep. of "<< setw(3)<< min_dist << " arcsec\n";

  bool do_list_match=true;

  if (do_list_match) {
    int idw1=0, idw2=0;
    const int iwidth=6, dwidth=12;
    const string bl=" ";
    cout << endl<<"Matched list:"<< endl

         << setw(iwidth)<<"SeqID1"<< bl <<setw(idw1)<<"htmID ";

      cout  <<setw(dwidth)<<"RAdeg1"<< bl <<setw(dwidth)<<"DECdeg1"<< bl
         << setw(iwidth)<<"SeqID2"<< bl;

      cout << setw(dwidth)<<"RAdeg2"<< bl <<setw(dwidth)<<"DECdeg2"<< bl
         <<"  Sep (arcsec)"<< endl;

    for (i=0; i<nmatch; i++) {
      cout << setw(iwidth) << match1[i]<< bl <<setw(idw1)<<id1[match1[i]] << bl;

      cout <<setw(dwidth)<<ra1[match1[i]]<< bl <<setw(dwidth)<<de1[match1[i]]<< bl
           << setw(iwidth) << match2[i]<< bl;

      cout << setw(dwidth)<<ra2[match2[i]]<< bl <<setw(dwidth)<<de2[match2[i]]<< bl
           <<setw(dwidth)<< distance12[i]*3600 << endl;
    }

  }

  return 0;
}
