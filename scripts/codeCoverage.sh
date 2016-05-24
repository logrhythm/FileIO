#!/bin/bash

# --- WHITELIST ---
#
# Add a header to the code coverage here
#    Examples:
#    HEADER_WHITELIST="myfile.h"
#    HEADER_WHITELIST="myfile.h myotherfile.h"
#
HEADER_WHITELIST=

# --- BLACKLIST ---
#
# Blacklisting of files can also be done
#    Please see commented out execution of gcovr at the bottom.
#    Example: 
#       SOURCE_BLACKLIST=".*ProbeTransmogrifier.cpp.*"
#
SOURCE_BLACKLIST=

function formatForFilter ()
{
   FORMATTED_HEADER_LIST=
   for header in $1
   do
      if [ -z $FORMATTED_HEADER_LIST ] ; then
         FORMATTED_HEADER_LIST=".*$header\$"
      else
         FORMATTED_HEADER_LIST="$FORMATTED_HEADER_LIST|.*$header\$"
      fi
   done
   echo "$FORMATTED_HEADER_LIST"
}

function createFilterForGcovr ()
{
   DEFAULT_LIST="$1"
   FORMATTED_USER_LIST="$2"
   FILTER=
   if [ -z $FORMATTED_USER_LIST ] ; then
      FILTER="$DEFAULT_LIST"
   else
      FILTER="$DEFAULT_LIST|$FORMATTED_USER_LIST"
   fi
   echo "$FILTER"
}

function getProjectNameFromCMake ()
{
   filename="$1"
   filenameRegex="SET\(LIBRARY_TO_BUILD (.*)\)$"
   while IFS='' read -r line || [[ -n "$line" ]]; do
      if [[ $line =~ $filenameRegex ]] ; then
         echo "${BASH_REMATCH[1]}"
         break
      fi
   done < $filename
}

function cleanAndRebuildDir ()
{
   DIRECTORY="$1"
   rm -rf $DIRECTORY
   mkdir -p $DIRECTORY
}

LAUNCH_DIR=`pwd`
CMAKELISTSFILE=$LAUNCH_DIR/CMakeLists.txt

PROJECT=$(getProjectNameFromCMake "$CMAKELISTSFILE")
OBJECT_DIR="${PROJECT}.dir"

DEFAULT_WHITELIST=".*cpp$"
DEFAULT_BLACKLIST="/usr/local/probe/.*"

echo "The Project name is : $PROJECT"

# --- DIRECTORY SETUP ---
#
# Get the gtest library
#
cd 3rdparty
unzip -u gtest-1.7.0.zip
cd ..

# Clean up and create coverage dir
COVERAGE_DIR=coverage
cleanAndRebuildDir "$COVERAGE_DIR"

# Clean up and create local build dir
BUILD_DIR=build
cleanAndRebuildDir "$BUILD_DIR"
cd build

# --- VERSIONING ---
# A dummy version to please cmake
# As version number we use the commit number on HEAD 
# we do not bother with other branches for now
GIT_VERSION=`git rev-list --branches HEAD | wc -l`
VERSION="1.$GIT_VERSION"

# --- BUILD ---
PATH=/usr/local/probe/bin:$PATH
/usr/local/probe/bin/cmake -DUSE_LR_DEBUG=ON -DVERSION=$VERSION -DCMAKE_CXX_COMPILER_ARG1:STRING=' -Wall -Werror -g -gdwarf-2 -fno-elide-constructors -fprofile-arcs -ftest-coverage -O0 -fPIC -m64 -Wl,-rpath -Wl,. -Wl,-rpath -Wl,/usr/local/probe/lib -Wl,-rpath -Wl,/usr/local/probe/lib64 -fno-inline -fno-inline-small-functions -fno-default-inline' -DCMAKE_CXX_COMPILER=/usr/local/probe/bin/g++ ..

make -j

# --- RUN THE UNIT TESTS ---
#
cp $LAUNCH_DIR/scripts/unitTestRunner.sh .
sh unitTestRunner.sh
cd ..

# --- WHITELIST FORMATTING ---
#
# Convert the Whitelist into a filter
#
cp build/CMakeFiles/$OBJECT_DIR/src/* $COVERAGE_DIR
FORMATTED_HEADER_WHITELIST=$(formatForFilter "$HEADER_WHITELIST")
echo "FORMATTED_HEADER_WHITELIST is : $FORMATTED_SOURCE_BLACKLIST"
FILTER=$(createFilterForGcovr "$DEFAULT_WHITELIST" "$FORMATTED_HEADER_WHITELIST")
echo "FILTER pattern is : $FILTER"

# --- BLACKLIST FORMATTING ---
#
# Convert the Blacklist into a filter
#
FORMATTED_SOURCE_BLACKLIST=$(formatForFilter "$SOURCE_BLACKLIST")
echo "FORMATTED_SOURCE_BLACKLIST is : $FORMATTED_SOURCE_BLACKLIST"
EXCLUDE=$(createFilterForGcovr "$DEFAULT_BLACKLIST" "$FORMATTED_SOURCE_BLACKLIST")
echo "EXCLUDE pattern is : $EXCLUDE"

cd $COVERAGE_DIR

PATH=/usr/local/probe/bin:$PATH

# --- GCOVR ---
#
# Includes BOTH the whitelist and the blacklist
#
gcovr  --verbose --filter="$FILTER" --exclude="$EXCLUDE" --sort-percentage --gcov-executable /usr/local/probe/bin/gcov --exclude-unreachable-branches --html --html-details -o coverage_${PROJECT}.html

cd $LAUNCH_DIR