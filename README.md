# Cloud storage extension for ROOT data analysis framework

## Overview
This software is an extension to CERN's [ROOT](http://root.cern.ch) data analysis framework. Its purpose is to help ROOT users transparently read files stored remotely (i.e. "in the cloud") through cloud storage protocols such as Amazon's S3 or OpenStack's Swift.


## Motivation
Since version 5.34.05 (released on February 2013) ROOT has improved its built-in support for reading files hosted by Amazon S3, Google Storage or any other cloud storage service provider exposing [Amazon's Simple Storage Service (S3) REST API](http://aws.amazon.com/s3/). This functionality has been successfully tested against Amazon S3, Google Storage, OpenStack Swift (through the S3 gateway) and S3 appliances such as Huawei's UDS. *(Disclosure: the author of this extension contributed to improve ROOT's built-in support for cloud storage)*.

However, for several reasons, some ROOT users (be them individual scientists or scientific collaborations) cannot upgrade to the latest version of ROOT to benefit from this feature. This project intends to help removing this constraint by providing an extension so that those users have the the possibility to exploit cloud storage with their favorite version of ROOT. If you are one of those users, we hope this software may help you.

Additional context and background information for this work can be found in [this presentation](https://speakerdeck.com/airnandez/exploring-cloud-storage-for-bes-iii-data).

## Benefits
As a ROOT user, the main benefits of using this extension are:

* you can use cloud storage services for storing your data and transparently use ROOT for accessing them,
* you don't need to install the latest version of ROOT in order to use cloud storage: this extension allows you to continue using legacy ROOT version and exploit your data stored in the cloud,
* you don't need to patch your current production version of ROOT for using this extension: this package can be installed on top of a broad range of ROOT versions,
* you don't need to modify your ROOT macros to use this extension: the names of your remote files just need to be prefixed by a scheme (such as `s3://`) so that ROOT can identify them and use this extension for retrieving their contents,
* this extension is forward compatible: if you upgrade to a recent version of ROOT which has built-in support for cloud storage you won't have to change your file naming scheme,
* the memory footprint of your ROOT-based software is not significantly increased: the binary version of this package is less that 500 KBytes

## How this extension works
This extension is composed of a set of C++ classes and a ROOT plugin for the [TFile](http://root.cern.ch/root/html/TFile.html) class.

When compiled and linked, the C++ classes produce a shared object library. This object library is dynamically loaded by ROOT when at runtime it requires to open a file which name matches one of the schemes handled by this extension (e.g `s3://`). The plugin file tells ROOT what schemes are handled by this extension and what classes need to be used to read the specified remote file.

See **How to Use** section below for more details on what file naming schemes are supported by this extension.

#### Supported versions of ROOT
We have tested this extension with several versions of ROOT since v5.24.00 (released on October 2009) up to the current production version (v5.34.18). ROOT versions prior to v5.24.00 may work but we have not tested.

#### Supported cloud storage service providers
The service providers we tested this extension against are:

* Amazon S3
* Google Storage
* Rackspace
* Self-hosted OpenStack Swift
* Plain HTTP(S) server

#### Supported operating systems
We have tested this extension with the operating systems below:

* MacOS X v10.7 and v10.8
* Scientific Linux v5 and v6

It is very likely that it works on other operating systems where ROOT and the dependencies of this package (see below) are supported.

## How to install
Before installing this extension, you need a working installation of ROOT. Please refer to the official documentation for detailed instructions on [how to install ROOT](http://root.cern.ch/drupal/content/downloading-root) on your preferred platform.

For developing and testing this extension we prefer to install ROOT from sources, so we recommend installing this way. However, if you have installed a binary version you may be able to install this extension if your execution environment matches the one used for building the binary version. You need your target computer to have at least a compatible version of the C++ compiler used for building the binary distribution of ROOT.

#### Dependencies
For handling the low level details of the HTTP protocol, this software uses [libNeon](http://www.webdav.org/neon/). To download and install libNeon please do:

```
$ cd /tmp
$ curl http://www.webdav.org/neon/neon-0.29.6.tar.gz
$ tar zxvf neon-0.29.6.tar.gz
$ cd neon-0.29.6
$ ./configure --with-ssl
$ make install
```

For building libNeon with support for HTTPS which we highly recommend (and is required by some cloud storage protocols anyway), you need to have OpenSSL installed. It is very likely that you already have it on your system.

As a result of the `make install` command, libNeon components (library, include files, etc.) will be installed under `/usr/local`. If you want to install them in another directory, say `$HOME/libNeon`, use the command below when configuring libNeon:

    $ ./configure --with-ssl --prefix=$HOME/libNeon

#### Installation instructions
Once ROOT and libNeon are installed in your system, you can download and install this extension by following the steps below.

1. Go to the directory where ROOT is installed in your system (say `$HOME/ROOT`) and activate that version of ROOT. As a consequence, some environmental variables will be modified (such as `PATH` and `LD_LIBRARY_PATH`) and in particular `$ROOTSYS` will be point to the location of your ROOT installation:
    
    ```
    $ source $HOME/ROOT/bin/thisroot.sh    # or source $HOME/ROOT/bin/thisroot.csh if you are using tcsh
    ```
    
3. Download the sources of this extension by cloning this Git repository into your local directory `$HOME/root-cloud`:

    ```
    $ git clone git://github.com/airnandez/root-cloud $HOME/root-cloud
    ```

4. Compile this extension using version of ROOT pointed to by `$ROOTSYS`:

    ```
    $ cd $HOME/root-cloud/src
    $ make install
    ```
    
**NOTE:** the provided `Makefile` assumes that the executable `neon-config` (installed by libNeon) is in your path. If it is not your case, do instead:

    $ NEON_CONFIG=/path/to/your/neon-config   make

`make install` installs the relevant components of this extension under `$ROOTSYS` as follows:
 
* the shared object library is copied to `$ROOTSYS/lib/libRootCloudStorage.so`
* the `TFile` plugin is copied to `$ROOTSYS/etc/plugins/TFile/P160_CloudStorage.C` 

## How to use
For opening a cloud file from within a ROOT macro, you have to specify two pieces of information:

1. the location of your file in the form of a URL,
2. the authentication information provided to you by your storage provider.

The **URL of your file** contains the name of the host ROOT has to connect to retrieve the contents of your file, the name of the container where your file is located (a.k.a. bucket) and the full path (a.k.a. object key) of your file. The name of the host depends on the service provider you use. Typical URLs for several providers are presented in the table below:

| Provider          | Example file URL |
| ----------------- |  ------------ |
| Amazon S3         |  `s3://s3.amazonaws.com/myBucket/path/to/my/file.root` |
| Google Storage    |  `gs://storage.googleapis.com/myBucket/path/to/my/file.root` |
| Rackspace         |  `swift://identity.api.rackspacecloud.com/myBucket/path/to/my/file.root` |
| Self-hosted OpenStack Swift   |  `swift://server.mylab.org/myBucket/path/to/my/file.root` |
| HTTP(S) server    |  `http://www.mylab.org/path/to/my/file.root` |

The **authentication information** is composed of an *access key* and a *secret key*. This key pair is used by this ROOT extension for sending information to the storage server so it can authenticate the requests. Please note that your secret key is never sent to the storage server, it is used for signing the requests ROOT sends to the server.

If all your remote files are hosted by a single provider, the easiest way is to set the authentication information in your environment by using a pair of environmental variables, for example:

```
$ export S3_ACCESS_KEY="my access key"
$ export S3_SECRET_KEY="my very long, very secret and impossible to remember secret key"
```

and then, in your C++ ROOT macro you can do:

```cpp
TFile* f = TFile::Open("s3://s3.amazonaws.com/myBucket/path/to/my/file.root");
if (f != 0) {
   // This ROOT file is successfully open: inspect its contents
   f->ls();
}
```

However, if from the same ROOT macro you want to read files hosted by several providers, you can specify the  authentication information for each file as a second argument to the `TFile::Open` method. For doing this, use the syntax `"AUTH=<access key>:<secret key>"` (note the `':'` separating the access and secret keys). For instance:


```cpp
// Open one file hosted by Amazon S3 (need to provide my credentials for Amazon AWS)
TFile* f1 = TFile::Open("s3://s3.amazonaws.com/myBucket/myFile.root", "AUTH=AWSACCESSKEY:1234567890WECUBIPO");

// Open another file hosted by Google Storage (need to provide my credentials for Google Storage)
TFile* f2 = TFile::Open("gs://storage.googleapis.com/myBucket/myFile.root", "AUTH=GOOGLEACCESSKEY:MWE56GBJDZBSIL8FV");
```

**WARNING:** *although we present it that way for illustration purposes, it is not recommended practice to store your authentication credentials in your source code. In particular, your secret key must be kept secret. One way of providing your authentication credentials to ROOT is storing them in a protected file and reading them by the ROOT macro at run time for computing the `AUTH=` argument just before you need to open the file.*

Based on the scheme of your file's URL, ROOT will automatically load and use the appropriate code of this extension for interacting with the server where your file is hosted for retrieving its contents.

The underlying protocol used to actually transport the file contents (i.e. HTTP or HTTPS) depends on the scheme used in your URL. The table below summarizes the possibilities currently supported:


| If you use this scheme…   | with this storage provider…       | this extension will use this protocol… | on top of… |
| ----------------------    | :-------------------------------: | ------------------------ | ------------ |
| `s3://`                   | Amazon S3                         | S3                       | HTTPS |
| `s3https://`              | Amazon S3                         | S3                       | HTTPS |
| `s3http://`               | Amazon S3                         | S3                       | HTTP  |
| `https://`                | Amazon S3                         |                          | HTTPS |
| `http://`                 | Amazon S3                         |                          | HTTP  |
|                           |                                   |                          |       |
| `gs://`                   | Google Storage                    | S3                       | HTTPS |
| `gshttps://`              | Google Storage                    | S3                       | HTTPS |
| `gshttp://`               | Google Storage                    | S3                       | HTTP  |
| `https://`                | Google Storage                    |                          | HTTPS |
| `http://`                 | Google Storage                    |                          | HTTP  |
|                           |                                   |                          |       |
| `swift://`                | Rackspace                         | Swift native protocol    | HTTPS |
| `swhttps://`              | Rackspace                         | *Not supported*          |       |
| `swhttp://`               | Rackspace                         | *Not supported*          |       |
| `s3://`                   | Rackspace                         | *Not supported*          |       |
| `s3https://`              | Rackspace                         | *Not supported*          |       |
| `s3http://`               | Rackspace                         | *Not supported*          |       |
|                           |                                   |                          |       |
| `swift://`                | OpenStack Swift                   | Swift native protocol    | HTTPS |
| `swhttps://`              | OpenStack Swift                   | Swift native protocol    | HTTPS |
| `swhttp://`               | OpenStack Swift                   | Swift native protocol    | HTTP  |
| `s3://`                   | OpenStack Swift *[with S3 gateway]* | S3                       | HTTPS |
| `s3https://`              | OpenStack Swift *[with S3 gateway]* | S3                       | HTTPS |
| `s3http://`               | OpenStack Swift *[with S3 gateway]* | S3                       | HTTP  |



## Limitations
The current implementation of this package has some limitations:

* The use of multi-range HTTP requests is not yet supported. Using this feature against the cloud storage implementations that support it is useful to reduce the number of round-trips necessary to retrieve the file contents. This is particularly relevant when accessing the remote files through a wide area network. Please note that neither Amazon S3 not Google Storage currently support this feature. Huawei's UDS storage appliance does though and OpenStack Swift has plans to support this feature.
* Not all cloud storage providers are currently supported: some big players such as Microsoft Azure is not yet supported but may be if really needed.
* This package has not been tested on Windows operating systems. It may work though, provided ROOT's and libNeon's dependencies can be satisfied there.
* It is not possible to write a remote cloud file from within a ROOT macro nor to retrieve the list of files stored in a cloud storage container.
* It does only a partial validation of the the server certificate (only checks its validity period but not all the chain of certification).

## Roadmap
There are several features we want to add to this software, such as:

* Add support for multi-range HTTP requests,
* Add support for other cloud storage providers,
* Add support for Rackspace (and Swift) identity API v2 which allows for discovery of storage endpoints,
* Improve integration with the operating system to cleanly validate the server certificate.

## How to contribute
Your contribution is more than welcome. There are several ways you can contribute if you wish:

* Testing this software with your preferred version of ROOT on your preferred operating system and letting us know if it works. If it does not work for you, please [open a new issue](https://github.com/airnandez/root-cloud/issues)
* If you find a bug, please report it by [opening an issue](https://github.com/airnandez/root-cloud/issues)
* If you spot a defect either in this documentation or in the source code documentation we consider it a bug so [please let us know](https://github.com/airnandez/root-cloud/issues)
* Providing feedback on how to improve this software [by opening an issue](https://github.com/airnandez/root-cloud/issues)

## Credits

### Author
This software was developed and is maintained by Fabio Hernandez. This work is funded by both [IN2P3/CNRS computing center](http://cc.in2p3.fr) (Lyon, France) and [IHEP computing center](http://english.ihep.cas.cn) (Beijing, China) 

### Acknowledgements
This work has been made possible by the contribution of several people:

* Lu Wang (IHEP computing center)
* Ziyan Deng and Qiumei Ma (IHEP, [BESIII experiment](http://bes3.ihep.ac.cn))

## License
Copyright 2013 Fabio Hernandez

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

[http://www.apache.org/licenses/LICENSE-2.0](http://www.apache.org/licenses/LICENSE-2.0)

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
