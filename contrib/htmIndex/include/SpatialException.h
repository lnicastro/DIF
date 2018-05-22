//# SpatialException.h
//#
//# Author:    Peter Z Kunszt based on John Doug Reynolds'code
//#
//# Creation:  March 1998
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
//# Modification history:
//#

// Modified: L. Nicastro @ IASF-INAF, 27/02/2009
//           method defaultstr modified



#ifndef _SpatialException_h
#define _SpatialException_h

#include "SpatialGeneral.h"

/** HTM SpatialIndex Exception base class
    This is the base class for all Science Archive exceptions.  It may
    be used as a generic exception, but programmers are encouraged to
    use the more specific derived classes.  Note that all Spatial
    exceptions are also Standard Library exceptions by
    inheritance.
*/

class LINKAGE SpatialException {
public:
  /** Default and explicit constructor.
      The default constructor
      supplies a generic message indicating the exception type.  The
      explicit constructor sets the message to a copy of the provided
      string.  This behavior is shared by all derived classes.
  */

  SpatialException( const char *what = 0, int defIndex = 1 ) throw();

  /** Standard constructor.
      The message is assembled from copies of
      the two component strings.  The first indicates where in the
      program the exception was thrown, and the second indicates why.
      The null pointer is used to select standard components according
      to the type of the exception.  This behavior is shared by all
      derived classes.
  */
   SpatialException( const char *context, const char *because,
		int defIndex = 1) throw();

  /// Copy constructor.
   SpatialException( const SpatialException& ) throw();

  /// Assignment operator.
   SpatialException& operator=( const SpatialException& ) throw();

  /// Destructor.
   virtual ~SpatialException() throw();

  /// Returns the message as set during construction.
   virtual const char *what() const throw();

  /// return string length also for null strings
   int slen(const char *) const;

  /// deallocate string
   void clear();

  /// default error string
//   static char *defaultstr[];
   const char *defaultstr(int i);

protected:
  /// error string to assemble
   char * str_;
};

/** SpatialException thrown by unimplemented functions.
    This Exception should be thrown wherever
    important functionality has been left temporarily unimplemented.
    Typically this exception will apply to an entire function.
*/

class LINKAGE SpatialUnimplemented : public SpatialException {
public:
  /// Default and explicit constructors.
   SpatialUnimplemented( const char *what = 0 ) throw();

  /// Standard constructor.
   SpatialUnimplemented( const char *context, const char *because ) throw();

  /// Copy constructor.
   SpatialUnimplemented( const SpatialUnimplemented& ) throw();
};

/** SpatialException thrown on operational failure.
    This Exception should be thrown when an operation
    fails unexpectedly.  A special constructor is provided for
    assembling the message from the typical components: program
    context, operation name, resource name, and explanation.  As usual,
    any component may be left out by specifying the null pointer.
*/

class LINKAGE SpatialFailure : public SpatialException {
public:
   /// Default and explicit constructors.
   SpatialFailure( const char *what = 0 ) throw();

  /// Standard constructor.
   SpatialFailure( const char *context, const char *because ) throw();

  /// Special constructor.
   SpatialFailure( const char *context, const char *operation
	      , const char *resource, const char *because = 0 ) throw();

  /// Copy constructor.
   SpatialFailure( const SpatialFailure& ) throw();
};

/** SpatialException thrown on violation of array bounds.
    This Exception should be thrown on detection of an
    attempt to access elements beyond the boundaries of an array.  A
    special constructor is provided for assembling the message from the
    typical components: program context, array name, violated boundary,
    and violating index.
*/

class LINKAGE SpatialBoundsError : public SpatialException {
public:
  /// Default and explicit constructors.
   SpatialBoundsError( const char *what = 0 ) throw();

  /** Standard constructor.
      If limit and index are -1, both are
      considered unknown.  Note that the upper limit of a zero-offset
      array is not the same as the number of elements.
  */
   SpatialBoundsError( const char *context
		  , const char *array, int32 limit =-1, int32 index=-1 ) throw();

   /// Copy constructor.
   SpatialBoundsError( const SpatialBoundsError& ) throw();
};

/** SpatialException thrown on violation of interface protocols.
    This Exception should be thrown when a program,
    class, or function interface requirement is breached.
    Specifically, this includes improper usage and invalid arguments.
    For the latter, a special constructor is provided for assembling
    the message from the typical components: program context, argument
    name, and explanation.
*/

class LINKAGE SpatialInterfaceError : public SpatialException {
public:
  /// Default and explicit constructors.
   SpatialInterfaceError( const char *what = 0 ) throw();

  /// Standard constructor.
   SpatialInterfaceError( const char *context, const char *because ) throw();

  /// Special constructor.
   SpatialInterfaceError( const char *context
		     , const char *argument, const char *because ) throw();

  /// Copy constructor.
   SpatialInterfaceError( const SpatialInterfaceError& ) throw();
};

#endif /* _SpatialException_h */
