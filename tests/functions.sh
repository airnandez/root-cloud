#!/bin/bash

#
# Set of functions used for testing cloud providers. This file is sourced by
# the main test script.
#

function printMessage {
   local message=$@
   echo ${message}
}

function printColor {
   local color=$1
   shift
   local message=$@
   echo -e "${color}" ${message} "\033[0m"
}

function printRed {
   local message=$@
   local red='\033[31m'
   printColor "${red}" ${message}
}

function printGreen {
   local message=$@
   green='\033[32m'
   printColor "${green}" ${message}
}

function printCyan {
   local message=$@
   cyan='\033[36m'
   printColor "${cyan}" ${message}
}


function printTestResult {
   local result=$1
   local fileName=$2
   if [ "${result}" == "OK" ]; then
      printGreen "OK" ${fileName}
   else
      printRed "ERROR" ${fileName}
   fi
}


function testFileSet {
   fileSet=$1

   # Test each file in the file set
   for f in ${fileSet[*]}; do
      printMessage "Testing with file '"$f"'"

      # Call the ROOT macro with this file
      root -l -b -q -x scanTree.cxx\(\"$f\"\)
      rootRetCode=$?

      # If there was an error, stop testing
      if [ "$rootRetCode" -ne "0" ]; then
         printTestResult "FAILED" "'"$f"'"
         printMessage "Stopping test."
         break
      else
         printTestResult "OK" "'"$f"'\n"
      fi
   done
}
