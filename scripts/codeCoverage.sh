#!/bin/bash
LAUNCH_DIR=`pwd`
PROJECT="FileIO"
OBJECT_DIR="${PROJECT}.dir"

# --- WHITELIST ---
# Add a header to the code coverage here
#    Examples:
#    HEADER_WHITELIST="myfile.h"
#    HEADER_WHITELIST="myfile.h myotherfile.h"
HEADER_WHITELIST=

# --- BLACKLIST ---
# Blacklisting of files can also be done
#    Please see commented out execution of gcovr at the bottom.
#    Example: 
#       SOURCE_BLACKLIST=".*ProbeTransmogrifier.cpp.*"
SOURCE_BLACKLIST="/usr/local/probe/*"

# --- DIRECTORY SETUP ---
# Get the gtest library
cd 3rdparty
unzip -u gtest-1.7.0.zip
cd ..

# Clean up and create coverage dir
COVERAGE_DIR=coverage
rm -rf $COVERAGE_DIR
mkdir -p $COVERAGE_DIR

# Clean up and create local build dir
rm -rf build
mkdir -p build
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
# The FileIO function "ReadAsciiFileContentAsRoot" modifies the permissions of the
#   process running it. For this reason, code coverage generation fails. It was 
#   previously thought that running tests as root was the culprit; that is not true.
#   Running tests as root is fine, but this particular test needs to have its .gcda
#   file generated first, then filled back in appropriately.
#
sudo ./UnitTestRunner --gtest_filter=-*TestReadAsciiFileContentAsRoot
./UnitTestRunner --gtest_filter=*TestReadAsciiFileContentAsRoot
cd ..

# --- WHITELIST FORMATTING ---
#
# Convert the Whitelist into a filter
#
cp build/CMakeFiles/$OBJECT_DIR/src/* $COVERAGE_DIR
FORMATTED_HEADER_LIST=
FILTER=
for header in $HEADER_WHITELIST 
do
   echo "Header file in Whitelist: " $header
    if [ -z $FORMATTED_HEADER_LIST ] ; then
        FORMATTED_HEADER_LIST=".*$header\$"
    else
        FORMATTED_HEADER_LIST="$FORMATTED_HEADER_LIST|.*$header\$"
    fi
done
cd $COVERAGE_DIR
if [ -z $FORMATTED_HEADER_LIST ] ; then
    FILTER=".*cpp$"
else
    FILTER=".*cpp$|$FORMATTED_HEADER_LIST"
fi

PATH=/usr/local/probe/bin:$PATH

# --- BLACKLIST GCOVR ---
# Uncomment the following GCOVR line to enable the BLACKLIST
# gcovr -v --exclude="$SOURCE_BLACKLIST" --gcov-executable /usr/local/probe/bin/gcov --exclude-unreachable-branches --html --html-details -o coverage.html

# --- WHITELIST GCOVR ---
gcovr --verbose --filter="$FILTER" --exclude="$SOURCE_BLACKLIST" --sort-percentage --gcov-executable /usr/local/probe/bin/gcov --exclude-unreachable-branches --html --html-details -o coverage_${PROJECT}.html

cd $LAUNCH_DIR