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
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_HEADERS([src/binlog_config.h])
AM_INIT_AUTOMAKE([dist-bzip2])
m4_include([ax_compare_version.m4])
m4_include([ax_cxx_compile_stdcxx.m4])

mypath_mysql_source=""

AC_ARG_WITH(mysql_source,
    [AC_HELP_STRING([--with-mysql-source=PATH],
    [PATH is the path to the MYSQL source directory (used to compile DB engines)])],
    [mypath_mysql_source="$withval"; ])

#if test -z $mypath_mysql_source; then
#  AC_MSG_ERROR([The path to MySQL source or build directory tree is needed to compile DIF.])
#fi

MYSQL_SOURCE=$mypath_mysql_source
AC_SUBST(MYSQL_SOURCE)


AC_PROG_CC_STDC
AC_LANG_PUSH(C++)
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX(14, ext, optional)
AC_PROG_LIBTOOL

AC_PATH_PROG([PERL], [perl])
if [[ -z "$PERL" ]]; then AC_MSG_ERROR([Could not find program: perl]); fi

AC_PATH_PROG([MYSQL], [mysql])
if [[ -z "$MYSQL" ]]; then AC_MSG_ERROR([Could not find program: mysql]); fi

AC_PATH_PROG([MYSQL_CONFIG], [mysql_config])
# If no mysql_config try with mariadb_config
if [[ -z "$MYSQL_CONFIG" ]]; then
  echo "Warning: mysql_config not found. Trying with mariadb_config ..."
  AC_PATH_PROG([MYSQL_CONFIG], [mariadb_config])
  if [[ -z "$MYSQL_CONFIG" ]]; then AC_MSG_ERROR([Could not find program: mariadb_config]); fi
fi

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

CPPFLAGS="-D MY_VERSION_ID=$MY_VERSION_ID $CPPFLAGS"

# NOT MariaDB
if test $MY_VERSION_ID -lt "100000" ; then
  CPPFLAGS="$CPPFLAGS $MYSQL_INCLUDE"
  #CXXFLAGS="-std=c++11"
  if test $MY_VERSION_ID -gt "80026" ; then
	CXXFLAGS="-std=c++1z"
  fi
else

# Exclude unnecessary libssl and libcrypto that could cause linking errors
  MYSQL_LIBS=`echo $MYSQL_LIBS | sed -e "s/-lssl//g;s/-lcrypto//g"`

# MariaDB 10.3.12 distributes libmaridbd.so instead of libmariadb.so
  libmaria=`echo $MYSQL_LIBS | perl selectflags.pl | sed 's/-L//; s/ *//g'`libmariadbd.so

  if [[ -e "$libmaria" ]]; then
        MYSQL_LIBS=`echo $MYSQL_LIBS | sed -e 's/mariadb/mariadbd/'`
  fi
fi

AC_SUBST(MYSQL_LIBS)

AX_COMPARE_VERSION([$MYSQL_VERSION],[gt],[5.2])
if test "$ax_compare_version" = "true" ; then

  CPPFLAGS="-DDBUG_OFF -DNDEBUG $CPPFLAGS"

if test "x$mypath_mysql_source" != "x" ; then
  echo "Warning: MySQL version $MYSQL_VERSION (greater than 5.1). Version ID $MY_VERSION_ID. When compiling mysql check debug mode is off, e.g use 'cmake -DWITH_DEBUG=0 ...'"
  echo "Warning: you must pass the BUILD dir. to configure, e.g. '--with-mysql-source=/path_to_mysql_source/bld'"

  if [[ -e "$mypath_mysql_source/CMakeCache.txt" ]]; then
    cmake_MySQL_SOURCE_DIR=`grep MySQL_SOURCE_DIR $mypath_mysql_source/CMakeCache.txt | cut -d "=" -f 2`
  else
    cmake_MySQL_SOURCE_DIR="$mypath_mysql_source"
    echo "Warning: $mypath_mysql_source/CMakeCache.txt not found. Will use $mypath_mysql_source only"
  fi

#CMake build directory is mandatory. Check if it coincides with source dir. 
  if test "x$mypath_mysql_source" != "x$cmake_MySQL_SOURCE_DIR" ; then

# MariaDB
    if test $MY_VERSION_ID -gt "100000" ; then
      CPPFLAGS="$CPPFLAGS -I$mypath_mysql_source/include -I$mypath_mysql_source/libmariadb/include"
    else
      CPPFLAGS="$CPPFLAGS -I$mypath_mysql_source/sql -I$mypath_mysql_source/include -I$mypath_mysql_source/include/mysql -I$mypath_mysql_source -I$mypath_mysql_source/libbinlogevents/export -I$mypath_mysql_source/libbinlogevents/include"
    fi

  fi


# MariaDB
  if test $MY_VERSION_ID -gt "100000" ; then
    CPPFLAGS="$CPPFLAGS -I$cmake_MySQL_SOURCE_DIR/sql -I$cmake_MySQL_SOURCE_DIR/libmariadb/include -I$cmake_MySQL_SOURCE_DIR/include -I$cmake_MySQL_SOURCE_DIR/extra/yassl/include"
  else
    CPPFLAGS="-I$mypath_mysql_source/zlib -I$cmake_MySQL_SOURCE_DIR/libbinlogevents/export -I$cmake_MySQL_SOURCE_DIR/libbinlogevents/include -I$cmake_MySQL_SOURCE_DIR/sql -I$cmake_MySQL_SOURCE_DIR/include/mysql -I$cmake_MySQL_SOURCE_DIR/include -I$cmake_MySQL_SOURCE_DIR/extra/rapidjson/include -I$cmake_MySQL_SOURCE_DIR $CPPFLAGS"
  fi

fi

  if test $MY_VERSION_ID -lt "50709" ; then
  # if m4_cmp($MYSQL_VERSION_ID,50709) ; then
    AC_CHECK_HEADERS([sql_priv.h], [], [AC_MSG_ERROR([Could not find required include file: sql_priv.h])])

# MySQL 5.6
    if test $MY_VERSION_ID -ge "50600" ; then
      CPPFLAGS="-DDISABLE_DTRACE $CPPFLAGS"
    fi
  fi

else

  AC_CHECK_HEADERS([mysql_priv.h], [], [AC_MSG_ERROR([Could not find required include file: mysql_priv.h])])

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

# Touch aclocal managed files - ignore returned status
AC_MSG_NOTICE([])
AC_MSG_NOTICE([...touching 'configure aclocal.m4 Makefile.in src/config.h.in'])
  dummy=`touch configure aclocal.m4 Makefile.in src/config.h.in`
AC_MSG_NOTICE([])

AC_OUTPUT
