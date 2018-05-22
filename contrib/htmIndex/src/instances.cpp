//#     Filename:       instances.cpp
//#
//#     The instances needed for the SpatialMap package.
//#
//#     Author:         Peter Z. Kunszt
//#     
//#     Date:           October 23, 1998
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
//#     Modification History:
//#
#include <VarVecDef.h>
#include <SpatialGeneral.h>
#include <SpatialInterface.h>
#include <SpatialConvex.h>

// In the SX environment this file is part of the sxGeneral library.
#ifndef SXDB
#include <VarStr.hpp>
#endif

#if defined(SpatialStandardTemplate)

// The sparc has a strange way of not explicitly defining the subclasses...
#if defined(SpatialSUN) && !defined(SpatialLinux)
template class ValVec<QuadNode>;
template class ValVec<Layer>;
#else
template class ValVec<SpatialIndex::QuadNode>;
template class ValVec<SpatialIndex::Layer>;
#endif

template class ValVec<BitList>;
template class ValVec<SpatialVector>;
template class ValVec<SpatialConstraint>;
template class ValVec<SpatialConvex>;
template class ValVec<int16>;
template class ValVec<int32>;
template class ValVec<uint8>;
#ifndef SXDB
template class ValVec<uint16>;
#endif
template class ValVec<uint32>;
template class ValVec<uint64>;
#ifdef SpatialDigitalUnix
template class ValVec<size_t>;
#endif
template class ValVec<htmRange>;
template class ValVec<htmPolyCorner>;

#elif defined(SpatialPragmaTemplateSGI)

#pragma instantiate ValVec<SpatialIndex::QuadNode>
#pragma instantiate ValVec<SpatialIndex::Layer>
#pragma instantiate ValVec<BitList>
#pragma instantiate ValVec<SpatialVector>
#pragma instantiate ValVec<SpatialConstraint>
#pragma instantiate ValVec<SpatialConvex>
#pragma instantiate ValVec<int16>
#pragma instantiate ValVec<int32>
#pragma instantiate ValVec<uint8>
#ifndef SXDB
#pragma instantiate ValVec<uint16>
#endif
#pragma instantiate ValVec<uint32>
#pragma instantiate ValVec<uint64>
#pragma instantiate ValVec<htmRange>
#pragma instantiate ValVec<htmPolyCorner>

#elif defined(SpatialWINTemplate)


#endif
