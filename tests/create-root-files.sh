#!/bin/sh
#
# Use this script for creating a pair of ROOT files for testing. The files
# will be created in the working directory with extension .root
#
root -l -b -q createTree.cxx
root -l -b -q createGaussHistogram.cxx
