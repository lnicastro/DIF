/*
  Test DIF_Region DIF class.

Last changed: 19/03/2009
*/

#include <iostream>
using namespace std;

#include "dif.hh"

int main() {
    DIF_Region ss;
    int i, j;

    double ra=20.,  // RA  (deg)
           de=-30.,  // Dec (deg)
           rs=10.;  // radius or side (arcmin)
    int param;


    //Test the read_reset(), read_next() interface
    ss.read_reset();
    int read_param, read_full;
    long long int read_val;
    for (i=0; i<ss.params.size(); i++) {
        cout << "Param. # "<< i+1 << endl;
//        while (!ss.read_next(read_param, read_val, read_full))
//	    cout << read_param << "\t" << read_val << "\t" << read_full << endl;

    }
    ss.clear_region(); //THIS MUST BE DONE BEFORE EACH NEW SEARCH


// Rectangular region
    param = 6;

    ss.setSchema(DIF_HTM);
    ss.regtype = DIF_REG_2VERT;
ss.ra1 = 271.69677;
ss.de1 = 26.954834;
ss.ra2 = 270.00000;
ss.de2 = 25.54803;

//ss.ra1 = 270.000;
//ss.de1 = 25.548040;
//ss.ra2 = 271.69677;
//
//ss.de2 = 26.95483;


    ss.setAvailParam(6);
    ss.go();

    cout <<endl<<"2 corners Rectangular region Trixels list:"<< endl;

    for (i=0; i<ss.params.size(); i++) {
        param = ss.params[i];
        cout << "Depth/order: " << param << endl;

        vector<long long int>& full = ss.flist(param);
        cout << "List of FULLY covered pixels (" << full.size() << "):" << endl;
        for (j=0; j<full.size(); j++)
            cout << full[j] << endl;
        cout << endl;
    }

//    part = ss.plist(param);
    vector<long long int>& p6 = ss.plist(param);
    cout << "List of PARTIALLY covered pixels (" << p6.size() << "):" << endl;
    for (j=0; j<p6.size(); j++)
        cout << p6[j] << endl;

}
