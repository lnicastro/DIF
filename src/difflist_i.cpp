/*
  Name: void difflist_i

  Description:
   Returns in vdiff the (v1 - v2).

  Input:
   vector<long long int>& v1
   vector<long long int>& v2

  Output:
   vector<long long int>& vdiff

  Note:
   Input vector lists must be sorted in "ascending" order.
   v2 should be a sub-set of v1.

  LN@IASF-INAF, Aug 2007                        Last change: 12/10/2007
*/

#include <vector>
using namespace std;

void difflist_i(vector<long long int>& v1,
                vector<long long int>& v2,
                vector<long long int>& vdiff)
{
  unsigned int i, j0=0, j=0, n1, n2;

  vdiff.clear();
  n1 = v1.size();
  n2 = v2.size();

  if (n1 == 0) return;

  for (i=0; i<n2; i++) {

    for (j=j0; j<n1; j++) {
      if (v1[j] < v2[i]) {  //
        vdiff.push_back(v1[j]);
      }
      else if (v1[j] == v2[i]) {
        j0 = j + 1;
        break;
      }
      else
      if (v1[j] > v2[i]) { // should never happen
        j0 = j;
        break;
      }
    }

  }

// Assign remaining elements
  if ( j < n1 )
    for (j=j0; j<n1; j++) vdiff.push_back(v1[j]);

}
