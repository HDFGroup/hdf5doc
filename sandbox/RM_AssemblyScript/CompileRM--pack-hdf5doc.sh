#!/bin/sh

# script for compiling HDF5 Reference Manual, as downloaded from Debian bug repository
#     http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=610866
# 
# Debian Bug report logs - #610866
# libhdf5-doc: reference manual does not contain the actual function descriptions
#
# Downloaded:          7 Feb 2011
# Bug report dated:   23 Jan 2011
#
# requires svn and php

BRANCH_VERSION=$(dpkg-parsechangelog | sed -rne 's,^Version: (\w)\.(\w)\.(\w)(.+),\1_\2_\3,p')

svn export https://svn.hdfgroup.uiuc.edu/hdf5doc/branches/hdf5_$BRANCH_VERSION/html

find html -name 'Makefile.*' -delete
rm -rf html/PSandPDF html/Specifications
rm -f html/HL/Examples/* html/Specs.html
#rm -f html/RM/Tools/h5check.htm html/RM/Tools/h5fix_swapped_ids.htm html/RM/CollectiveCalls.html

cd html/RM
for f in *.html
do
	echo "prcessing $f...";\
	php -f $f > $f.new;\
	rm $f;\
	mv $f.new $f;\
done
cd -
#rm -rf html/RM/Tools html/RM/H5 html/RM/H5?

tar czf - html | uuencode - > html.tgz.uu
rm -rf html

