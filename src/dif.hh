// ----------------------------------------------------------------------^
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010
// Giorgio Calderone <gcalderone@ifc.inaf.it>
// 
// This file is part of DIF.
// 
// DIF is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// DIF is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with DIF; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
// 
// ----------------------------------------------------------------------$


#ifndef DEF_SQL_HH
#define DEF_SQL_HH

#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef DATADIR
#undef PACKAGE
#undef VERSION

#include <math.h>
#include <pthread.h>
#include <vector>
using namespace std;


/*
HTM:
----
d (depth) : [0 , 25]
Npix = 8*4^d
Range id : [Npix , 2*Npix-1]

d        Type (unsigned)     Npix
0        BYTE                   8
1                              32
2                             128
3        SMALL                512
4                            2048
5                            8192
6                           32768
7        MEDIUM            131072
11       INTEGER
15..25   SIGNED BIGINT



Healpix:
--------
k (resolution parameter): [0 , 29]
Nside = 2^k
Npix = 12 * Nside^2
Range id : [0, Npix-1]


k        Type (unsigned)
0        BYTE
3        SMALL
7        MEDIUM
11       INTEGER
15..29   SIGNED BIGINT

References:
- http://www.journals.uchicago.edu/ApJ/journal/issues/ApJ/v622n2/61342/61342.web.pdf

- http://www.sdss.jhu.edu/htm/doc/womullan_082000.pdf

*/




/*
  class: ThreadSpecificData

  This is (almost) the same class as in mcs.hh
 */
template<class BASE>
class ThreadSpecificData 
{
private:
  pthread_key_t key;
  int ltag;

  static void base_destructor(void* p)
  { delete ((BASE*) p); }
  
public:
    ThreadSpecificData() 
	{ pthread_key_create(&key, base_destructor); }

    ~ThreadSpecificData()
	{ pthread_key_delete(key); }

    int tag() { return ltag; }

    void destructor() {
	BASE* p = getp();
	if (p) delete p;
	pthread_setspecific(key, NULL);
    }

    void constructor(int tag = 0) {
	destructor();
	BASE* p = new BASE();
	pthread_setspecific(key, p);
	ltag = tag;
    }

    inline BASE* getp() const
	{ return ((BASE*) pthread_getspecific(key)); }

    inline BASE* operator->() const
	{ return ((BASE*) pthread_getspecific(key)); }
};




/*
  enum: DIF_RegionType

  Specify region type
 */
enum DIF_RegionType {
  DIF_REG_NONE      ,  //none specified
  DIF_REG_CIRCLE    ,  //circle (center, radius)
  DIF_REG_RECT      ,  //rectangle (center, side, [side2])
  DIF_REG_2VERT     ,  //rectangle (two opposite corners)
  DIF_REG_4VERT     ,  //four-sided region (four corners)
  DIF_REG_NEIGHBC   ,  //IDs list of neighbors to a (coords) given one
  DIF_REG_SNEIGHB   ,  //IDs list of neighbors at higher depth/order given one
};




/*
  enum: DIF_Schema

  Specify pixelization schema
 */
enum DIF_Schema {
  DIF_NONE          ,
  DIF_HTM           , 
  DIF_HEALP_RING    ,
  DIF_HEALP_NEST    
};





//class atomRegion {
//public:
//  enum DIF_RegionType regtype;    //region type
//  bool flagNOT;
//
//  atomRegion* child1;
//  atomRegion* child2;
//  atomRegion* parent;
//
//  atomRegion() {
//    flagNOT = false;
//    regtype = DIF_REG_NONE;
//    child1 = NULL;
//    child2 = NULL;
//    parent = NULL;
//  }
//
//  ~atomRegion() {
//    if (child1) delete child1;
//    if (child2) delete child2;
//  }
//};
//
//
//class atomCircle : public atomRegion {
//public:
//  double ra, de, rad;
//  
//  atomCircle() : atomRegion() {
//    regtype = DIF_REG_CIRCLE;
//  }
//};



double skysep_h(double phi1, double theta1, double phi2, double theta2, short radians);


/*

long spherematch2(long npoints1, double *ra1, double *dec1,
		  long npoints2, double *ra2, double *dec2,
		  double matchlength, double minchunksize,
		  vector<long> &match1, vector<long>&match2, vector<double>&distance12, long *nmatch);




class XMatch {
private:
    vector<double> ra1;
    vector<double> de1;
    vector<double> ra2;
    vector<double> de2;

    vector<double> distance12;
    vector<long> match1, match2;
    long nmatch;
    long icount;

public:

    XMatch() {}


    ~XMatch() {
	clean(1);
	clean(2);
    }


    void clean(int what=0) {
	if (what == 1) {
	    ra1.clear();
	    de1.clear();
	}
	if (what == 2) {
	    ra2.clear();
	    de2.clear();
	}

	distance12.clear();
	match1.clear();
	match2.clear();
	nmatch = 0;
	icount = 0;
    }



    long addData(int id, double ra, double de) {
	if (id == 1) {
	    ra1.push_back(ra);
	    de1.push_back(de);
	    return ra1.size()-1;
	}
	else {
	    ra2.push_back(ra);
	    de2.push_back(de);
	    return ra2.size()-1;
	}
    }



    void match(double dist) {
      double matchlength=dist/3600, minchunksize=matchlength * 4;
      if (ra1.size() < ra2.size())
	spherematch2(ra2.size(), &ra2[0], &de2[0], ra1.size(), &ra1[0], &de1[0],
		     matchlength, minchunksize,
		     match2, match1, distance12, &nmatch);
      else
	spherematch2(ra1.size(), &ra1[0], &de1[0], ra2.size(), &ra2[0], &de2[0],
		     matchlength, minchunksize,
		     match1, match2, distance12, &nmatch);

//	int i1;
//	int i2;
//
//	for (i1=0; i1<ra1.size(); i1++)
//	    for (i2=0; i2<ra2.size(); i2++)
//		if (skysep_h(ra1[i1], de1[i1], ra2[i2], de2[i2], 0) < dist) {
//		    match1.push_back(i1);
//		    match2.push_back(i2);
//		}
//
//	nmatch = match1.size();
    }



    int checkMatch(long id1, long id2) {
	//int n = match1.size();
	int i;

	for(i=icount; i<match1.size(); i++)
	    //if ((match1[i] == id1)   &&   (match2[i] == id2)) {
	    if ((match1[i] == id1)) {
                icount = i+1;
		return 1;
            }
	return 0;
    }
};

*/





/*
  class: DIF_Region
 */
class DIF_Region {
private:
  clock_t cpustart;  //CPU time measurement
  double cputime;

  //vector of pointers to vectors of FULL pixel IDs
  vector<vector<long long int>*> pflist; 

  //vector of pointers to vectors of PARTIAL pixel IDs
  vector<vector<long long int>*> pplist;
 
  //NULL vector
  static vector<long long int> nullvec;


  //Available order/depth parameters
  vector<int> avail_params;

  //Search for an available param
  int locateAvailParam(int param) {
    int i;
    for (i=0; i<avail_params.size(); i++) {
      if (avail_params[i] == param) 
	return i;
    }
    return -1;
  }

  //Search for a param in the param vector
  int locateParam(int param) {
    int i;
    for (i=0; i<params.size(); i++) {
      if (params[i] == param) 
	return i;
    }
    return -1;
  }


  //Read_next interface
  int rn_iavail_param;
  int rn_param;
  int rn_full;
  int rn_ipos;
  vector<long long int>* rn_list;
  //-------------------



  bool go_performed; //whether go() has already been executed

  enum DIF_Schema schema;         //pixelization schema

public:
  enum DIF_RegionType regtype;    //region type

  //Region parameters
  double ra1, ra2, de1, de2, rad;
  double ra3, ra4, de3, de4;

  //Reference pixel ID for which higher depth/order neighbors are requested
  long long int refpix;

  //Depth of the reference and neighbors pixels
  int indepth, outdepth;

  //CPU time measurement
  void subStart()
  { cpustart = clock(); }
  
  void subStop()
  { cputime += ((double) (clock()-cpustart)) / CLOCKS_PER_SEC; }
  
  double cpuTime() {
    double tmp = cputime;
    cputime = 0;
    return tmp;
  }


  //Clear pixel
  void clear_pixel() {
    int i;
    for (i=0; i<pflist.size(); i++)
      delete pflist[i];
    for (i=0; i<pplist.size(); i++)
      delete pplist[i];
    
    pflist.clear();
    pplist.clear();
    go_performed = false;
    cputime = 0.;

    avail_params.clear();
    params.clear();
    schema = DIF_NONE;
   }


  //Clear region and pixel
  void clear_region() {
    regtype = DIF_REG_NONE;
    ra1 = 0.;
    de1 = 0.;
    ra2 = 0.;
    de2 = 0.;
    ra3 = 0.;
    de3 = 0.;
    ra4 = 0.;
    de4 = 0.;
    rad = 0.;
    indepth = 0;
    refpix = 0;
    outdepth = 0;
    clear_pixel();
  }


  //Constructor
  DIF_Region()
  { clear_region(); }
  
  //Destructor
  ~DIF_Region() 
  { clear_region(); }
  

  //Add available param
  void setAvailParam(int p) {
    if (locateAvailParam(p) == -1) {
	avail_params.push_back(p);
    }
  }


  void setSchema(enum DIF_Schema ss) {
      if ((schema != DIF_NONE)   &&
	  (schema != ss)) {
	  clear_pixel();
      }
      schema = ss;
  }
    

  enum DIF_Schema getSchema() {
    return schema;
  }


  vector<long long int>& flist(int param) {
    int i = locateParam(param);
    if (i == -1) return nullvec;
    return *pflist[i];
  }
  
  vector<long long int>& plist(int param) {
    int i = locateParam(param);
    if (i == -1) return nullvec;
    return *pplist[i];
  }
  

  vector<int> params;            /*list of order/depth parameter to be used
				   for search (populated in ::go()) */


  //Read interface
  void read_update_list() {
    rn_list = &nullvec;
    
    if (avail_params.size() == 0)
      return;
    
    int i = locateParam(avail_params[rn_iavail_param]);
    if (i > -1) {
	if (pflist.size() > i)
	    rn_list = (rn_full ?   pflist[i]   :   pplist[i]);
	else 
	    rn_list = &nullvec;
    } 
    rn_param = params[i];
    rn_ipos = 0;
  }
  
  
  void read_reset() {
    rn_iavail_param = 0;
    rn_full = 1;
    read_update_list();
  }
  
  
  inline int read_next(int& param, long long int& val, int& full) {
    if (rn_ipos < rn_list->size()) {
      param = rn_param;
      val = (*rn_list)[rn_ipos++];
      full = rn_full;
      return 0;
    }
    else {
      if (rn_full) {
	rn_full = 0;
	read_update_list();
	return read_next(param, val, full);
      }
      else {
	rn_iavail_param++;
	
	if (rn_iavail_param < avail_params.size()) {
	  rn_full = 1;
	  read_update_list();
	  return read_next(param, val, full); //Change param
	}
	else
	  return 1; //Final EOF
      }
    }
  }
  
  
  void go();
};






//General external functions


//HTM-related functions
int DIFhtmCircleRegion(DIF_Region &p);
//int DIFhtmRectRegion(DIF_Region &p);
//int DIFhtmRectRegion2V(DIF_Region &p);
int DIFhtmRectRegion4V(DIF_Region &p);

//int DIFgetHTMNeighb(char*& saved, DIF_Region &p, long long int id);
//int DIFgetHTMNeighb1(DIF_Region &p, long long int id);
//int DIFgetHTMsNeighb(char*& saved, char*& osaved, DIF_Region &p, long long int id);
//int DIFgetHTMsNeighb1(DIF_Region &p, long long int id);
int DIFgetHTMNeighbC(char*& saved, DIF_Region &p, double ra, double dec);
int DIFgetHTMNeighbC1(DIF_Region &p);

int DIFgetHTMsNeighb(char*& saved, char*& osaved, DIF_Region &p, int depth, unsigned long long int id, int odepth);
int DIFgetHTMsNeighb1(DIF_Region &p);

int getHTMnameById(char*& saved, unsigned long long int id, char *idname);
int getHTMnameById1(unsigned long long int id, char *idname);
int getHTMidByName(char*& saved, const char *idname, unsigned long long int* id);
int getHTMid(char*& saved, int depth, double ra, double dec, unsigned long long int* id);

void cleanHTMUval(char*& saved);
void cleanHTMsUval(char*& saved, char*& osaved);

int getHTMNeighb(char*& saved, int depth, unsigned long long int id,
                 vector<unsigned long long int>& idn);

int getHTMNeighb1(int depth, unsigned long long int id,
                  vector<unsigned long long int>& idn);

int getHTMsNeighb(char*& saved, char*& osaved, int depth, unsigned long long int id, int odepth,
                 vector<unsigned long long int>& idn);

int getHTMsNeighb1(int depth, unsigned long long int id, int odepth,
                 vector<unsigned long long int>& idn);

int getHTMNeighbC(char*& saved, int depth, double ra, double dec,
                  vector<unsigned long long int>& idn);

int getHTMNeighbC1(int depth, double ra, double dec,
                   vector<unsigned long long int>& idn);

int getHTMBary(char*& saved, int depth, unsigned long long int id,
               double *bc_ra, double *bc_dec);

int getHTMBary1(int depth, unsigned long long int id,
                double *bc_ra, double *bc_dec);

int getHTMBaryC(char*& saved, int depth, double ra, double dec,
                double *bc_ra, double *bc_dec);

int getHTMBaryC1(int depth, double ra, double dec,
                 double *bc_ra, double *bc_dec);

double getHTMBaryDist(char*& saved, int depth, unsigned long long int id,
                      double ra, double dec);

double getHTMBaryDist1(int depth, unsigned long long int id,
                       double ra, double dec);



//HEALPix-related functions
int DIFmyHealPRect4V(DIF_Region &p);
int DIFmyHealPCone(DIF_Region &p);

int DIFgetHealPNeighbC(char*& saved, DIF_Region &p, double ra, double dec);
int DIFgetHealPNeighbC1(DIF_Region &p);


int getHealPid(char*& saved, int nested, int order, double ra, double dec, long long int *id);
void cleanHealPUval(char*& saved);

int getHealPNeighb(char*& saved, int nested, int order, long long int id,
                   vector<long long int>& idn);

int getHealPNeighb1(int nested, int order, long long int id,
                   vector<long long int>& idn);

int getHealPNeighbC(char*& saved, int nested, int order, double ra, double dec,
                    vector<long long int>& idn);

int getHealPNeighbC1(int nested, int order, double ra, double dec,
                     vector<long long int>& idn);


int getHealPBary(char*& saved, int nested, int order, long long int id,
                 double *bc_ra, double *bc_dec);

int getHealPBary1(int nested, int order, long long int id,
                  double *bc_ra, double *bc_dec);

int getHealPBaryC(char*& saved, int nested, int order, double ra, double dec,
                  double *bc_ra, double *bc_dec);

int getHealPBaryC1(int nested, int order, double ra, double dec,
                   double *bc_ra, double *bc_dec);

double getHealPBaryDist(char*& saved, int nested, int order, long long int id,
                        double ra, double dec);

double getHealPBaryDist1(int nested, int order, long long int id,
                         double ra, double dec);


double getHealPMaxS(char*& saved, int order);

double getHealPMaxS1(int order);

int getHealPBound(char*& saved, int nested, int k, long long int id, unsigned int step,
                 vector<double> &b_ra, vector<double> &b_dec);

int getHealPBound1(int nested, int k, long long int id, unsigned int step,
                  vector<double> &b_ra, vector<double> &b_dec);

int getHealPBoundC(char*& saved, int nested, int k, double ra, double dec, unsigned int step,
                 vector<double> &b_ra, vector<double> &b_dec);

int getHealPBoundC1(int nested, int k, double ra, double dec, unsigned int step,
                  vector<double> &b_ra, vector<double> &b_dec);
#endif // DEF_SQL_HH
