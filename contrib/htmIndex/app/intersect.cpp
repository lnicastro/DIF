//#     Filename:       intersect.cpp
//#
//#     Intersect a domain with the index, returning the full and partial nodes
//#
//#
//#     Author:         Peter Z. Kunszt
//#
//#     Date:           October 15, 1999
//#
//#
//#
//# Copyright (C) 2000  Peter Z. Kunszt, Alex S. Szalay, Aniruddha R. Thakar
//#                     The Johns Hopkins University
//#
//# This program is free software; you can redistribute it and/or
//# modify it under the terms of the GNU General Public License
//# as published by the Free Software Foundation; either version 2
//# of the License, or (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//#
//#
//# LN change for gcc 3.3: 07/12/2003

using namespace std;

#include <fstream>
#include <time.h>
#include "VarStr.h"
#include <VarVecDef.h>
#include "SpatialInterface.h"
#include "SpatialDomain.h"


/*******************************************************
 
  DESCRIPTION
 
  This example code demonstrates the intersection functionality of
  the SpatialIndex.
 
  It can be invoked by
 
  	intersect level savelevel domainfile [-b]
 

  where

     level     : the level depth to build the index (2 - 14)
     savelevel : the level depth to keep index in memory (recommended 2 - 6)
     domainfile: file containing a domain
     [-b]      : return result as bitlist instead of a list of id's.



  The level and savelevel just indicate the index depth.  The -b
  switch might speed up things for depths up to 7 with large-area
  domains.

  Domain files:
 
  A domain is a set of convexes, connected by logical OR. 
  A convex is a set of constraints, connected by logical AND.
  A constraint specifies a cap on the unit sphere by specifying
  a vector 'a' and a distance 'd' from the origin. 's' is the
  opening angle defining the cap.

  Example: positive distance  a = (1,0,0), d = 0.5, s = 60 degrees

                   ____
                ---    ---
               /        /|\
              /        / |=\
             |        /  |==|     this side is in the convex.
            |        /\s |===|
            |------------|---| -> direction a
            |        \   |===|
             |        \  |==|
              \        \ |=/
               \        \|/
                ---____---


                     <-d-> is positive (s < 90)


 Example: negative distance  a = (-1,0,0), d = -0.5, s = 120 degrees

                   ____
                ---====---
  this side is /========/|\
  in the      /========/=| \
  convex     |==== s__/==|  |
            |===== / /===|   |
  dir. a <- |------------|---|  'hole' in the sphere
            |========\===|   |
             |========\==|  |
              \========\=| /
               \========\|/
                ---____---



  
  A constraint is represented in the file as a set of 4 floatingpoint numbers
  x y z d  where a=(x,y,z). Example:

  1 0 0 0.5
 
  A convex is the area defined by several constraints on the sphere
  that is common to all constraints. Of course such an area has to be convex.

  A convex in the file is defined by a number (number of constraints
  in the convex) followed by as many convexes (each on one line.) Example:

  4
  0.5 0.707107 0.5 0.58077530122080200
  0.5 0.707107 0.5 0.84077530122080200
  -0.5 -0.907107 0.3  -0.87530122080200
  0.2 -0.907107 0.3  -0.77530122080200

  A domain is an accumulation of convexes. A domain in the domainfile
  is represented by a number (number of convexes) followed by that many
  convexes. A domainfile may only contain one domain. It may contain
  comment lines, starting with #.



  Special Identifiers
  -------------------

  The domain file accepts several special convex identifiers to
  read in special formats. These identifiers must come as a single
  comment line before the data, and no additional comments are allowed
  These are

    #TRIANGLE			- the next three lines are triangle corners
    #TRIANGLE_RADEC		- same, given in ra/dec
    #RECTANGLE			- the next four lines are rectangle corners
    #RECTANGLE_RADEC		- same, given in ra/dec
    #CONVEX			- read convex in default format
    #CONVEX_RADEC		- read the convex in ra/dec format

  Examples of a domain files:

-----------------

Example 1:

#DOMAIN
1
#CONVEX
3
 0.5 0.707107 0.5 0.58077530122080200
 0.5 0.5 0.707107 0.63480775301220802
 0.707107 -0.5 0.5 0.8480775301220802


-----------------

Example 2:

#DOMAIN
2
#CONVEX
4
 0 0 1 0.3
 -1 -3 -5 -0.97
 5 -3 -10 -0.97
 2 2 -5 -0.97
#CONVEX_RADEC
1
25.23 -55.9 0.99

-----------------

Example 3:

#DOMAIN
1
#RECTANGLE_RADEC
54 29
54 30
55 29
55 30

-----------------

Example 4:

#DOMAIN
1
#TRIANGLE
0 0 1
0 0.9 0.9
0.9 0 0.9

-----------------
-----------------

  Example of a run of intersect:

%intersect 4 2 example1

OUTPUT:

  index depth 2 built in 0.01 sec
#DOMAIN
1
#CONVEX
3
0.707107 -0.5 0.5 0.848078
0.5 0.5 0.707107 0.634808
0.5 0.707107 0.5 0.580775
intersection done in 0.01 sec
List of full nodes : 
3088:N00100  0,45 -8.04156,40.3737 0,39.375
3090:N00102  0,33.75 0,39.375 -7.43146,34.7373
3091:N00103  -7.43146,34.7373 0,39.375 -8.04156,40.3737
3097:N00121  0,33.75 -6.91598,28.9693 0,28.125
3101:N00131  0,33.75 -7.43146,34.7373 -6.91598,28.9693
3296:N03200  0,45 -9.34287,48.9681 -8.04156,40.3737
List of partial nodes : 
3076:N00010 0,22.5 -6.02366,17.0658 0,16.875
3079:N00013 -5.84825,11.415 0,16.875 -6.02366,17.0658
3089:N00101 -15,35.2644 -7.43146,34.7373 -8.04156,40.3737
3096:N00120 0,22.5 0,28.125 -6.56686,23.3217
3098:N00122 -13.2015,23.867 -6.56686,23.3217 -6.91598,28.9693
3099:N00123 -6.91598,28.9693 -6.56686,23.3217 0,28.125
3100:N00130 -13.2015,23.867 -6.91598,28.9693 -14.0498,29.5687
3102:N00132 -15,35.2644 -14.0498,29.5687 -7.43146,34.7373
3103:N00133 -7.43146,34.7373 -14.0498,29.5687 -6.91598,28.9693
3128:N00320 0,22.5 -6.56686,23.3217 -6.02366,17.0658
3168:N01200 0,45 0,50.625 -9.34287,48.9681
3169:N01201 0,56.25 -10.566,54.5745 0,50.625
3170:N01202 -20.1039,52.0619 -9.34287,48.9681 -10.566,54.5745
3171:N01203 -10.566,54.5745 -9.34287,48.9681 0,50.625
3180:N01230 -27.301,62.9135 -23.1655,57.5378 -12.2711,60.2863
3181:N01231 -20.1039,52.0619 -10.566,54.5745 -23.1655,57.5378
3182:N01232 0,56.25 -12.2711,60.2863 -10.566,54.5745
3183:N01233 -10.566,54.5745 -12.2711,60.2863 -23.1655,57.5378
3297:N03201 -20.1039,52.0619 -17.1921,43.691 -9.34287,48.9681
3298:N03202 -15,35.2644 -8.04156,40.3737 -17.1921,43.691
3299:N03203 -17.1921,43.691 -8.04156,40.3737 -9.34287,48.9681
3920:N31100 0,45 9.34287,48.9681 0,50.625
4000:N32200 0,45 0,39.375 8.04156,40.3737
4001:N32201 0,33.75 7.43146,34.7373 0,39.375
4003:N32203 7.43146,34.7373 8.04156,40.3737 0,39.375
4004:N32210 0,22.5 6.56686,23.3217 0,28.125
4006:N32212 0,33.75 0,28.125 6.91598,28.9693
4007:N32213 6.91598,28.9693 0,28.125 6.56686,23.3217
4014:N32232 0,33.75 6.91598,28.9693 7.43146,34.7373
4032:N33000 0,45 8.04156,40.3737 9.34287,48.9681

*******************************************************/

void
usage(char *name) {

  cout << "usage: " << endl
       << name << " [-save savelevel] [-verbose] [-n n] [-bit|-range] [-count] [-1] [-text] level domainfile" << endl;
  cout << " [-save savelevel] : store up to this depth (default 2)" << endl
       << " [-verbose]        : verbose" << endl
       << " [-n n]            : do n intersections" << endl
       << " [-bit|-range]     : do either bitlists or ranges" << endl
       << " [-count]          : only give a count of partial, full and total triangles" << endl
       << " [-1]              : print a 1-column list of all nodes touched" << endl
       << " [-text]           : use the text interface (for testing), always range output" << endl
       << " level             : index depth " << endl
       << " domainfile        : filename of domain " << endl
       << endl;
  exit(0);
}

int
main(int argc, char *argv[]) {

/*******************************************************
/
/ Initialization
/
 ******************************************************/

  bool verbose = false;		// verbosity flag
  bool count = false;		// countonly flag
  bool onecol = false;		// single column output flag
  bool text = false;		// text interface flag
  bool bitresult = false;       // bitlist intersection
  bool range = false;           // range intersection
  int arg = 2;			// number of required arguments
  char *infile;			// infile name
  int depth,savedepth=2;	// depth and stored depth
  size_t i,j,n=1;
  int args = 1;			// start to parse argument no 1
  argc--;

  while(argc > 0) {
    if(strcmp(argv[args],"-count")==0)
      count = true;
    else if(strcmp(argv[args],"-verbose")==0) 
      verbose = true;
    else if(strcmp(argv[args],"-1")==0) 
      onecol = true;
    else if(strcmp(argv[args],"-text")==0) {
      text = true;
      range = true;
      bitresult = false;
    } else if(strcmp(argv[args],"-bit")==0) {
      if(range)usage(argv[0]);
      bitresult = true;
    } else if(strcmp(argv[args],"-range")==0) {
      if(bitresult)usage(argv[0]);
      range = true;
    } else if(strcmp(argv[args],"-save")==0) {
      savedepth = atoi(argv[++args]); argc--;
      if(savedepth < 1 || savedepth > 8) {
	cerr << "savedepth should be between 1 and 8" << endl;
	return -1;
      }
    } else if(strcmp(argv[args],"-n")==0) {
      n = atoi(argv[++args]); argc--;
    } else {
      if( *argv[args] == '-' ) usage(argv[0]);
      switch(arg) {
      case 2:
	depth = atoi(argv[args]);
	if(depth < 1 || depth > HTMMAXDEPTH) {
	  cerr << "depth should be between 1 and " << HTMMAXDEPTH << endl;
	  return -1;
	}
	break;
      case 1:
	infile = argv[args];
	break;
      default:
	usage(argv[0]);
      }
      arg--;
    }
    argc--;
    args++;
  }
  if(arg > 0)usage(argv[0]);

  ifstream in(infile);
  if(!in) {
    cout << "cannot open file " << argv[3] << endl;
    return -1;
  }

  try
    {
      // construct index with given depth and savedepth
      htmInterface htm(depth,savedepth);  // generate htm interface
      const SpatialIndex &index = htm.index();

      // Read in domain and echo it to screen
      SpatialDomain domain;    // initialize empty domain
      in >> domain;		 // read domainfile
      if(verbose)
	cout << domain;


/*******************************************************
/
/ Intersection
/
 ******************************************************/

      BitList partial,full;		// BitList results
      ValVec<uint64> plist, flist;	// List results
      const ValVec<htmRange> *rlist;

      // do intersection and time it
      time_t t0 = clock();
      j = n;
      if(bitresult)
	while(j--)
	  domain.intersect(&index,partial,full);  // intersect with bitlist
      else if(range) {
	if(text) {
	  VarStr domstr;
	  char line[100];
	  sprintf(line,"DOMAIN %d\n%d\n",depth,domain.numConvexes());
	  domstr = line;
	  for(size_t k = 0; k < domain.numConvexes(); k++ ) {
	    sprintf(line,"%d\n",domain[k].numConstraints());
	    domstr += line;
	    for(size_t l = 0; l < domain[k].numConstraints(); l++ ) {
	      sprintf(line, "%20.16f %20.16f %20.16f %20.16f\n", 
		      domain[k][l].v().x(),
		      domain[k][l].v().y(),
		      domain[k][l].v().z(),
		      domain[k][l].d() );
	      domstr += line;
	    }
	  }
	  rlist = &htm.domainCmd(domstr.data());
	} else {
	  while(j--) {
	    rlist = &htm.domain(domain);	  // intersect with range
	  }
	}
      } else
	while(j--) {
	  domain.intersect(&index,plist,flist);	  // intersect with list
	}
      time_t t1 = clock();
      if(verbose)
	printf("%d intersections done at a rate of %f intersections/sec\n",
	       n, ((float)n)/(((double)(t1-t0)/(double)CLOCKS_PER_SEC)));

/*******************************************************
/
/ Print result
/
 ******************************************************/

      size_t f=0,p=0;
      SpatialVector vertex1,vertex2,vertex3;	  // Fill vertices in these

      // ----------------
      // BitList result
      // ----------------
      if(bitresult) {
	BitListIterator fIter(full), pIter(partial);  // iterators to bitlist

	// ----------- FULL NODES -------------
	if(!count) {
	  cout << "List of full nodes : " << endl;
	  while(fIter.next(true,i)) {
	    PRINTID_HEX(index.idByLeafNumber(i)); 	// print ID
//	    PRINTID(index.idByLeafNumber(i)); 	// print ID
		printf(" : %s  ",index.nameByLeafNumber(i));  // print Name
	    index.nodeVertex(i,vertex1,vertex2,vertex3);// get Node vertices
	    printf("%20.16f,%20.16f ",vertex1.ra(),vertex1.dec());  // print ra,
	    printf("%20.16f,%20.16f ",vertex2.ra(),vertex2.dec());  // dec of
	    printf("%20.16f,%20.16f\n",vertex3.ra(),vertex3.dec()); // vertices
	  }
	} else
	  f = full.count();
	
	// ----------- PARTIAL NODES ----------
	if(!count) {
	  cout << "List of partial nodes : " << endl;
	  while(pIter.next(true,i)) {
	    PRINTID_HEX(index.idByLeafNumber(i)); 	// print ID
//	    PRINTID(index.idByLeafNumber(i)); 	// print ID
		printf(" : %s  ",index.nameByLeafNumber(i));  // print Name
	    index.nodeVertex(i,vertex1,vertex2,vertex3);// get Node vertices
	    printf("%20.16f,%20.16f ",vertex1.ra(),vertex1.dec());  // print ra,
	    printf("%20.16f,%20.16f ",vertex2.ra(),vertex2.dec());  // dec of
	    printf("%20.16f,%20.16f\n",vertex3.ra(),vertex3.dec()); // vertices
	  }
	} else
	  p = partial.count();
 
      // ----------------
      // Range result
      // ----------------
      } else if(range){
	if(count) {
	  p = rlist->length();
	} else if(onecol) {
	  for(i = 0; i < rlist->length(); i++)
	    for(uint64 id = (*rlist)(i).lo; id <= (*rlist)(i).hi; id++)
	      printf("%s\n",index.nameById(id));
	} else {
	  printf("Ranges of nodes : \n");
	  for(i = 0; i < rlist->length(); i++){  // just loop this list
	    PRINTID_HEX( (*rlist)(i).lo );
	    printf(":%s ", index.nameById((*rlist)(i).lo));// print Name
	    PRINTID_HEX( (*rlist)(i).hi );  		   // print ID
	    printf(":%s\n", index.nameById((*rlist)(i).hi));// print Name
	  }
	}

      // ----------------
      // List result
      // ----------------
      } else {
	// ----------- FULL NODES -------------
	if(count) {
	  f = flist.length();
	} else if(onecol) {
	  for(i = 0; i < flist.length(); i++)
	    printf("%s\n", index.nameById(flist(i)));
	} else {
	  printf("List of full nodes : \n");
	  for(i = 0; i < flist.length(); i++){  // just loop this list
	    PRINTID_HEX(flist(i)); 			// print ID
//	    PRINTID(flist(i)); 			// print ID
	    printf(":%s ", index.nameById(flist(i)));	// print Name
	    index.nodeVertex(flist(i),// get vertices
			     vertex1,vertex2,vertex3);
	    printf("%f,%f ; %f,%f ; %f,%f\n",
		   vertex1.ra(), vertex1.dec(), // print ra,
		   vertex2.ra(), vertex2.dec(), // dec of
		   vertex3.ra(), vertex3.dec());// vertices
	  }
	}    
	// ----------- PARTIAL NODES ----------
	if(count) {
	  p = plist.length();
	} else if(onecol) {
	  for(i = 0; i < plist.length(); i++)
	    printf("%s\n", index.nameById(plist(i)));
	} else {
	  printf("List of partial nodes : \n");
	  for(i = 0; i < plist.length(); i++){  // just loop this list
	    PRINTID_HEX(plist(i)); 			// print ID
//	    PRINTID(plist(i)); 			// print ID
	    printf(":%s ", index.nameById(plist(i)));	// print Name
	    index.nodeVertex(plist(i),// get vertices
			     vertex1,vertex2,vertex3);
	    printf("%f,%f ; %f,%f ; %f,%f\n",
		   vertex1.ra(), vertex1.dec(), // print ra,
		   vertex2.ra(), vertex2.dec(), // dec of
		   vertex3.ra(), vertex3.dec());// vertices
	  }
	}
      }

      if(count) 
	printf("%d Full leaves ; %d Partial leaves : %d Total\n",f,p,f+p);

    }
  catch (SpatialException &x)
    {
      printf("%s\n",x.what());
    }
  return 0;

}
