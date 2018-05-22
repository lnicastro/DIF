//#     Filename:       sqlExample.cpp
//#
//#     Example code how to use sql interface
//#
//#
//#     Author:         Peter Z. Kunszt
//#
//#     Date:           Sept 6 2000
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

#include "VarVecDef.h"
#include "sqlInterface.h"


void
printRanges(size_t nRanges, ValueVector &ranges) {
  for(size_t i = 0; i < nRanges; i++) {
    PRINTID(ranges(i).lo);
    printf(" - ");
    PRINTID(ranges(i).hi);
    printf("\n");
  }
}

int
main(int argc, char *argv[]) {

	MsgStr errMsg;
	ValueVector ranges;
	size_t nRanges;
	htmSqlInterface htm(6);
	HTM_ID id;

	/* 1st Example of id lookup (using your diagnostic syntax)*/
	char lookup[100] = "J2000 6 41.4 47.9";
	errMsg = htm.lookupIDDiagnostic(lookup);
	if(errMsg.empty()) {
		id = htm.lookupID(lookup);
		printf("Id looked up using \"%s\"(1) = ",lookup);
		PRINTID(id);
		printf("\n");
	} else
		cout << errMsg;

	/* 2nd Example of id lookup (using err)
	   I like this better because you don't need
	   2 calls and you don't need an intermediate errMsg string.
	*/
	id = htm.lookupID(lookup);
	if(htm.err())
		cout << htm.error() << endl;
	else {
		printf("Id looked up (2) = ");
		PRINTID(id);
		printf("\n");
	}

	/* 1st Example of circle region */
	char circle[100] = "J2000 6 41.4 47.9";
	errMsg = htm.circleRegionDiagnostic(circle);
	if(errMsg.empty()) {
		nRanges = htm.circleRegion(circle,ranges);
		printf("Ranges for circleRegion %s\n",circle);
		printRanges(nRanges,ranges);
	} else
		cout << errMsg;
	
	/* 2nd Example of circle region */
	nRanges = htm.circleRegion(circle,ranges);
	if(htm.err())
		cout << htm.error() << endl;
	else {
		printf("Ranges for circleRegion (2) %s\n",circle);
		printRanges(nRanges,ranges);
	}

	/* Generic interface */
	strcpy(circle,"CIRCLE J2000 41.4 47.9 20");
	nRanges = htm.intersect2(circle, ranges);
	if(htm.err())
		cout << htm.error() << endl;
	else {
		printf("Ranges for circleRegion (2) %s\n",circle);
		printRanges(nRanges,ranges);
	}

	/* Example of convex hull */
	char hull[100] = "J2000 6 41.4 47.9 41.2.47.9 41.0 47.5 41.4 48";
	nRanges = htm.convexHull(hull,ranges);
	if(htm.err())
		cout << htm.error() << endl;
	else {
		printf("Ranges for convexHull %s\n",hull);
		printRanges(nRanges,ranges);
	}

	/* Generic interface */
	strcpy(hull,"CONVEX J2000 41.4 47.9 41.2.47.9 41.0 47.5 41.4 48");
	nRanges = htm.intersect2(hull, ranges);
	if(htm.err())
		cout << htm.error() << endl;
	else {
		printf("Ranges for convexHull %s\n",hull);
		printRanges(nRanges,ranges);
	}

	/* Example of domain */
	char domain[100] = "DOMAIN 6 1 2 1.0 0.0 0.0 0.0 0.0 1.0 0.0 0.0";
	nRanges = htm.domain(domain,ranges);
	if(htm.err())
		cout << htm.error() << endl;
	else
		printf("Ranges for Domain %s\n",domain);
		printRanges(nRanges,ranges);

	/* Generic interface */
	strcpy(domain,"DOMAIN 1 2 1.0 0.0 0.0 0.0 0.0 1.0 0.0 0.0");
	nRanges = htm.intersect2(domain, ranges);
	if(htm.err())
		cout << htm.error() << endl;
	else
		printf("Ranges for Domain %s\n",domain);
		printRanges(nRanges,ranges);
	return 0;
}
