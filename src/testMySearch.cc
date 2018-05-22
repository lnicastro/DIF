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

// Circular region
    param = 0;  // (all depths)
    //param = 7;  // (use only depth=7)
    //param = -768;  // (use depths 8 and 9: -768 = -1 * (2^8 + 2^9))

    ss.setSchema(DIF_HTM);
    ss.regtype = DIF_REG_CIRCLE;
    ss.ra1 = ra;
    ss.de1 = de;
    ss.rad = rs;


//Suppose we have a table indexed with the following HTM depths
    ss.setAvailParam(8);
    ss.setAvailParam(7);
    ss.setAvailParam(6);
    ss.go(); //Perform the search and populate vectors
    
//Print results
    cout <<endl<<"RA, Dec: "<< ra <<" "<< de <<" (deg) rad.: "<< rs <<" (')"<< endl;

    cout <<endl<<"Circular region Trixels list:"<< endl;
    for (i=0; i<ss.params.size(); i++) {
	param = ss.params[i];
	cout << "Depth/order: " << param << endl;

	vector<long long int>& full = ss.flist(param);
	cout << "List of FULLY covered pixels (" << full.size() << "):" << endl;
//	for (j=0; j<full.size(); j++)
//	    cout << full[j] << endl; 
//      cout << endl;

	vector<long long int>& part = ss.plist(param);
	cout << "List of PARTIALLY covered pixels (" << part.size() << "):" << endl;
//      for (j=0; j<part.size(); j++)
//          cout << part[j] << endl; 
//      cout << endl;
    }

//    return 0;


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
ss.de2 = 25.54803; // 25.54804

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


// Rectangular region
    param = 8;  // (use only depth=8)
    rs *= 2;

    ss.setSchema(DIF_HTM);
    ss.regtype = DIF_REG_RECT;
    ss.ra1 = ra;
    ss.de1 = de;
    ss.ra2 = rs;

    ss.setAvailParam(8);
    ss.setAvailParam(7);
    ss.go();

    cout <<endl<<"Rectangular region Trixels list:"<< endl;

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
    vector<long long int>& par = ss.plist(param);
    cout << "List of PARTIALLY covered pixels (" << par.size() << "):" << endl;
    for (j=0; j<par.size(); j++)
        cout << par[j] << endl;


// HEALPix index on circular region
    ss.clear_region();
    param = 7;  // (use only depth=7)
    rs /= 2;
    //ss.newSearch(DIF_REG_HEALPCIRCLE, 0, ra, de, rs, flag);
    ss.setSchema(DIF_HEALP_RING);
    ss.regtype = DIF_REG_CIRCLE;
    ss.ra1 = ra;
    ss.de1 = de;
    ss.rad = rs;

    ss.setAvailParam(7);
    ss.go();

    cout <<endl<<"HEALPix Circular region pixels list:"<< endl;

    for (i=0; i<ss.params.size(); i++) {
        param = ss.params[i];
        cout << "Depth/order: " << param << endl;

//cout << "locateParam:" << ss.locateParam(param)<< endl;
//cout << "avail_params:" << ss.avail_params[0]<< endl;
        vector<long long int>& full = ss.flist(param);
        cout << "List of FULLY covered pixels (" << full.size() << "):" << endl;
        for (j=0; j<full.size(); j++)
            cout << full[j] << endl;
        cout << endl;
    }

//    part = ss.plist(param);
    vector<long long int>& par2 = ss.plist(param);
    cout << "List of PARTIALLY covered pixels (" << par2.size() << "):" << endl;
    for (j=0; j<par2.size(); j++)
        cout << par2[j] << endl;


// HTM pixel neighbours
    ss.clear_region();
    param = 10;  // (use only depth=10)

    ss.setSchema(DIF_HTM);
    ss.regtype = DIF_REG_NEIGHBC;
    ss.ra1 = ra;
    ss.de1 = de;
    ss.rad = 0;

    ss.go();

    cout <<endl<<"HTM pixel neighbors IDs:"<< endl;
    vector<long long int>& full = ss.flist(param);
    for (j=0; j<full.size(); j++)
            cout << full[j] << endl;
        cout << endl;

//HTM higher depth neighbors
    ss.clear_region();
    param = 6;

    ss.setSchema(DIF_HTM);
    ss.regtype = DIF_REG_SNEIGHB;
    ss.refpix = 64369;
    ss.outdepth = 8;
    ss.rad = 0;

    ss.go();

    cout <<endl<<"HTM pixel neighbors IDs:"<< endl;
    full = ss.flist(param);
    for (j=0; j<full.size(); j++)
            cout << full[j] << endl;
        cout << endl;


}
