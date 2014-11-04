#!/bin/bash

#
# This is a sample file you need to customize with the file URLs and credentials
# specific for each storage provider you want to test.
#

#
# Parse argument: we expect the identifier of a cloud provider
#
if [ $# -eq 0 ]; then
   return
fi

targetProvider="$(tr [a-z] [A-Z] <<< "$1")"  # Set in uppercase

#
# Set the credentials and the set of files to test against for the specified
# cloud storage provider.
# If you don't want to test a specific provider, leave commented out all the file
# sample file URLs, i.e. leave the array variable 'fileSet' empty.
#
case $targetProvider in

   "AMAZON")
         fileSet=(
            #      "s3://s3.amazonaws.com/mybucket/tree.root"
            # "s3https://s3.amazonaws.com/mybucket/tree.root"
            #  "s3http://s3.amazonaws.com/mybucket/tree.root"
            #    "http://s3.amazonaws.com/mybucket/world-readable-tree.root"
            #   "https://s3.amazonaws.com/mybucket/world-readable-tree.root"
         )
         accessKey="my Amazon access key"
         secretKey="my Amazon secret key"
         ;;


   "GOOGLE")
         fileSet=(
            #      "gs://storage.googleapis.com/mybucket/tree.root"
            # "gshttps://storage.googleapis.com/mybucket/tree.root"
            #  "gshttp://storage.googleapis.com/mybucket/tree.root"
            #    "http://storage.googleapis.com/mybucket/world-readable-tree.root"
            #   "https://storage.googleapis.com/mybucket/world-readable-tree.root"
         )
         accessKey="my Google access key"
         secretKey="my Google secret key"
         ;;


   "SWIFT")
         fileSet=(
            #   "swift://root/tree.root"
         )
         authUrl="https://identity.example.com:5000/v2.0/"
         tenantName="my Swift tenant"
         userName="my Swift user"
         password="my Swift password"
         ;;


   "RACKSPACE")
         fileSet=(
            # "swift://root/tree.root"
         )
         authUrl="https://identity.api.rackspacecloud.com/v2.0/"
         tenantName="my Rackspace tenant"
         userName="my Rackspace user"
         password="my Rackspace password"
         ;;


   *)
         #
         # Unknown provider
         #
         fileSet=()
         accessKey=""
         secretKey=""
esac

