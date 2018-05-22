/*
  Name:  int getHTMid, int getHTMid1

  Purpose:
   Return the HTM ID for a given depth mesh.
   getHTMid1 always creates a new SpatialIndex class instance and destroys
   it on exit.

  Parameters:
 ( (i) char*& saved: if not NULL then re-use existing SpatialIndex )
   (i) int depth:  Pixelization depth level in the range [0, 25]
   (i) double ra:  Right Ascension (degrees)
   (i) double dec: Declination (degrees)

   (o) unsigned long long int *id: HTM id

  Note:
    Check for depth in the range [0, 25].

  Return 0 on success.


  LN@IASF-INAF, Jan 2003                        Last change: 27/10/2015
*/

using namespace std;

#include "SpatialInterface.h"


int getHTMid(char*& saved, int depth, double ra, double dec,
             unsigned long long int *id)
{
  SpatialIndex *index;

// Depth in allowed range
  if ((depth < 0) || (depth > 25))
    return -1;

  try {
    if (! saved) {
      index = new SpatialIndex(depth);
      saved = (char*) index;

    } else
      index = (SpatialIndex*) saved;

    *id = index->idByPoint(ra,dec);

  } catch (SpatialException x) {
#ifdef DEBUG_PRINT
    cerr << "Error: " << x.what() << endl;
#endif
    *id = 0;
    return -1;
  }

  return 0;
}


void cleanHTMUval(char*& saved)
{
  if (saved)
    delete (SpatialIndex*) saved;
}


int getHTMid1(int depth, double ra, double dec, unsigned long long int *id)
{
  char* saved = NULL;
  int ret = getHTMid(saved, depth, ra, dec, id);
  cleanHTMUval(saved);
  return ret;
}
