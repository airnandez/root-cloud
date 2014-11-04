#!/bin/bash

#
# Parse argument: we expect the identifier of a cloud provider to test against
#
allProviders=( "Amazon" "Google" "Rackspace" "Swift" )
if [ $# -eq 0 ]; then
   # No argument provided
   thisScript=`basename $0`
   echo "Usage:"
   echo
   echo "   $thisScript all"
   echo
   echo "   $thisScript <provider>"
   echo "        where <provider> may be one of: " ${allProviders[@]}
   exit 1
fi

#
# Initialize
#
provider="$(tr [a-z] [A-Z] <<< "$@")"  # Set in uppercase
fileSet=()
accessKey=""
secretKey=""
testDir="$(cd "$(dirname "${BASH_SOURCE[0]}" )" && pwd )"

#
# Make sure we have a customized file containing the URL of the files to test
# and the credentials to use with this provider.
#
providerSettings="settings.sh"
fullProviderSettings=${testDir}/${providerSettings}
if [ ! -f ${fullProviderSettings} ]; then
   echo "could not find file '"${providerSettings}"'"
   echo "please make a copy of file 'sample-"${providerSettings}"', name it '"${providerSettings}"' and customize it"
   exit 1
fi

#
# Set the provider set
#
if [ "$provider" == "ALL" ]; then
   #
   # Test all support providers
   #
   providerSet=${allProviders[@]}
else
   #
   # Test only the specified provider
   #
   providerSet=( "$provider" )
fi

#
# Load the library of functions we will use
#
source ${testDir}/functions.sh

#
# Test the requested providers
#
separator=":::::::::::::::::::::::::::::::::::"
for p in ${providerSet[*]}; do
   #
   # Set the file set and credentials to be used with this provider
   #
   source ${fullProviderSettings} $p

   #
   # Show a banner with this provider's name
   #
   pUpper="$(tr [a-z] [A-Z] <<< "$p")"
   printCyan "\n\n" ${separator} ${pUpper} ${separator} "\n"

   #
   # Do we have some files to test with?
   #
   if [ ${#fileSet[@]} -eq 0 ]; then
      printCyan "No files to scan: skipping testing against" $p
      continue
   fi

   #
   # Set the credentials (if any) for this provider in the environment
   #
   if [ "X$accessKey" != "X" ]; then
      export S3_ACCESS_KEY="$accessKey"
      export S3_SECRET_KEY="$secretKey"
   else
      export OS_AUTH_URL="$authUrl"
      export OS_TENANT_NAME="$tenantName"
      export OS_USERNAME="$userName"
      export OS_PASSWORD="$password"
   fi

   #
   # Test every file in this provider's file set
   #
   testFileSet ${fileSet}

   #
   # Unset the credentials
   #
   unset S3_ACCESS_KEY S3_SECRET_KEY
   unset OS_AUTH_URL OS_TENANT_NAME OS_USERNAME OS_PASSWORD
done

