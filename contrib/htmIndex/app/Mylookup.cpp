//#     Filename:       lookup.cpp
//#
//#     specify a point on the sphere, return its ID/Name to a certain depth
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

#include <cstdlib>
#include <VarVecDef.h>
#include "VarStr.h"
#include "SpatialVector.h"
#include "SpatialInterface.h"

int getHTMid(float ra, float dec, uint64 *id, char *name);

int main(){
	uint64 id=0;
	char name[200];
	
	getHTMid(10.,-10.,&id,name);
	printf("id=%lu\n",id);
	printf("%s\n",name);
	return 0;
}


int getHTMid(float ra, float dec, uint64 *id, char *name) 
{

  size_t depth=6,savedepth=2, n=1;
  //char *idname;
  
  htmInterface *htm;
  VarStr cmd;
      char str[200];
	 
  try {
    
//    htmInterface htm(depth,savedepth);
    htm = new htmInterface(depth);
	//sprintf(str,"J2000 6 %20.16f %20.16f",depth,ra,dec);
	cmd = str;
//	*id = htm.lookupID(ra,dec);
	*id = htm->lookupID(ra,dec);
//	strcpy(name,htm.lookupName((float64) ra, (float64) dec) );
	strcpy(name,htm->lookupName((float64) ra, (float64) dec) );
	
  } catch (SpatialException x) {
    printf("%s\n",x.what());
  }
	delete htm;
   return 0;
}
