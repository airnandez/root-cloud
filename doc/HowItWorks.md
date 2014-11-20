## How this extension works

This extension is composed of a set of C++ classes and a ROOT plugin for the [TFile](http://root.cern.ch/root/html/TFile.html) class.

The compilation and link process of the C++ classes composing this extension produces a shared object library. This library is dynamically loaded by ROOT at runtime when it needs to open a file which name matches one of the schemes handled by this extension (e.g `s3://` or `swift://`). The plugin file tells the ROOT runtime what schemes are handled by this extension and what dynamic libraries it needs to load read the specified remote file.

See **How to Use** section provide details on what file naming schemes are supported by this extension.

### Supported versions of ROOT

We have tested this extension with several versions of ROOT since v5.24/00 (released on October 2009) up to version 5.34/23. ROOT versions prior to v5.24.00 may work as well but we have not tested. This extension also works with ROOT v6 master.

### Supported cloud storage service providers
The service providers we tested this extension against are:

* Amazon S3
* Google Storage
* Rackspace
* Self-hosted OpenStack Swift
* Plain HTTP(S) server

### Supported operating systems
We have tested this extension with the operating systems below:

* MacOS X v10.7, v10.8, v10.9
* Scientific Linux v5 and v6
* Ubuntu Linux v14.04

It is very likely that this extension also works on other operating systems where ROOT itself and the dependencies of this package (see below) are supported.
