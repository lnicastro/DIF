# ----------------------------------------------------------------------^
# Copyright (C) 2005 - 2018
# Giorgio Calderone <giorgio.calderone@inaf.it>, Luciano Nicastro <luciano.nicastro@inaf.it>
# 
# This file is part of DIF.
# 
#     DIF is free software; you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation; either version 2 of the License, or
#     (at your option) any later version.
# 
#     DIF is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
# 
#     You should have received a copy of the GNU General Public License
#     along with DIF; if not, write to the Free Software
#     Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
# 
# ----------------------------------------------------------------------$
AC_INIT([dif],
        [0.5.5],
        [Giorgio Calderone <giorgio.calderone@inaf.it>, Luciano Nicastro <luciano.nicastro@inaf.it>],
        [dif])

AC_CONFIG_AUX_DIR(config)
# AM_CONFIG_HEADER(src/config.h)
AC_CONFIG_HEADERS(src/config.h)
AM_INIT_AUTOMAKE([dist-bzip2])
m4_include([ax_compare_version.m4])

mypath_mysql_source=""

AC_ARG_WITH(mysql_source,
    [AC_HELP_STRING([--with-mysql-source=PATH],
    [PATH is the path to the MYSQL source directory (used to compile DB engines)])],
    [mypath_mysql_source="$withval"; ])

if test -z $mypath_mysql_source; then
  AC_MSG_ERROR([The path to MySQL source or build directory tree is needed to compile DIF.])
fi

MYSQL_SOURCE=$mypath_mysql_source
AC_SUBST(MYSQL_SOURCE)


AC_PROG_CC_STDC
#AX_CXX_COMPILE_STDCXX11
AC_LANG_PUSH(C++)
AC_PROG_CXX
AC_PROG_LIBTOOL

AC_PATH_PROG([PERL], [perl])
if [[ -z "$PERL" ]]; then AC_MSG_ERROR([Could not find program: perl]); fi

AC_PATH_PROG([MYSQL], [mysql])
if [[ -z "$MYSQL" ]]; then AC_MSG_ERROR([Could not find program: mysql]); fi

AC_PATH_PROG([MYSQL_CONFIG], [mysql_config])
if [[ -z "$MYSQL_CONFIG" ]]; then AC_MSG_ERROR([Could not find program: mysql_config]); fi

#Note the difference in error handling for AC_PATH_PROG and AC_CHECK_HEADERS!

MYSQL_VERSION=`$MYSQL_CONFIG --version | cut -d "." -f -2 | sed -e 's/\./_/g'`
AC_SUBST(MYSQL_VERSION)

#Standard version id (e.g. 50709 or 50626 or 100119)
MY_VERSION_ID=`$MYSQL_CONFIG --version | sed -e 's/\./0/' | cut -d "." -f -2 | sed -e 's/\./ /' | xargs printf "%02d"`
AC_SUBST(MY_VERSION_ID)


#Used to decide ha_dif source file
#AM_CONDITIONAL([MYSQL8], [test $MY_VERSION_ID -gt "80000"])
AM_CONDITIONAL([MYSQL8], [ [[ $MY_VERSION_ID -gt "80000" ]] && [[ $MY_VERSION_ID -lt "100000" ]] ])


#Installed MySQL include dir
MYSQL_INCLUDE=`$MYSQL_CONFIG --include`

#Installed MySQL library flags (see src/Makefile.am)
MYSQL_LIBS=`$MYSQL_CONFIG --libs`
AC_SUBST(MYSQL_LIBS)

#CPPFLAGS="$MYSQL_INCLUDE -I$mypath_mysql_source/sql -I$mypath_mysql_source/include -I$mypath_mysql_source/include/mysql -I$mypath_mysql_source $CPPFLAGS"
CPPFLAGS="$MYSQL_INCLUDE $CPPFLAGS"
CXXFLAGS="-std=c++11"


AX_COMPARE_VERSION([$MYSQL_VERSION],[gt],[5.2])
if test "$ax_compare_version" = "true" ; then
  echo "Warning: MySQL version $MYSQL_VERSION (greater than 5.1). Version ID $MY_VERSION_ID. When compiling mysql check debug mode is off, e.g use 'cmake -DWITH_DEBUG=0 ...'"
  echo "Warning: you must pass the BUILD dir. to configure, e.g. '--with-mysql-source=/path_to_mysql_source/bld'"
  if test $MY_VERSION_ID -lt "50709" ; then
  # if m4_cmp($MYSQL_VERSION_ID,50709) ; then
    AC_CHECK_HEADERS([sql_priv.h], [], [AC_MSG_ERROR([Could not find required include file: sql_priv.h])])
  fi

cmake_MySQL_SOURCE_DIR=`grep MySQL_SOURCE_DIR $mypath_mysql_source/CMakeCache.txt | cut -d "=" -f 2`

#CMake build directory is mandatory. Check if it coincides with source dir. 
  if test "x$mypath_mysql_source" != "x$cmake_MySQL_SOURCE_DIR" ; then
    CPPFLAGS="$CPPFLAGS -I$mypath_mysql_source/sql -I$mypath_mysql_source/include -I$mypath_mysql_source/include/mysql -I$mypath_mysql_source -I$mypath_mysql_source/libbinlogevents/export -I$mypath_mysql_source/libbinlogevents/include"
  fi

  CPPFLAGS="-I$mypath_mysql_source/zlib -I$cmake_MySQL_SOURCE_DIR/libbinlogevents/export -I$cmake_MySQL_SOURCE_DIR/libbinlogevents/include -I$cmake_MySQL_SOURCE_DIR/sql -I$cmake_MySQL_SOURCE_DIR/include/mysql -I$cmake_MySQL_SOURCE_DIR/include -I$cmake_MySQL_SOURCE_DIR/extra/rapidjson/include -I$cmake_MySQL_SOURCE_DIR $CPPFLAGS -D MY_VERSION_ID=$MY_VERSION_ID -DDBUG_OFF"
else
  AC_CHECK_HEADERS([mysql_priv.h], [], [AC_MSG_ERROR([Could not find required include file: mysql_priv.h])])
  CPPFLAGS="$CPPFLAGS -D MY_VERSION_ID=$MYSQL_VERSION_ID"
fi

DIF_VERSION="$PACKAGE_VERSION"

AC_CONFIG_FILES([
   Makefile
   version
   doc/Makefile
   contrib/Makefile
   src/Makefile
   scripts/Makefile
   sql/Makefile
   sql/Messier.sql
   sql/funcdesc.sql
   scripts/dif
])

AC_OUTPUT
