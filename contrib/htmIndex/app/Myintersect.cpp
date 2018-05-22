//#     Filename:       Myintersect.cpp (from "intersect.cpp")
//#
//#     Intersect a domain with the index, returning the full and partial nodes
//#
//#
//# LN change for gcc 3.3: 07/12/2003

using namespace std;

#include <fstream>
#include "VarStr.h"
#include <VarVecDef.h>
#include "SpatialInterface.h"
#include "SpatialDomain.h"

int getIntersect(float64 *ra, float64 *dec, ValVec<uint64> *flist, ValVec<uint64> *plist);

void
usage(char *name) {

  cout << "usage: " << endl
       << name << " RAcorner1 DECcorner1 RAcorner2 DECcorner2" << endl
       << endl;
  exit(0);
}

int
main(int argc, char *argv[]) {

  int arg = 4;			// number of required arguments
  int args = 1;			// start to parse argument no 1
  int i;
//  float64 ra1, dec1, ra2, dec2, ra3, dec3, ra4, dec4;
//  ValVec<float64> ra[4], dec[4];
  float64 ra[4], dec[4];

   ValVec<uint64> plist, flist;	// List results

  argc--;

  if (argc != arg) usage(argv[0]);
ra[0]  = atof(argv[args++]);
dec[0] = atof(argv[args++]);
ra[2]  = atof(argv[args++]);
dec[2] = atof(argv[args++]);
ra[1]  = ra[0];
dec[1] = dec[2];
ra[3]  = ra[2];
dec[3] = dec[0];

getIntersect(ra, dec, &flist, &plist);

	  printf("List 2 of partial nodes (%d): \n",plist.length());
	  for(i = 0; i < plist.length(); i++){  // just loop this list
	    PRINTID(plist(i)); 			// print ID
	    printf(":");
	    PRINTID_HEX(plist(i)); 			// print ID
	    printf("\n");
	  }
}

int getIntersect(float64 *ra, float64 *dec, ValVec<uint64> *flist2, ValVec<uint64> *plist2)
{
  int depth=6,savedepth=2;	// depth and stored depth
  size_t i,j,n=1;
  ValVec<uint64> plist, flist;	// List results

  try
    {
      // construct index with given depth and savedepth
      htmInterface htm(depth,savedepth);  // generate htm interface
      const SpatialIndex &index = htm.index();

      SpatialDomain domain;    // initialize empty domain


SpatialVector v1(ra[0], dec[0]), v2(ra[1], dec[1]), v3(ra[2], dec[2]), v4(ra[3], dec[3]);
SpatialConvex cvx(&v1,&v2,&v3,&v4);
domain.add(cvx);


/*******************************************************
/
/ Intersection
/
 ******************************************************/


      j = n;
	while(j--) {
	  domain.intersect(&index,plist,flist);	  // intersect with list
      }

/*******************************************************
/
/ Print result
/
 ******************************************************/

      SpatialVector vertex1,vertex2,vertex3;	  // Fill vertices in these

	// ----------- FULL NODES -------------
	  printf("List of full nodes : \n");
	  for(i = 0; i < flist.length(); i++){  // just loop this list
	    PRINTID(flist(i)); 			// print ID
	    printf(":");
	    PRINTID_HEX(flist(i)); 			// print ID
	    printf(":%s ", index.nameById(flist(i)));	// print Name
	    index.nodeVertex(flist(i),// get vertices
			     vertex1,vertex2,vertex3);
	    printf("%f,%f ; %f,%f ; %f,%f\n",
		   vertex1.ra(), vertex1.dec(), // print ra,
		   vertex2.ra(), vertex2.dec(), // dec of
		   vertex3.ra(), vertex3.dec());// vertices
	  }
	  printf("List of partial nodes : \n");
	  for(i = 0; i < plist.length(); i++){  // just loop this list
	    PRINTID(plist(i)); 			// print ID
	    printf(":");
	    PRINTID_HEX(plist(i)); 			// print ID
	    printf(":%s ", index.nameById(plist(i)));	// print Name
	    index.nodeVertex(plist(i),// get vertices
			     vertex1,vertex2,vertex3);
	    printf("%f,%f ; %f,%f ; %f,%f\n",
		   vertex1.ra(), vertex1.dec(), // print ra,
		   vertex2.ra(), vertex2.dec(), // dec of
		   vertex3.ra(), vertex3.dec());// vertices
	  }
    }
  catch (SpatialException &x)
    {
      printf("%s\n",x.what());
    }
  *flist2 = flist;
  *plist2 = plist;
  return 0;

}
