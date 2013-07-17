#!/bin/sh
#
# Use this script for setting the HTTP proxy before you run the test suite.
# If set, ROOT will proxy its HTTP requests to the cloud providers though this
# server.
#
export http_proxy="localhost:3213"
