//#     Filename:       SpatialInterface.hxx
//#
//#     Interfaceion interface inline methods
//#
//#
//#     Author:         Peter Z. Kunszt, based on A. Szalay's code
//#     
//#     Date:           October 15, 1998
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

///////////LOOKUP METHODS////////////////////

inline
uint64 htmInterface::lookupID(float64 ra, float64 dec) const {
  return index_->idByPoint(ra,dec);
}

inline
uint64 htmInterface::lookupID(float64 x, float64 y, float64 z) const {
  SpatialVector v(x,y,z);
  return index_->idByPoint(v);
}

inline
uint64 htmInterface::lookupID(char *nm) const {
  return index_->idByName(nm);
}

inline
const char * htmInterface::lookupName(float64 ra, float64 dec) {
  index_->nameByPoint(ra,dec,name_);
  return name_;
}

inline
const char * htmInterface::lookupName(float64 x, float64 y, float64 z) {
  SpatialVector v(x,y,z);
  index_->nameByPoint(v,name_);
  return name_;
}

inline
const char * htmInterface::lookupName(uint64 id) {
  index_->nameById(id,name_);
  return name_;
}

//////////OTHERS/////////////////////////////

inline
void htmInterface::changeDepth(size_t depth, size_t saveDepth) {
  if(index_->maxlevel_ != depth || index_->buildlevel_ != saveDepth) {
    delete index_;
    index_ = new SpatialIndex(depth, saveDepth);
  }
}


inline
const SpatialIndex & htmInterface::index() const {
  return *index_;
}
