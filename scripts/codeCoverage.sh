#!/bin/bash
LAUNCH_DIR=`pwd`
COVERAGE_DIR=coverage
#  uncoment to enabled source blacklist: exmple --> SOURCE_BLACKLIST=".*ProbeTransmogrifier.cpp.*"
HEADER_WHITELIST=

#clean up coverage dir
rm -rf $COVERAGE_DIR
mkdir -p $COVERAGE_DIR

PROJECT="FileIO"
OBJECT_DIR="FileIO.dir"
#copy source cpp files and profile files to the coverage dir
cp ~/rpmbuild/BUILD/$PROJECT/CMakeFiles/$OBJECT_DIR/src/* $COVERAGE_DIR
#convert the whitelist to a filter
FORMATTED_HEADER_LIST=
for header in $HEADER_WHITELIST 
do
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
# Uncomment/replace with the gcovr line at the bottom to enabled source blacklist
#gcovr -v --filter="$FILTER" --exclude="$SOURCE_BLACKLIST" --gcov-executable /usr/local/probe/bin/gcov --exclude-unreachable-branches --html --html-details -o coverage.html

gcovr -v --filter="$FILTER"  --gcov-executable /usr/local/probe/bin/gcov --exclude-unreachable-branches --html --html-details -o coverage.html
cd $LAUNCH_DIR