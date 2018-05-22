/*
  Name:  int getHTMnameById, int getHTMnameById1

  Purpose:
   Return the HTM trixel name from ID (uint32).
   getHTMnameById1 always creates a new SpatialIndex class instance and destroys
   it on exit.

  Parameters:
 ( (i) char*& saved: if not NULL then re-use existing SpatialIndex )
   (i) unsigned long long int id: HTM id

   (o) char* idname: Trixel ASCII name

  Note:
    Check for ID in the range [8, 18446744073709551615].

  Return 0 on success.


  LN@IASF-INAF, Oct 2015                        Last change: 07/06/2016
*/

//using namespace std;

#include "SpatialInterface.h"

void cleanHTMUval(char*& saved);

int getHTMnameById(char*& saved, unsigned long long int id, char *idname)
{
  SpatialIndex *index;
  char *name;

// ID in allowed range
// Max Depth=30
// S0 -> 8 and N3333333333333333333333333333333 -> 18446744073709551615
  if ((id < 8) || (id > 18446744073709551615U))
    return -1;

  try {
    if (! saved) {
      index = new SpatialIndex(0);
      saved = (char*) index;

    } else
      index = (SpatialIndex*) saved;

    name = index->nameById(id);
    memcpy(idname, name, strlen(name));
    idname[strlen(name)] = '\0';
  } catch (SpatialException x) {
#ifdef DEBUG_PRINT
    cerr << "Error: " << x.what() << endl;
#endif
    return -1;
  }

  return 0;
}

int getHTMnameById1(unsigned long long int id, char *idname)
{
  char* saved = NULL;
  int ret = getHTMnameById(saved, id, idname);
  cleanHTMUval(saved);
  return ret;
}
