/*
  Name:  int getHTMidByName, int getHTMidByName1

  Purpose:
   Return the HTM ID (uint32) from a given trixel name.
   getHTMidByName1 always creates a new SpatialIndex class instance and destroys
   it on exit.

  Parameters:
 ( (i) char*& saved: if not NULL then re-use existing SpatialIndex )
   (i) const char* idname: Trixel ASCII name

   (o) unsigned long long int *id: HTM id

  Note:
    Check for depth in the range [0, 30].

  Return 0 on success.


  LN@IASF-INAF, Oct 2015                        Last change: 27/10/2015
*/

using namespace std;

#include "SpatialInterface.h"

void cleanHTMUval(char*& saved);

int getHTMidByName(char*& saved, const char *idname, unsigned long long int *id)
{
  SpatialIndex *index;

  int depth = strlen(idname)-2;

// Depth in allowed range
  if ((depth < 0) || (depth > 30))
    return -1;

  try {
    if (! saved) {
      index = new SpatialIndex(depth);
      saved = (char*) index;

    } else
      index = (SpatialIndex*) saved;

    *id = index->idByName(idname);

  } catch (SpatialException x) {
#ifdef DEBUG_PRINT
    cerr << "Error: " << x.what() << endl;
#endif
    *id = 0;
    return -1;
  }

  return 0;
}


int getHTMidByName1(int depth, const char *idname, unsigned long long int *id)
{
  char* saved = NULL;
  int ret = getHTMidByName(saved, idname, id);
  cleanHTMUval(saved);
  return ret;
}
