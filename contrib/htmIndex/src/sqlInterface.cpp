//#     Filename:       SpatialInterface.cpp
//#
//#     The htmSqlInterface class is defined here.
//#
//#     Author:         Peter Z. Kunszt 
//#
//#     Date:           August 30 , 2000
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
//#
//#     Modification History:
//#
#include "sqlInterface.h"

//==============================================================
//
// These are the implementations of the htm interface.
//
//==============================================================

///////////CONSTRUCTOR///////////////////////
htmSqlInterface::htmSqlInterface(size_t depth) : err_(false) {
  char d[10];
  sprintf(d,"%d",(int)depth);
  depth_ = d;
}

///////////LOOKUP METHODS////////////////////
MsgStr
htmSqlInterface::lookupIDDiagnostic(char *str) {
  result_ = nONE;
  try {
    err_ = false;
    resID_ = htm_.lookupIDCmd(str);
	result_ = lOOKUP;
	error_.clear();
  } catch (SpatialException &x) {
    error_ = x.what();
    err_ = true;
  }
  return error_;
}


HTM_ID htmSqlInterface::lookupID(char *str) {

  if(result_ == lOOKUP) {
	result_ = nONE;
	return resID_;
  }
  try {
    err_ = false;
    return htm_.lookupIDCmd(str);
  } catch (SpatialException &x) {
    error_ = x.what();
    err_ = true;
  }
  return 0;
}

/////////////////////General Intersect////////////////
size_t
htmSqlInterface::intersect1( char *s, ValVec<htmRange> &vec ) {

  char *str = s;
  VarStr key;
  int i = parseKey( str, key );
  if(i < 0)return 0;

  if( key == "DOMAIN" )
    return domain(str, vec);
  if( key == "CONVEX" )
    return convexHull( str+i, vec);
  if( key == "CIRCLE" )
    return circleRegion( str+i, vec);
  error_ = "htmSqlInterface::intersect: Unrecognized keyword ";
  error_ += key;
  err_ = true;
  return 0;

}

size_t
htmSqlInterface::intersect2( char *s, ValVec<htmRange> &vec ) {

  char *str = s;
  VarStr key, qualifier, istr;
  int i = parseKey( str, key ), j = 0;
  if(i < 0)return 0;

  if(key == "DOMAIN")
    qualifier = key;
  else
    j = parseKey( str+i, qualifier );

  istr = qualifier;
  istr += " ";
  istr += depth_;
  istr += " ";
  istr += (str+i+j);

  if( key == "DOMAIN" )
    return domain(istr.data(), vec);
  if( key == "CONVEX" )
    return convexHull(istr.data(), vec);
  if( key == "CIRCLE" )
    return circleRegion( istr.data(), vec);
  error_ = "htmSqlInterface::intersect: Unrecognized keyword ";
  error_ += key;
  err_ = true;
  return 0;

}

int
htmSqlInterface::parseKey( char *str, VarStr &key ) {

  if( str == NULL ) {
    error_ = "htmSqlInterface::parseKey: No String to process";
    err_ = true;
    return -1;
  }

  size_t len = strlen(str);
  uint32 i = 0, j = 0;
  bool gotkey = false;

  // strip leading whitespace
  while( *str == ' ' || *str == '\n' || *str == '\t' || *str == '\r' ) {
    str++; j++;
  }

  for( i = 0; i < 10 && i < len; i++ ) {
    if(str[i] == ' '){
      gotkey = true;
      break;
    }
    key.at(i) = str[i];
  }
  if( !gotkey ) {
    error_ = "htmSqlInterface::parseKey: Could not parse keyword";
    err_ = true;
    return -1;
  }

  i++;
  return i+j;
}

/////////////////////CircleRegion/////////////////////

MsgStr
htmSqlInterface::circleRegionDiagnostic( char *str ) {

  result_ = nONE;
  try {
    err_ = false;
    resVec_ = &htm_.circleRegionCmd(str);
	result_ = cIRCLE;
	error_.clear();
  } catch (SpatialException &x) {
    error_ = x.what();
    err_ = true;
  }
  return error_;

}

size_t
htmSqlInterface::circleRegion( char *str, ValVec<htmRange> &vec ) {

  if(result_ == cIRCLE) {
	result_ = nONE;
	vec = *resVec_;
	return resVec_->length();
  }
  try {
	vec.cut(vec.length());
    err_ = false;
    vec = htm_.circleRegionCmd(str);
	return vec.length();
  } catch (SpatialException &x) {
    error_ = x.what();
    err_ = true;
  }
  return 0;

}

//////////////////ConvexHull///////////////////////
MsgStr
htmSqlInterface::convexHullDiagnostic( char *str ) {

  result_ = nONE;
  try {
    err_ = false;
    resVec_ = &htm_.convexHullCmd(str);
	result_ = cHULL;
	error_.clear();
  } catch (SpatialException &x) {
    error_ = x.what();
    err_ = true;
  }
  return error_;

}


size_t
htmSqlInterface::convexHull( char *str, ValVec<htmRange> &vec  ) {
  if(result_ == cHULL) {
	result_ = nONE;
	vec = *resVec_;
	return resVec_->length();
  }
  try {
    err_ = false;
    vec = htm_.convexHullCmd(str);
	return vec.length();
  } catch (SpatialException &x) {
    error_ = x.what();
    err_ = true;
  }
  return 0;

}



//////////////////////////domain/////////////////////////

MsgStr
htmSqlInterface::domainDiagnostic( char *str ) {
  result_ = nONE;
  try {
    err_ = false;
    resVec_ = &htm_.domainCmd(str);
	result_ = dOMAIN;
	error_.clear();
  } catch (SpatialException &x) {
    error_ = x.what();
    err_ = true;
  }
  return error_;

}

size_t
htmSqlInterface::domain( char *str, ValVec<htmRange> &vec  ) {
  if(result_ == dOMAIN) {
	result_ = nONE;
	vec = *resVec_;
	return resVec_->length();
  }
  try {
    err_ = false;
    vec = htm_.domainCmd(str);
	return vec.length();
  } catch (SpatialException &x) {
    error_ = x.what();
    err_ = true;
  }
  return 0;

}

