// ----------------------------------------------------------------------^
// Copyright (C) 2004 - 2018
// Giorgio Calderone <giorgio.calderone@inaf.it>
// Luciano Nicastro <luciano.nicastro@inaf.it>
// 
// This file is part of DIF.
// 
// DIF is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// DIF is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with DIF; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
// 
// ----------------------------------------------------------------------$

#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
//#include <regex>
#include <algorithm>    // std::sort
#include <string>

#include "dif.hh"

//#include <my_global.h>
#include <mysql.h>

#if MY_VERSION_ID >= 80000 &&  MY_VERSION_ID < 100000
typedef bool   my_bool;
#endif
typedef long long   longlong;
typedef unsigned long long      uint64;


#include "udf_utils.hh"

// In the Calpont InfiniDB this is not set
#ifndef NOT_FIXED_DEC
#define NOT_FIXED_DEC                   31
#endif



/* Minimum cone radius or rectangle half side: ~ 1 mas - this is function
  of K and "fact" in "query_disc_inclusive" and "query_multidisc" - TBC
*/
//static const double MIN_CONE_RAD = 5e-9;
static const double MIN_CONE_DEG = 2.865e-7;

// use an approx offset for coords close to limits 0,360 and +/-90 deg
static const double MIN_OFF_DEG = 6e-4;


ThreadSpecificData<DIF_Region> difreg;
vector<long long int> DIF_Region::nullvec;

//ThreadSpecificData<XMatch> xmatch;



/* 
   This method fills the PARTIAL and FULL pixel list arrays. 

   Search is performed using criteria specified in regtype, schema, ra1, ra2,
   de1, de2, rad. The depth/order parameters to be used (either one
   or more) are stored in params array.
 */
void DIF_Region::go() {
    int i;


    if (go_performed) return;
    go_performed = true;
    params.clear();

    params = avail_params;

    if ((regtype == DIF_REG_NONE)   ||    //no region type specified
	(avail_params.size() == 0)  ||    //no param available
	(params.size()       == 0))    {  //no param chosen
	read_reset();
	return;
    }


    //Sort parameters
    sort(params.begin(), params.end());

    //prepare pixel lists
    for (i=0; i<params.size(); i++) {
	pflist.push_back(new vector<long long int>);
	pplist.push_back(new vector<long long int>);
    }




// For circle and rectangular searches, provide some criteria to find the
// best suited depth/order parameter combination among the available params
// to perform the requested search.
// At the moment the MD HTM functions just return the minimum number of IDs
// using all the available depths.



    switch (schema) {
      //------ HTM ------
    case DIF_HTM:
      switch (regtype) {
        case DIF_REG_CIRCLE:
	  DIFhtmCircleRegion(*this);
	  break;
/*
      case DIF_REG_RECT:
	DIFhtmRectRegion(*this);
	break;

      case DIF_REG_2VERT:
	DIFhtmRectRegion2V(*this); 
	break;
*/
// All rectangular area requests converted to 4 vertices version 
        case DIF_REG_4VERT:
	  DIFhtmRectRegion4V(*this); 
	  break;

        case DIF_REG_NEIGHBC:
	  DIFgetHTMNeighbC1(*this); 
	  break;

        case DIF_REG_SNEIGHB:
	  DIFgetHTMsNeighb1(*this); 
	  break;
        case DIF_REG_RECT:
        case DIF_REG_2VERT:
        case DIF_REG_NONE:
          break;
      }
      break;
      
      
      //------ HEALPix ------
      case DIF_HEALP_RING: 
      case DIF_HEALP_NEST:
      switch (regtype) {
        case DIF_REG_CIRCLE:
	  DIFmyHealPCone(*this);
	  break;

// All rectangular area requests converted to 4 vertices version 
        case DIF_REG_4VERT:
	  DIFmyHealPRect4V(*this); 
	  break;

        case DIF_REG_NEIGHBC:
	  DIFgetHealPNeighbC1(*this); 
	  break;

        case DIF_REG_RECT:
        case DIF_REG_2VERT:
        case DIF_REG_SNEIGHB:
        case DIF_REG_NONE:
          break;
      }
      break;
      case DIF_NONE:
        break;
    }
    
    read_reset();
}






//Implement UDFs
extern "C" {
  DEFINE_FUNCTION(longlong, HTMidByName);
  DEFINE_FUNCTION(longlong, HTMLookup);
  DEFINE_FUNCTION(longlong, HEALPLookup);

  DEFINE_FUNCTION(double, Sphedist);
  DEFINE_FUNCTION(double, HTMBaryDist);
  DEFINE_FUNCTION(double, HEALPBaryDist);
  DEFINE_FUNCTION(double, HEALPMaxS);

  DEFINE_FUNCTION_CHAR(char*, HTMnameById);
  DEFINE_FUNCTION_CHAR(char*, HTMBary);
  DEFINE_FUNCTION_CHAR(char*, HTMBaryC);
  DEFINE_FUNCTION_CHAR(char*, HEALPBary);
  DEFINE_FUNCTION_CHAR(char*, HEALPBaryC);
  DEFINE_FUNCTION_CHAR(char*, HTMNeighb);
  DEFINE_FUNCTION_CHAR(char*, HTMsNeighb);
  DEFINE_FUNCTION_CHAR(char*, HTMNeighbC);
  DEFINE_FUNCTION_CHAR(char*, HEALPNeighb);
  DEFINE_FUNCTION_CHAR(char*, HEALPNeighbC);
  DEFINE_FUNCTION_CHAR(char*, HEALPBound);
  DEFINE_FUNCTION_CHAR(char*, HEALPBoundC);

  DEFINE_FUNCTION(longlong, DIF_setHTMDepth);
  DEFINE_FUNCTION(longlong, DIF_setHEALPOrder);
  DEFINE_FUNCTION(longlong, DIF_clear);    
  DEFINE_FUNCTION(double  , DIF_cpuTime);    
  DEFINE_FUNCTION(longlong, DIF_FineSearch);


  DEFINE_FUNCTION(longlong, DIF_Circle);
  DEFINE_FUNCTION(longlong, DIF_Rect);
  DEFINE_FUNCTION(longlong, DIF_Rectv);

  DEFINE_FUNCTION(longlong, DIF_NeighbC);
  DEFINE_FUNCTION(longlong, DIF_sNeighb);

//  DEFINE_FUNCTION(longlong, IDMatch);
//  DEFINE_FUNCTION(longlong, CrossMatch);


  //DEFINE_FUNCTION(longlong, DIF_HTMCircle);
  //DEFINE_FUNCTION(longlong, DIF_HTMRect);
  //DEFINE_FUNCTION(longlong, DIF_HTMRectV);
  //DEFINE_FUNCTION(longlong, DIF_HTMNeighbC);
  //DEFINE_FUNCTION(longlong, DIF_HEALPCircle);
  //DEFINE_FUNCTION(longlong, DIF_HEALPNeighbC);
}






/*

//--------------------------------------------------------------------
my_bool IDMatch_init(UDF_INIT* init, UDF_ARGS *args, char *message)
{    
    const char* argerr = "IDMatch(table INT, Ra_deg DOUBLE, Dec_deg DOUBLE)";  
     
    CHECK_ARG_NUM(3);    
    CHECK_ARG_TYPE(    0, INT_RESULT);     
    CHECK_ARG_NOT_TYPE(1, STRING_RESULT);  
    CHECK_ARG_NOT_TYPE(2, STRING_RESULT);  
     
    init->ptr = NULL;    
    if (! xmatch.getp()) xmatch.constructor();   
     
     
    int table = IARGS(0);
    xmatch->clean(table);
     
    return 0;
}


longlong IDMatch(UDF_INIT *init, UDF_ARGS *args,
                 char *is_null, char* error)
{
    int table = IARGS(0);
    double ra = DARGS(1);
    double de = DARGS(2);

    return xmatch->addData(table, ra, de);
}

void IDMatch_deinit(UDF_INIT *init)
{}




//--------------------------------------------------------------------
my_bool CrossMatch_init(UDF_INIT* init, UDF_ARGS *args, char *message)
{
    const char* argerr = "CrossMatch(id1 INT, id2 INT, dist DOUBLE)";

    CHECK_ARG_NUM(3);
    CHECK_ARG_TYPE(    0, INT_RESULT);
    CHECK_ARG_TYPE(    1, INT_RESULT);
    CHECK_ARG_NOT_TYPE(2, STRING_RESULT);


    double dist = DARGS(2);

    init->ptr = NULL;
    if (! xmatch.getp()) xmatch.constructor();
    //xmatch->clean();
    xmatch->match(dist);

    return 0;
}


longlong CrossMatch(UDF_INIT *init, UDF_ARGS *args,
		    char *is_null, char* error)
{
    int id1 = IARGS(0);
    int id2 = IARGS(1);

    return xmatch->checkMatch(id1, id2);
}

void CrossMatch_deinit(UDF_INIT *init)
{ }


*/



//--------------------------------------------------------------------
my_bool HTMnameById_init(UDF_INIT* init, UDF_ARGS *args, char *message)
{
  const char* argerr = "HTMnameById(Id INT)";

  CHECK_ARG_NUM(1);
  CHECK_ARG_TYPE(0, INT_RESULT);

  //init->ptr = NULL;

  init->maybe_null = 0;
  init->max_length = 255;
  init->const_item = 0;

  return 0;
}


char * HTMnameById(UDF_INIT *init, UDF_ARGS *args,
                 char *idname, unsigned long *length,
                 char *is_null, char *error)
{
  unsigned long long int id = IARGS(0);

  //char idname[33];

  if ( getHTMnameById1(id, idname) ) {
    *error = 1;
    *is_null = 1;
    return NULL;
  } else {
    *length = (unsigned long) strlen(idname);
    //result = idname;
  }

  init->ptr = idname;
  return idname;
}

void HTMnameById_deinit(UDF_INIT *init)
//{ cleanHTMUval(init->ptr); }
{}




//--------------------------------------------------------------------
my_bool HTMidByName_init(UDF_INIT* init, UDF_ARGS *args, char *message)
{
  const char* argerr = "HTMidByName(IdName STRING)";

  CHECK_ARG_NUM(1);
  CHECK_ARG_TYPE(0, STRING_RESULT);

  init->ptr = NULL;

  return 0;
}


longlong HTMidByName(UDF_INIT *init, UDF_ARGS *args,
                   char *is_null, char* error)
{
  const char *myValue = CARGS(0);
  unsigned long long int id;

  size_t argLength = args->lengths[0];
  if (argLength > 32) {
    *error = 1;
    return 0;
  }

//char idname[18]="";
//strncpy(idname, myValue, argLength);
  char *idname = (char *)malloc(argLength+1);
  memcpy(idname, myValue, argLength);
  idname[argLength] = '\0';

  if ( getHTMidByName(init->ptr, idname, &id) )
    *error = 1;

  return id;
}

void HTMidByName_deinit(UDF_INIT *init)
{ cleanHTMUval(init->ptr); }





//--------------------------------------------------------------------
my_bool HTMLookup_init(UDF_INIT* init, UDF_ARGS *args, char *message)
{
  const char* argerr = "HTMLookup(Depth INT, Ra_deg DOUBLE, Dec_deg DOUBLE)";

  CHECK_ARG_NUM(3);
  CHECK_ARG_TYPE(    0, INT_RESULT);
  CHECK_ARG_NOT_TYPE(1, STRING_RESULT);
  CHECK_ARG_NOT_TYPE(2, STRING_RESULT);

  init->ptr = NULL;

  return 0;
}


longlong HTMLookup(UDF_INIT *init, UDF_ARGS *args,
                   char *is_null, char* error)
{
  int depth  = IARGS(0);
  double raa = DARGS(1);
  double dec = DARGS(2);
  unsigned long long int id;

  if ( getHTMid(init->ptr, depth, raa, dec, &id) )
    *error = 1;

  return id;
}

void HTMLookup_deinit(UDF_INIT *init)
{ cleanHTMUval(init->ptr); }





//--------------------------------------------------------------------
my_bool HEALPLookup_init(UDF_INIT* init, UDF_ARGS *args, char *message)
{
  const char* argerr = "HEALPLookup(nested INT, order INT, Ra_deg DOUBLE, Dec_deg DOUBLE)";

  CHECK_ARG_NUM(4);
  CHECK_ARG_TYPE(    0, INT_RESULT);
  CHECK_ARG_TYPE(    1, INT_RESULT);
  CHECK_ARG_NOT_TYPE(2, STRING_RESULT);
  CHECK_ARG_NOT_TYPE(3, STRING_RESULT);

  init->ptr = NULL;

  return 0;
}


longlong HEALPLookup(UDF_INIT *init, UDF_ARGS *args,
                     char *is_null, char* error)
{
  int nested = IARGS(0);
  int order  = IARGS(1);
  double raa = DARGS(2);
  double dec = DARGS(3);
  long long int id;

  if ( getHealPid(init->ptr, nested, order, raa, dec, &id) )
    *error = 1;

  return id;
}

void HEALPLookup_deinit(UDF_INIT *init)
{ cleanHealPUval(init->ptr); }





//--------------------------------------------------------------------
my_bool Sphedist_init(UDF_INIT* init, UDF_ARGS *args, char *message)
{
  const char* argerr = "Sphedist(Ra1_deg DOUBLE, Dec1_deg DOUBLE, Ra2_deg DOUBLE, Dec2_deg DOUBLE)";

  CHECK_ARG_NUM(4);
  CHECK_ARG_NOT_TYPE(0, STRING_RESULT);
  CHECK_ARG_NOT_TYPE(1, STRING_RESULT);
  CHECK_ARG_NOT_TYPE(2, STRING_RESULT);
  CHECK_ARG_NOT_TYPE(3, STRING_RESULT);

  init->decimals = NOT_FIXED_DEC;

  return 0;
}


double Sphedist(UDF_INIT *init, UDF_ARGS *args,
                char *is_null, char* error)
{
  return skysep_h(DARGS(0), DARGS(1), DARGS(2), DARGS(3), 0);
}


void Sphedist_deinit(UDF_INIT *init)
{}





//--------------------------------------------------------------------
my_bool HTMBaryDist_init(UDF_INIT *init, UDF_ARGS *args, char *message)
{
  const char* argerr = "HTMBaryDist(depth INT, id INT, ra DOUBLE, dec DOUBLE)";

  CHECK_ARG_NUM(4);
  CHECK_ARG_TYPE(0, INT_RESULT);
  CHECK_ARG_TYPE(1, INT_RESULT);
  CHECK_ARG_NOT_TYPE(2, STRING_RESULT);
  CHECK_ARG_NOT_TYPE(3, STRING_RESULT);

//  init->maybe_null = 0;
//  init->max_length = 255;
//  init->const_item = 0;

//  hinterface_saved = NULL;

  init->decimals = NOT_FIXED_DEC;

  return 0;
}


double HTMBaryDist(UDF_INIT *init, UDF_ARGS *args,
                   char *is_null, char *error)
{
  int order  = IARGS(0);
  unsigned long long int id = IARGS(1);
  double ra  = DARGS(2);
  double dec = DARGS(3);
//  char* saved=hinterface_saved;

//  return getHTMBaryDist(saved, order, id, ra, dec);
  return getHTMBaryDist1(order, id, ra, dec);
}


void HTMBaryDist_deinit(UDF_INIT* init)
//{ cleanHTMUval(hinterface_saved); }
{}





//--------------------------------------------------------------------
my_bool HEALPBaryDist_init(UDF_INIT *init, UDF_ARGS *args, char *message)
{
  const char* argerr = "HEALPBaryDist(nested INT, order INT, id INT, ra DOUBLE, dec DOUBLE)";

  CHECK_ARG_NUM(5);
  CHECK_ARG_TYPE(0, INT_RESULT);
  CHECK_ARG_TYPE(1, INT_RESULT);
  CHECK_ARG_TYPE(2, INT_RESULT);
  CHECK_ARG_NOT_TYPE(3, STRING_RESULT);
  CHECK_ARG_NOT_TYPE(4, STRING_RESULT);

//  init->maybe_null = 0;
//  init->max_length = 255;
//  init->const_item = 0;

//  hbase_saved = NULL;

  init->decimals = NOT_FIXED_DEC;

  return 0;
}


double HEALPBaryDist(UDF_INIT *init, UDF_ARGS *args,
                     char *is_null, char *error)
{
  int nested = IARGS(0);
  int order  = IARGS(1);
  long long int id = IARGS(2);
  double ra  = DARGS(3);
  double dec = DARGS(4);
//  char* saved=hbase_saved;

//  return getHealPBaryDist(saved, nested, order, id, ra, dec);
  return getHealPBaryDist1(nested, order, id, ra, dec);
}


void HEALPBaryDist_deinit(UDF_INIT* init)
//{ cleanHealPUval(hbase_saved); }
{}





//--------------------------------------------------------------------
my_bool HTMBary_init(UDF_INIT *init, UDF_ARGS *args, char *message)
{
  const char* argerr = "HTMBary(depth INT, id INT)";

  CHECK_ARG_NUM(2);
  CHECK_ARG_TYPE(0, INT_RESULT);
  CHECK_ARG_TYPE(1, INT_RESULT);

  init->maybe_null = 0;
  init->max_length = 255;
  init->const_item = 0;

//  hinterface_saved = NULL;

  return 0;
}


char * HTMBary(UDF_INIT *init, UDF_ARGS *args,
                 char *result, unsigned long *length,
                 char *is_null, char *error)
{
  int depth  = IARGS(0);
  unsigned long long int id = IARGS(1);
//  char* saved=hinterface_saved;

  double bc_ra, bc_dec;

//  if ( getHTMBary(saved, depth, id, &bc_ra, &bc_dec) )
  if ( getHTMBary1(depth, id, &bc_ra, &bc_dec) ) {
    *error = 1;
    *is_null = 1;
    return NULL;
  } else {
    sprintf(result,"%.16g, %.16g",bc_ra, bc_dec);
    *length = (unsigned long) strlen(result);
  }

  init->ptr = result;
  return result;
}


void HTMBary_deinit(UDF_INIT* init)
//{ cleanHTMUval(hinterface_saved); }
{}




//--------------------------------------------------------------------
my_bool HTMBaryC_init(UDF_INIT *init, UDF_ARGS *args, char *message)
{
  const char* argerr = "HTMBaryC(depth INT, ra DOUBLE, dec DOUBLE)";

  CHECK_ARG_NUM(3);
  CHECK_ARG_TYPE(0, INT_RESULT);
  CHECK_ARG_NOT_TYPE(1, STRING_RESULT);
  CHECK_ARG_NOT_TYPE(2, STRING_RESULT);

  init->maybe_null = 0;
  init->max_length = 255;
  init->const_item = 0;

//hbase_saved = (char *) malloc( sizeof(char) * 64 );
//  hinterface_saved = NULL;

  return 0;
}


char * HTMBaryC(UDF_INIT *init, UDF_ARGS *args,
                  char *result, unsigned long *length,
                  char *is_null, char *error)
{
  int depth  = IARGS(0);
  double ra  = DARGS(1);
  double dec = DARGS(2);
//  char* saved=hinterface_saved;

  double bc_ra, bc_dec;

//  if ( getHTMBaryC(saved, depth, ra, dec, &bc_ra, &bc_dec) )
  if ( getHTMBaryC1(depth, ra, dec, &bc_ra, &bc_dec) ) {
    *error = 1;
    *is_null = 1;
    return NULL;
  } else {
    sprintf(result,"%.16g, %.16g", bc_ra, bc_dec);
    *length = (unsigned long) strlen(result);
  }

  init->ptr = result;
  return result;
}


void HTMBaryC_deinit(UDF_INIT* init)
//{ cleanHTMUval(hinterface_saved); }
{}





//--------------------------------------------------------------------
my_bool HEALPBary_init(UDF_INIT *init, UDF_ARGS *args, char *message)
{
  const char* argerr = "HEALPBary(nested INT, order INT, id INT)";

  CHECK_ARG_NUM(3);
  CHECK_ARG_TYPE(0, INT_RESULT);
  CHECK_ARG_TYPE(1, INT_RESULT);
  CHECK_ARG_TYPE(2, INT_RESULT);

  init->maybe_null = 0;
  init->max_length = 255;
  init->const_item = 0;

//hbase_saved = (char *) malloc( sizeof(char) * 64 );
//  hbase_saved = NULL;

  return 0;
}


char * HEALPBary(UDF_INIT *init, UDF_ARGS *args,
                 char *result, unsigned long *length,
                 char *is_null, char *error)
{
  int nested = IARGS(0);
  int order  = IARGS(1);
  unsigned long long int id = IARGS(2);
//  char* saved=hbase_saved;

  double bc_ra, bc_dec;

//  if ( getHealPBary(saved, nested, order, id, &bc_ra, &bc_dec) )
  if ( getHealPBary1(nested, order, id, &bc_ra, &bc_dec) ) {
    *error = 1;
    *is_null = 1;
    return NULL;
  } else {
    sprintf(result,"%.16g, %.16g", bc_ra, bc_dec);
    *length = (unsigned long) strlen(result);
  }

  init->ptr = result;
  return result;
}


void HEALPBary_deinit(UDF_INIT* init)
//{ cleanHealPUval(hbase_saved); }
{}




//--------------------------------------------------------------------
my_bool HEALPBaryC_init(UDF_INIT *init, UDF_ARGS *args, char *message)
{
  const char* argerr = "HEALPBaryC(nested INT, order INT, ra DOUBLE, dec DOUBLE)";

  CHECK_ARG_NUM(4);
  CHECK_ARG_TYPE(0, INT_RESULT);
  CHECK_ARG_TYPE(1, INT_RESULT);
  CHECK_ARG_NOT_TYPE(2, STRING_RESULT);
  CHECK_ARG_NOT_TYPE(3, STRING_RESULT);

  init->maybe_null = 0;
  init->max_length = 255;
  init->const_item = 0;

//hbase_saved = (char *) malloc( sizeof(char) * 64 );
//  hbase_saved = NULL;

  return 0;
}


char * HEALPBaryC(UDF_INIT *init, UDF_ARGS *args,
                  char *result, unsigned long *length,
                  char *is_null, char *error)
{
  int nested = IARGS(0);
  int order  = IARGS(1);
  double ra  = DARGS(2);
  double dec = DARGS(3);
//  char* saved=hbase_saved;

  double bc_ra, bc_dec;

//  if ( getHealPBaryC(saved, nested, order, ra, dec, &bc_ra, &bc_dec) )
  if ( getHealPBaryC1(nested, order, ra, dec, &bc_ra, &bc_dec) ) {
    *error = 1;
    *is_null = 1;
    return NULL;
  } else {
    sprintf(result,"%.16g, %.16g", bc_ra, bc_dec);
    *length = (unsigned long) strlen(result);
  }

  init->ptr = result;
  return result;
}


void HEALPBaryC_deinit(UDF_INIT* init)
//{ cleanHealPUval(hbase_saved); }
{}





//--------------------------------------------------------------------
my_bool HTMNeighb_init(UDF_INIT *init, UDF_ARGS *args, char *message)
{
  const char* argerr = "HTMNeighb(depth INT, id INT)";

  CHECK_ARG_NUM(2);
  CHECK_ARG_TYPE(0, INT_RESULT);
  CHECK_ARG_TYPE(1, INT_RESULT);

  init->maybe_null = 0;
  init->max_length = 255;
  init->const_item = 0;

//hbase_saved = (char *) malloc( sizeof(char) * 64 );
//  hinterface_saved = NULL;
//  init->ptr = NULL;

  return 0;
}



char * HTMNeighb(UDF_INIT *init, UDF_ARGS *args,
                 char *result, unsigned long *length,
                 char *is_null, char *error)
{
  int depth = IARGS(0);
  unsigned long long int id = IARGS(1);
//  char* saved=hinterface_saved;

  vector<unsigned long long int> idn;
  char temp[20];

//  if ( getHTMNeighb(init->ptr, depth, id, idn) )
//  if ( getHTMNeighb(saved, depth, id, idn) )
  if ( getHTMNeighb1(depth, id, idn) ) {
    *error = 1;
    *is_null = 1;
    return NULL;
  } else {
    sprintf(result,"%lld", idn[0]);
    for (unsigned int i=1; i<idn.size(); i++) {
      sprintf(temp,", %lld", idn[i]);
      strcat(result,temp);
    }
    *length = (unsigned long) strlen(result);
  }

  init->ptr = result;
  return result;
}


void HTMNeighb_deinit(UDF_INIT* init)
//{ cleanHTMUval(hinterface_saved); }
//{ cleanHTMUval(init->ptr); }
{}





//--------------------------------------------------------------------
my_bool HTMsNeighb_init(UDF_INIT *init, UDF_ARGS *args, char *message)
{
  const char* argerr = "HTMsNeighb(depth INT, id INT, out_depth INT)";

  CHECK_ARG_NUM(3);
  CHECK_ARG_TYPE(0, INT_RESULT);
  CHECK_ARG_TYPE(1, INT_RESULT);
  CHECK_ARG_TYPE(2, INT_RESULT);

  init->maybe_null = 1;
//  init->max_length = 255;
  init->const_item = 0;

// Initial buffer size is 2048 chars
  init->ptr = NULL;
  if ( !(init->ptr = (char *) malloc(sizeof(char) * 2048)) ) {
        strcpy(message, "Couldn't allocate memory!");
        return 1;
   }
   memset( init->ptr, 0, sizeof(char) * 2048 );

  return 0;
}



char * HTMsNeighb(UDF_INIT *init, UDF_ARGS *args,
                 char *result, unsigned long *length,
                 char *is_null, char *error)
{
  int depth = IARGS(0);
  unsigned long long int id = IARGS(1);
  int odepth = IARGS(2);

  vector<unsigned long long int> idn;
  unsigned long nc=0;
  char temp[20];
  char *ss = init->ptr;

  if ( getHTMsNeighb1(depth, id, odepth, idn) ) {
    *error = 1;
    *is_null = 1;
    return NULL;
  } else {
    sprintf(ss,"%lld", idn[0]);
    nc = strlen(ss);
    for (unsigned int i=1; i<idn.size(); i++) {
      sprintf(temp,", %lld", idn[i]);
      nc += strlen(temp);
      if (nc > 2047) {
        char *p = (char *) realloc(ss, sizeof(char) * (nc+1));
        if (!p) {
          *error = 1;
          return 0;
        }
       ss = p;
      }
      strcat(ss,temp);
    }
    *length = (unsigned long) strlen(ss);
  }

  init->ptr = ss;
  return ss;
}


void HTMsNeighb_deinit(UDF_INIT* init)
{
  if (init->ptr)
    free(init->ptr);
}




//--------------------------------------------------------------------
my_bool HTMNeighbC_init(UDF_INIT *init, UDF_ARGS *args, char *message)
{
  const char* argerr = "HTMNeighbC(depth INT, Ra_deg DOUBLE, Dec_deg DOUBLE)";

  CHECK_ARG_NUM(3);
  CHECK_ARG_TYPE(    0, INT_RESULT);
  CHECK_ARG_NOT_TYPE(1, STRING_RESULT);
  CHECK_ARG_NOT_TYPE(2, STRING_RESULT);

  init->maybe_null = 0;
  init->max_length = 255;
  init->const_item = 0;

//if ( !(init->ptr = (char *) malloc(sizeof(char) * 256)) ) {
//        strcpy(message, "Couldn't allocate memory!");
//        return 1;
//    }
//    bzero( init->ptr, sizeof(char) * 256 );
//  init->ptr = NULL;
//  hinterface_saved = NULL;

  return 0;
}


char * HTMNeighbC(UDF_INIT *init, UDF_ARGS *args,
                  char *result, unsigned long *length,
                  char *is_null, char *error)
{
  int depth  = IARGS(0);
  double raa = DARGS(1);
  double dec = DARGS(2);

  vector<unsigned long long int> idn;
  char temp[20];
//  char* saved=hinterface_saved;

//  if ( getHTMNeighbC(saved, depth, raa, dec, idn) )
  if ( getHTMNeighbC1(depth, raa, dec, idn) ) {
    *error = 1;
    *is_null = 1;
    return NULL;
  } else {
    sprintf(result,"%lld", idn[0]);
    for (unsigned int i=1; i<idn.size(); i++) {
      sprintf(temp,", %lld", idn[i]);
      strcat(result,temp);
    }
    *length = (unsigned long) strlen(result);
  }

  init->ptr = result;
  return result;  //return init->ptr
}


void HTMNeighbC_deinit(UDF_INIT* init)
//{ cleanHTMUval(hinterface_saved); }
{}




//--------------------------------------------------------------------
my_bool HEALPNeighb_init(UDF_INIT *init, UDF_ARGS *args, char *message)
{
  const char* argerr = "HEALPNeighb(nested INT, order INT, id INT)";

  CHECK_ARG_NUM(3);
  CHECK_ARG_TYPE(0, INT_RESULT);
  CHECK_ARG_TYPE(1, INT_RESULT);
  CHECK_ARG_TYPE(2, INT_RESULT);

  init->maybe_null = 0;
  init->max_length = 255;
  init->const_item = 0;

//hbase_saved = (char *) malloc( sizeof(char) * 64 );
//  hbase_saved = NULL;

  return 0;
}


char * HEALPNeighb(UDF_INIT *init, UDF_ARGS *args,
                   char *result, unsigned long *length,
                   char *is_null, char *error)
{
  int nested = IARGS(0);
  int order  = IARGS(1);
  long long int id = IARGS(2);
//  char* saved=hbase_saved;

  vector<long long int> idn;
  char temp[20];

//  if ( getHealPNeighb(saved, nested, order, id, idn) )
  if ( getHealPNeighb1(nested, order, id, idn) ) {
    *error = 1;
    *is_null = 1;
    return NULL;
  } else {
    sprintf(result,"%lld", idn[0]);
    for (unsigned int i=1; i<idn.size(); i++) {
      sprintf(temp,", %lld", idn[i]);
      strcat(result,temp);
    }
    *length = (unsigned long) strlen(result);
  }

  init->ptr = result;
  return result;
}


void HEALPNeighb_deinit(UDF_INIT* init)
//{ cleanHealPUval(hbase_saved); }
{}



//--------------------------------------------------------------------
my_bool HEALPNeighbC_init(UDF_INIT *init, UDF_ARGS *args, char *message)
{
  const char* argerr = "HEALPNeighbC(nested INT, order INT, Ra_deg DOUBLE, Dec_deg DOUBLE)";

  CHECK_ARG_NUM(4);
  CHECK_ARG_TYPE(    0, INT_RESULT);
  CHECK_ARG_TYPE(    1, INT_RESULT);
  CHECK_ARG_NOT_TYPE(2, STRING_RESULT);
  CHECK_ARG_NOT_TYPE(3, STRING_RESULT);

  init->maybe_null = 0;
  init->max_length = 255;
  init->const_item = 0;

//if ( !(init->ptr = (char *) malloc(sizeof(char) * 256)) ) {
//        strcpy(message, "Couldn't allocate memory!");
//        return 1;
//    }
//    bzero( init->ptr, sizeof(char) * 256 );
//  init->ptr = NULL;
//  hbase_saved = NULL;

  return 0;
}


char * HEALPNeighbC(UDF_INIT *init, UDF_ARGS *args,
                    char *result, unsigned long *length,
                    char *is_null, char *error)
{
  int nested = IARGS(0);
  int order  = IARGS(1);
  double raa = DARGS(2);
  double dec = DARGS(3);

  vector<long long int> idn;
  char temp[20];
//  char* saved=hbase_saved;

//  if ( getHealPNeighbC(saved, nested, order, raa, dec, idn) )
  if ( getHealPNeighbC1(nested, order, raa, dec, idn) ) {
    *error = 1;
    *is_null = 1;
    return NULL;
  } else {
    sprintf(result,"%lld", idn[0]);
    for (unsigned int i=1; i<idn.size(); i++) {
      sprintf(temp,", %lld", idn[i]);
      strcat(result,temp);
    }
    *length = (unsigned long) strlen(result);
  }

  init->ptr = result;
  return result;  //return init->ptr
}


void HEALPNeighbC_deinit(UDF_INIT* init)
//{ cleanHealPUval(hbase_saved); }
{}



//--------------------------------------------------------------------
my_bool HEALPBound_init(UDF_INIT *init, UDF_ARGS *args, char *message)
{
  const char* argerr = "HEALPBound(nested INT, order INT, id INT [, step INT])";

  switch (args->arg_count) {
  case 4:
    CHECK_ARG_TYPE(3, INT_RESULT);
  case 3:
    CHECK_ARG_TYPE(0, INT_RESULT);
    CHECK_ARG_TYPE(1, INT_RESULT);
    CHECK_ARG_TYPE(2, INT_RESULT);
    break;
  default:
    CHECK_ARG_NUM(4);  //Raise an error
  }


  init->maybe_null = 0;
  //init->max_length = 255;
  init->const_item = 0;

// Initial buffer size is 2048 chars
  init->ptr = NULL;
  if ( !(init->ptr = (char *) malloc(sizeof(char) * 2048)) ) {
        strcpy(message, "Couldn't allocate memory!");
        return 1;
   }
   memset( init->ptr, 0, sizeof(char) * 2048 );

  return 0;
}


char * HEALPBound(UDF_INIT *init, UDF_ARGS *args,
                   char *result, unsigned long *length,
                   char *is_null, char *error)
{
  int nested = IARGS(0);
  int order  = IARGS(1);
  long long int id = IARGS(2);
  unsigned int step = 1;
  if (args->arg_count == 4)
    step = IARGS(3);

  vector<double> b_ra, b_dec;
  unsigned long nc=0;
  char temp[51];
  char *ss = init->ptr;

  if ( getHealPBound1(nested, order, id, step, b_ra, b_dec) ) {
    *error = 1;
    *is_null = 1;
    return NULL;
  } else {
    sprintf(ss,"%.16g, %.16g", b_ra[0], b_dec[0]);
    nc = strlen(ss);
    for (unsigned int i=1; i<b_ra.size(); i++) {
      sprintf(temp,", %.16g, %.16g", b_ra[i], b_dec[i]);
      nc += strlen(temp);
      if (nc > 2047) {
        char *p = (char *) realloc(ss, sizeof(char) * (nc+1));
        if (!p) {
          *error = 1;
          return 0;
        }
       ss = p;
      }
      strcat(ss,temp);
    }
    *length = (unsigned long) strlen(ss);
  }

  init->ptr = ss;
  return ss;
}


void HEALPBound_deinit(UDF_INIT* init)
//{ cleanHealPUval(hbase_saved); }
{}



//--------------------------------------------------------------------
my_bool HEALPBoundC_init(UDF_INIT *init, UDF_ARGS *args, char *message)
{
  const char* argerr = "HEALPBoundC(nested INT, order INT, RA_deg DOUBLE, Dec_deg DOUBLE [, step INT])";

  switch (args->arg_count) {
  case 5:
    CHECK_ARG_TYPE(4, INT_RESULT);
  case 4:
    CHECK_ARG_TYPE(0, INT_RESULT);
    CHECK_ARG_TYPE(1, INT_RESULT);
    CHECK_ARG_NOT_TYPE(2, STRING_RESULT);
    CHECK_ARG_NOT_TYPE(3, STRING_RESULT);
    break;
  default:
    CHECK_ARG_NUM(5);  //Raise an error
  }

  init->maybe_null = 0;
  //init->max_length = 255;
  init->const_item = 0;

// Initial buffer size is 2048 chars
  init->ptr = NULL;
  if ( !(init->ptr = (char *) malloc(sizeof(char) * 2048)) ) {
        strcpy(message, "Couldn't allocate memory!");
        return 1;
   }
   memset( init->ptr, 0, sizeof(char) * 2048 );

  return 0;
}


char * HEALPBoundC(UDF_INIT *init, UDF_ARGS *args,
                   char *result, unsigned long *length,
                   char *is_null, char *error)
{
  int nested = IARGS(0);
  int order  = IARGS(1);
  double ra = DARGS(2);
  double de = DARGS(3);
  unsigned int step = 1;
  if (args->arg_count == 5)
    step = IARGS(4);

  vector<double> b_ra, b_dec;
  unsigned long nc=0;
  char temp[51];
  char *ss = init->ptr;

  if ( getHealPBoundC1(nested, order, ra, de, step, b_ra, b_dec) ) {
    *error = 1;
    *is_null = 1;
    return NULL;
  } else {
    sprintf(ss,"%.16g, %.16g", b_ra[0], b_dec[0]);
    nc = strlen(ss);
    for (unsigned int i=1; i<b_ra.size(); i++) {
      sprintf(temp,", %.16g, %.16g", b_ra[i], b_dec[i]);
      nc += strlen(temp);
      if (nc > 2047) {
        char *p = (char *) realloc(ss, sizeof(char) * (nc+1));
        if (!p) {
          *error = 1;
          return 0;
        }
       ss = p;
      }
      strcat(ss,temp);
    }
    *length = (unsigned long) strlen(ss);
  }

  init->ptr = ss;
  return ss;
}


void HEALPBoundC_deinit(UDF_INIT* init)
//{ cleanHealPUval(hbase_saved); }
{}



//--------------------------------------------------------------------
my_bool HEALPMaxS_init(UDF_INIT *init, UDF_ARGS *args, char *message)
{
  const char* argerr = "HEALPMaxS(order INT)";

  CHECK_ARG_NUM(1);
  CHECK_ARG_TYPE(0, INT_RESULT);

//  init->maybe_null = 0;
//  init->max_length = 255;
//  init->const_item = 0;

//hbase_saved = (char *) malloc( sizeof(char) * 64 );
//  hbase_saved = NULL;

  init->decimals = NOT_FIXED_DEC;

  return 0;
}


double HEALPMaxS(UDF_INIT *init, UDF_ARGS *args,
                 char *is_null, char *error)
{
  int order  = IARGS(0);
//  char* saved=hbase_saved;

//  if ( getHealPMaxS(saved, order, maxs) )
  return getHealPMaxS1(order);
}


void HEALPMaxS_deinit(UDF_INIT* init)
{}



//--------------------------------------------------------------------
my_bool DIF_setHTMDepth_init(UDF_INIT* init, UDF_ARGS *args, char *message)
{
  const char* argerr = "DIF_setHTMDepth(depth INT)";

  CHECK_ARG_NUM(1);
  CHECK_ARG_TYPE(0, INT_RESULT);

  init->maybe_null = 0;
  init->max_length = 4;
  init->const_item = 1;

  if (! difreg.getp()) difreg.constructor();
  //difreg->clear_region();
  difreg->setSchema(DIF_HTM);

// 14/11/2018: changed for MySQL 8
// NOTE: args->args can be undefined alias (AS) are being used. Use "attributes".
//int depth = IARGS(0);
#if MY_VERSION_ID >= 80000 &&  MY_VERSION_ID < 100000
int depth = atoi(args->attributes[0]);
#else
int depth = *((long long*) args->args[0]);
#endif

  difreg->setAvailParam( depth );

//sprintf(message, "\n args[0]=%s n=%lu depth=%d\n", (char *)args->attributes[0], args->lengths[0], depth );

  return 0;
}


longlong DIF_setHTMDepth(UDF_INIT *init, UDF_ARGS *args,
                         char *is_null, char* error)
{
 //return *(args->args[0]);
 return 1;
 }

void DIF_setHTMDepth_deinit(UDF_INIT *init)
{
    difreg->clear_pixel();
}



//--------------------------------------------------------------------
my_bool DIF_setHEALPOrder_init(UDF_INIT* init, UDF_ARGS *args, char *message)
{
  const char* argerr = "DIF_setHEALPOrder(nested INT, order INT)";

  CHECK_ARG_NUM(2);
  CHECK_ARG_TYPE(0, INT_RESULT);
  CHECK_ARG_TYPE(1, INT_RESULT);

  init->maybe_null = 0;
  init->max_length = 4;
  init->const_item = 1;

  if (! difreg.getp()) difreg.constructor();
  //difreg->clear_region();
  // 5/7/2016 exchange: difreg->setSchema(IARGS(0)   ?   DIF_HEALP_RING   :   DIF_HEALP_NEST);
// 14/11/2018: changed for MySQL 8
  //difreg->setSchema(IARGS(0)   ?   DIF_HEALP_NEST   :   DIF_HEALP_RING);
  //difreg->setAvailParam( IARGS(1) );

#if MY_VERSION_ID >= 80000 &&  MY_VERSION_ID < 100000
  int nested = atoi(args->attributes[0]);
  int order = atoi(args->attributes[1]);
#else
  int nested = IARGS(0);
  int order = IARGS(1);
#endif
  difreg->setSchema(nested   ?   DIF_HEALP_NEST   :   DIF_HEALP_RING);
  difreg->setAvailParam( order );

  return 0;
}


longlong DIF_setHEALPOrder(UDF_INIT *init, UDF_ARGS *args,
                           char *is_null, char* error)
{ return 1; }

void DIF_setHEALPOrder_deinit(UDF_INIT *init)
{
    difreg->clear_pixel();
}






//--------------------------------------------------------------------
my_bool DIF_clear_init(UDF_INIT* init, UDF_ARGS *args, char *message)
{
  const char* argerr = "DIF_clear()";

  if (! difreg.getp()) difreg.constructor();
  difreg->clear_region();
  return 0;
}

longlong DIF_clear(UDF_INIT *init, UDF_ARGS *args,
                         char *is_null, char* error)
{ return 1; }

void DIF_clear_deinit(UDF_INIT *init)
{}




//--------------------------------------------------------------------
my_bool DIF_cpuTime_init(UDF_INIT* init, UDF_ARGS *args, char *message)
{
  init->decimals = NOT_FIXED_DEC;

  return 0;
}

double DIF_cpuTime(UDF_INIT *init, UDF_ARGS *args, char *is_null, char* error)
{
    if (difreg.getp())
	return difreg->cpuTime();

    return 0;
}

void DIF_cpuTime_deinit(UDF_INIT *init)
{}

     



//--------------------------------------------------------------------
my_bool DIF_FineSearch_init(UDF_INIT* init, UDF_ARGS *args, char *message)
{
  const char* argerr = "DIF_FineSearch(...)";

// Always 3 params: RA, Dec, full_pixel_flag
  if (difreg.getp()) {
    switch (difreg->regtype) {
      case DIF_REG_CIRCLE:
      case DIF_REG_4VERT:
      case DIF_REG_NEIGHBC:
      case DIF_REG_SNEIGHB:
        CHECK_ARG_NUM(3);
        CHECK_ARG_NOT_TYPE(0, STRING_RESULT);
        CHECK_ARG_NOT_TYPE(1, STRING_RESULT);
        CHECK_ARG_NOT_TYPE(2, STRING_RESULT);
        break;

      case DIF_REG_RECT:
      case DIF_REG_2VERT:
      case DIF_REG_NONE:
        break;
     }
  }

  return 0;
}



longlong DIF_FineSearch(UDF_INIT *init, UDF_ARGS *args,
                        char *is_null, char* error)
{
    double side1, side2, sep;
    double ra = DARGS(0);
    double de = DARGS(1);
    double ra1 = difreg->ra1;
    double de1 = difreg->de1;
    double ra2 = difreg->ra3;    // Clockwise coords. See DIF_Rectv_init !
    double de2 = difreg->de2;
    double rad = difreg->rad;
    longlong ret = 0;

    if (*(args->args[2]))
      return 1; //If the pixel is "full" return immediately

    if (! difreg.getp())
      return 0;
    difreg->subStart();
    
    unsigned short ra1border=0; // Toggle for negative start RA range

    switch (difreg->regtype) {
	case DIF_REG_CIRCLE:
	    sep = skysep_h(ra1, de1, ra, de, 0);
	    if (sep >= 0. && sep <= rad)
		ret = 1;
	    break;

// Note:
// the rectangle is always defined by its 4 vertices therefore the
// two following cases should never be verified!
/*
	case DIF_REG_RECT:
	    //Actually ra2 and de2 are the sides in arcmin
	    side1 = ra2 / 60.; //arcmin to degree
	    side2 = de2 / 60.;
	    if (side2 == 0) { side2 = side1; }
	    ra2 = ra1 + side1 / 2.;
	    de2 = de1 + side2 / 2.;
	    ra1 = ra1 - side1 / 2.;
	    de1 = de1 - side2 / 2.;
	    
	    //At this point we know that ra2 > 0
	    if ((ra1 > ra2)    ||    (ra2 > 360.))   {  //Cross 0
		if (ra2 > 360.)
		    ra2 -= 360.;
		
		if (
		    ((ra1 < ra  &&  ra < 360.)  ||  (0. <= ra  &&  ra < ra2))   &&
		    (de1 < de  &&  de < de2))
		    ret = 1;
	    }
	    else {  //Simple range check
		if (ra1 < ra    &&   ra < ra2   &&
		    de1 < de    &&   de < de2)
		    ret = 1;
	    }
	    break;

	case DIF_REG_2VERT:
	    //At this point we know that ra2 > 0
	    if ((ra1 > ra2)    ||    (ra2 > 360.))   {  //Cross 0
		if (ra2 > 360.)
		    ra2 -= 360.;
		
		if (
		    ((ra1 < ra  &&  ra < 360.)  ||  (0. <= ra  &&  ra < ra2))   &&
		    (de1 < de  &&  de < de2))
		    ret = 1;
	    }
	    else {  //Simple range check
		if (ra1 < ra    &&   ra < ra2   &&
		    de1 < de    &&   de < de2)
		    ret = 1;
	    }
	    break;
*/

        case DIF_REG_4VERT: //Simple range check
/* Check for 0 crossing effects: only two cases considered here! See also DIF_Rectv_init */
          if (ra1 < 0.)
          {
            ra1 += 360.;
            ra1border = 1;
          } else if ((ra1 > ra2) && (ra1-ra2 > 180.))
            ra1border = 1;

	  if (ra1border) {
            if ((ra1 <= ra    ||   ra <= ra2)   &&
                (de1 <= de    &&   de <= de2))     // Clockwise coords
	      ret = 1;
	  } else {
            if ((ra1 <= ra    &&   ra <= ra2)   &&
                (de1 <= de    &&   de <= de2))     // Clockwise coords
	      ret = 1;
	  }
	  break;

	case DIF_REG_NEIGHBC:
	    ret = 1;
	    break;

	case DIF_REG_SNEIGHB:
	    ret = 1;
	    break;

	case DIF_REG_NONE:
	    break;
    }


    difreg->subStop();
    return ret;
}


void DIF_FineSearch_deinit(UDF_INIT *init)
{}




//--------------------------------------------------------------------
my_bool DIF_Circle_init(UDF_INIT* init, UDF_ARGS *args, char *message)
{
  const char* argerr = "DIF_Circle(Ra_deg DOUBLE, Dec_deg DOUBLE, Rad_arcmin DOUBLE)";

  CHECK_ARG_NUM(3);
  CHECK_ARG_NOT_TYPE(0, STRING_RESULT);
  CHECK_ARG_NOT_TYPE(1, STRING_RESULT);
  CHECK_ARG_NOT_TYPE(2, STRING_RESULT);

  if (! difreg.getp()) difreg.constructor();
  difreg->regtype = DIF_REG_CIRCLE;
  difreg->ra1 = DARGS(0);
  difreg->de1 = DARGS(1);
  difreg->rad = DARGS(2);

// Force to minimum radius - should alert 
  if (difreg->rad < MIN_CONE_DEG) 
    difreg->rad = MIN_CONE_DEG;

  return 0;
}


longlong DIF_Circle(UDF_INIT *init, UDF_ARGS *args,
                       char *is_null, char* error)
{ return 1; }


void DIF_Circle_deinit(UDF_INIT *init)
{}



//--------------------------------------------------------------------
my_bool DIF_Rect_init(UDF_INIT* init, UDF_ARGS *args, char *message)
{
//This function requires the coordinates of the center of the rectangular region and one or two sides.
  const char* argerr = "DIF_Rect(Ra_deg DOUBLE, Dec_deg DOUBLE, side_ra_arcmin DOUBLE [, side_dec_arcmin DOUBLE])";

  switch (args->arg_count) { 
  case 4:
    CHECK_ARG_NOT_TYPE(3, STRING_RESULT);
  case 3:
    CHECK_ARG_NOT_TYPE(0, STRING_RESULT);
    CHECK_ARG_NOT_TYPE(1, STRING_RESULT);
    CHECK_ARG_NOT_TYPE(2, STRING_RESULT);
    break;
  default:
    CHECK_ARG_NUM(4);  //Raise an error
  }

  if (! difreg.getp()) difreg.constructor();
//  difreg->clear_region();
//  difreg->regtype = DIF_REG_RECT;

  double cra = DARGS(0);
  double cde = DARGS(1);
  double hside_ra = DARGS(2)/120.;
// Force to minimum side length - should alert 
  if (hside_ra < MIN_CONE_DEG) 
    hside_ra = MIN_CONE_DEG;
  //double hside_de = (args->arg_count == 4  ?  DARGS(3)/120.  :  hside_ra);
  double hside_de = hside_ra;
  if (args->arg_count == 4) {
    hside_de = DARGS(3)/120.;
    if (hside_de < MIN_CONE_DEG) 
      hside_de = MIN_CONE_DEG;
  }

// Some parameter checks
  while (cra < 0.) cra += 360.;

  if ( (! (  0. <= cra      &&  cra       <  360.))  ||
       (! (-90. <= cde      &&  cde       <=  90.))  ||
       (! (  0. < hside_ra  &&  hside_ra  <= 180.))  ||
       (! (  0. < hside_de  &&  hside_de  <=  90.)) ) {
    strcpy(message, argerr);
    strcat(message, ": invalid parameters given");
    return 1;
  }

  hside_ra /= cos(cde*M_PI/180.);

// Note:
// always calculate the 4 corners in order to use always the
// DIF_REG_4VERT region type; clockwise coords from SW !
  double ra1 = cra - hside_ra;
  double de1 = cde - hside_de;
  double ra3 = cra + hside_ra;
  double de2 = cde + hside_de;
  bool is_ring = false;

  if (hside_ra >= 180.) {
    is_ring = true;
    hside_ra = 180.;
    ra1 = MIN_OFF_DEG;
    ra3 = 360.;
  } else {
    if (ra1 < 0.)
      ra1 += 360.;

    if (ra3 > 360.)
      ra3 -= 360.;
  }
  difreg->ra1 = ra1;
  difreg->ra3 = ra3;
  difreg->ra2 = difreg->ra1;
  difreg->ra4 = difreg->ra3;

  if (difreg->getSchema() == DIF_HEALP_NEST || difreg->getSchema() == DIF_HEALP_RING)
  {
    if (de1 < -90.) {
      //if (is_ring)
        de1 = -90. + MIN_OFF_DEG;
      //else
        //de1 = -180. - de1 ;
    } else if (de1 == -90.)
      de1 += MIN_OFF_DEG;

    if (de2 >  90.) {
      //if (is_ring)
        de2 = 90. - MIN_OFF_DEG;
      //else
        //de2 = 180. - de2;
    } else if (de2 == 90.)
      de2 -= MIN_OFF_DEG;
  }

  if (de1 < de2) {
    difreg->de1 = de1;
    difreg->de2 = de2;
  } else {
    difreg->de1 = de2;
    difreg->de2 = de1;
  }
  difreg->de3 = difreg->de2;
  difreg->de4 = difreg->de1;

//sprintf(message, "%13.8lf,%13.8lf %13.8lf,%13.8lf  \0", difreg->ra1,difreg->de1, difreg->ra3,difreg->de2);
//return 1;
  difreg->regtype = DIF_REG_4VERT;
  return 0;
}


longlong DIF_Rect(UDF_INIT *init, UDF_ARGS *args,
                     char *is_null, char* error)
{ return 1; }


void DIF_Rect_deinit(UDF_INIT *init)
{}







//--------------------------------------------------------------------
my_bool DIF_Rectv_init(UDF_INIT* init, UDF_ARGS *args, char *message)
{
//This function requires the coordinates of the 2 opposite (or 4) corners of the rectangular region.
  const char* argerr = "DIF_Rectv(Ra1_deg DOUBLE, Dec1_deg DOUBLE, Ra2_deg DOUBLE, Dec2_deg DOUBLE [, x 2])";


  switch (args->arg_count) {
  case 8:
    CHECK_ARG_NOT_TYPE(4, STRING_RESULT);
    CHECK_ARG_NOT_TYPE(5, STRING_RESULT);
    CHECK_ARG_NOT_TYPE(6, STRING_RESULT);
    CHECK_ARG_NOT_TYPE(7, STRING_RESULT);
  case 4:
    CHECK_ARG_NOT_TYPE(0, STRING_RESULT);
    CHECK_ARG_NOT_TYPE(1, STRING_RESULT);
    CHECK_ARG_NOT_TYPE(2, STRING_RESULT);
    CHECK_ARG_NOT_TYPE(3, STRING_RESULT);
    break;
  default:
    CHECK_ARG_NUM(4);  //Raise an error
  }

  if (! difreg.getp()) difreg.constructor();
//  difreg->clear_region();
//  difreg->regtype = DIF_REG_RECT;

  double ra1, de1, ra2, de2;
  bool is_ring = false;

  if (args->arg_count == 4) {
    ra1 = DARGS(0);
    de1 = DARGS(1);
    ra2 = DARGS(2);
    de2 = DARGS(3);

    // Sort RA
    if (ra1 > ra2) {
      double ratemp = ra1;
      ra1 = ra2;
      ra2 = ratemp;
    }
 
    // Sort Dec
    if (de1 > de2) {
      double detemp = de1;
      de1 = de2;
      de2 = detemp;
    }
 
  } else {
    vector<double> ra(4,0.), de(4,0.);
    for (int i=0; i<8; i+=2) {
      ra[i/2] = DARGS(i);
      de[i/2] = DARGS(i+1);
    }

// Use maximum rectangle including the given coordinates
    sort(ra.begin(),ra.end());
    sort(de.begin(),de.end());
    ra1 = ra[0];
    de1 = de[0];
    ra2 = ra[3];
    de2 = de[3];
  }

  if (ra1 < 0.)
    ra1 += 360.;

  if (ra2 > 360.)
    ra2 -= 360.;

  if (ra1 == ra2) {
    ra1 = MIN_OFF_DEG;
    ra2 = 360.;
  }

// Order ranges (clockwise) assuming a convex
  if (ra1 < ra2) {
    difreg->ra1 = ra1;
    difreg->ra3 = ra2;
  } else {
    difreg->ra1 = ra2;
    difreg->ra3 = ra1;
  }
  difreg->ra2 = difreg->ra1;
  difreg->ra4 = difreg->ra3;

  if (difreg->getSchema() == DIF_HEALP_NEST || difreg->getSchema() == DIF_HEALP_RING)
  {
    if (de1 < -90.) {
      //if (is_ring)
        de1 = -90. + MIN_OFF_DEG;
      //else
        //de1 = -180. - de1 ;
    } else if (de1 == -90.)
      de1 += MIN_OFF_DEG;

    if (de2 > 90.) {
      //if (is_ring)
        de2 = 90. - MIN_OFF_DEG;
      //else
        //de2 = 180. - de2;
    } else if (de2 == 90.)
      de2 -= MIN_OFF_DEG;
  }

  if (de1 == de2) {
    de1 = MIN_OFF_DEG;
    de2 = 90. - MIN_OFF_DEG;
  }

  if (de1 < de2) {
    difreg->de1 = de1;
    difreg->de2 = de2;
  } else {
    difreg->de1 = de2;
    difreg->de2 = de1;
  }
  difreg->de3 = difreg->de2;
  difreg->de4 = difreg->de1;

//sprintf(message, "%13.8lf,%13.8lf %13.8lf,%13.8lf  \0", difreg->ra1,difreg->de1, difreg->ra3,difreg->de2);
//return 1;
  difreg->regtype = DIF_REG_4VERT;
  return 0;
}


longlong DIF_Rectv(UDF_INIT *init, UDF_ARGS *args,
                     char *is_null, char* error)
{ return 1; }


void DIF_Rectv_deinit(UDF_INIT *init)
{}




//--------------------------------------------------------------------
my_bool DIF_NeighbC_init(UDF_INIT *init, UDF_ARGS *args, char *message)
{
  const char* argerr = "DIF_NeighbC(Ra_deg DOUBLE, Dec_deg DOUBLE)";

  CHECK_ARG_NUM(2);
  CHECK_ARG_NOT_TYPE(0, STRING_RESULT);
  CHECK_ARG_NOT_TYPE(1, STRING_RESULT);

  if (! difreg.getp()) difreg.constructor();
  difreg->regtype = DIF_REG_NEIGHBC;
  difreg->ra1 = DARGS(0);
  difreg->de1 = DARGS(1);

  return 0;
}


longlong DIF_NeighbC(UDF_INIT *init, UDF_ARGS *args,
                        char *is_null, char *error)
{ return 1; }


void DIF_NeighbC_deinit(UDF_INIT* init)
{}




//--------------------------------------------------------------------
my_bool DIF_sNeighb_init(UDF_INIT *init, UDF_ARGS *args, char *message)
{
  const char* argerr = "DIF_sNeighb(in_depth INT, id INT, out_depth INT)";

  CHECK_ARG_NUM(3);
  CHECK_ARG_TYPE(0, INT_RESULT);
  CHECK_ARG_TYPE(1, INT_RESULT);
  CHECK_ARG_TYPE(2, INT_RESULT);

  if (! difreg.getp()) difreg.constructor();
  difreg->regtype = DIF_REG_SNEIGHB;
  difreg->indepth = IARGS(0);
  difreg->refpix = DARGS(1);
  difreg->outdepth = DARGS(2);

  return 0;
}


longlong DIF_sNeighb(UDF_INIT *init, UDF_ARGS *args,
                        char *is_null, char *error)
{ return 1; }


void DIF_sNeighb_deinit(UDF_INIT* init)
{}













////--------------------------------------------------------------------
//my_bool DIF_HTMRectV_init(UDF_INIT* init, UDF_ARGS *args, char *message)
//{
//  //This function requires the coordinate of the two opposite corners
//  //of a "rectangle" or the coordinates of the four corners of a
//  //parallelogram. Strictly speaking the edges of the "rectangles"
//  //lie along costant RA or Dec.
//  const char* argerr = "DIF_HTMRectV(Ra1_deg DOUBLE, Dec1_deg DOUBLE, Ra2_deg DOUBLE, Dec2_deg DOUBLE) [ x 2 ]";
//
//  //8-args version
//  switch (args->arg_count) {
//// Disabled!
////  case 8:
////    CHECK_ARG_NOT_TYPE(4, STRING_RESULT);
////    CHECK_ARG_NOT_TYPE(5, STRING_RESULT);
////    CHECK_ARG_NOT_TYPE(6, STRING_RESULT);
////    CHECK_ARG_NOT_TYPE(7, STRING_RESULT);
//  case 4:
//    CHECK_ARG_NOT_TYPE(0, STRING_RESULT);
//    CHECK_ARG_NOT_TYPE(1, STRING_RESULT);
//    CHECK_ARG_NOT_TYPE(2, STRING_RESULT);
//    CHECK_ARG_NOT_TYPE(3, STRING_RESULT);
//    break;
//  default:
////    CHECK_ARG_NUM(8);
//    CHECK_ARG_NUM(4); //Raise an error
//  }
//
//
//  if (! difreg.getp()) difreg.constructor();
//  difreg->clear_region();
//  difreg->regtype = DIF_REG_2VERT;
//  difreg->ra1 = DARGS(0);
//  difreg->de1 = DARGS(1);
//  difreg->ra2 = DARGS(2);
//  difreg->de2 = DARGS(3);
//
//  return 0;
//}
//
//
//longlong DIF_HTMRectV(UDF_INIT *init, UDF_ARGS *args,
//                      char *is_null, char* error)
//{ return 1; }
//
//void DIF_HTMRectV_deinit(UDF_INIT *init)
//{}
//
//
//
//
//
//
//
//
//
////--------------------------------------------------------------------
//my_bool DIF_HEALPNeighbC_init(UDF_INIT *init, UDF_ARGS *args, char *message)
//{
// const char* argerr = "DIF_HEALPNeighbC(ra DOUBLE, dec DOUBLE)";
//
//  CHECK_ARG_NUM(2);
//  CHECK_ARG_NOT_TYPE(0, STRING_RESULT);
//  CHECK_ARG_NOT_TYPE(1, STRING_RESULT);
//
//  if (! difreg.getp()) difreg.constructor();
//  difreg->clear_region();
//  difreg->regtype = DIF_REG_NEIGHBC;
//  difreg->ra1 = DARGS(0);
//  difreg->de1 = DARGS(1);
//  difreg->rad = DARGS(2);
//
//  return 0;
//}
//
//
//longlong DIF_HEALPNeighbC(UDF_INIT *init, UDF_ARGS *args,
//                          char *is_null, char *error)
//{ return 1; }
//
//void DIF_HEALPNeighbC_deinit(UDF_INIT* init)
//{}
