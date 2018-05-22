//#     Filename:       BitList.hxx
//#
//#     Friend declarations for bitlist
//#
//#
//#     Author:         Peter Z. Kunszt
//#     
//#     Date:           June 3, 1998
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

//#######################################################################
//
// Friend functions to BitList
//

// Bitwise operators returning the result in a separate BitList.
// First argument: return list, second and third arguments: bitlists
// to process. AND operator
BitList & AND (BitList &, const BitList &, const BitList &);

// OR operator
BitList & OR  (BitList &, const BitList &, const BitList &);

// XOR operator
BitList & XOR (BitList &, const BitList &, const BitList &);

// NOT operator
BitList & NOT (BitList &, const BitList &);

// </GROUP>
// 
