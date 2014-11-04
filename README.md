# Cloud storage extension for ROOT data analysis framework

## Overview
This software is an extension to CERN's [ROOT](http://root.cern.ch) data analysis framework. Its purpose is to help ROOT users transparently read files stored remotely (i.e. "in the cloud") through cloud storage protocols such as Amazon's S3 or OpenStack's Swift.

That is, once this extension is installed, you can use the instruction below in in your ROOT macro or application to open the remote file for reading:

```cpp
TFile* f = TFile::Open("s3://s3.amazonaws.com/mybucket/path/to/my/file.root");
```

and then you can read its contents as if the file was in your local disk. See the **How to use** section below for more details.

## Motivation
Since version 5.34.05 (released on February 2013) ROOT has improved its built-in support for reading files hosted by Amazon S3, Google Storage or any other cloud storage service provider exposing [Amazon's Simple Storage Service (S3) REST API](http://aws.amazon.com/s3/). This functionality has been successfully tested against Amazon S3, Google Storage, OpenStack Swift (through the S3 gateway) and S3 appliances such as Huawei's UDS. *(Disclosure: the author of this extension contributed to improve ROOT's built-in support for cloud storage)*.

However, for several reasons, some ROOT users (be them individual scientists or scientific collaborations) cannot upgrade to the latest version of ROOT to benefit from this feature. This project intends to help removing this constraint by providing an extension so that those users have the the possibility to exploit cloud storage with their favorite version of ROOT. If you are one of those users, we hope this software may help you.

Additional context and background information for this work can be found in [this presentation](https://speakerdeck.com/airnandez/exploring-cloud-storage-for-bes-iii-data) and in [this paper](http://iopscience.iop.org/1742-6596/513/4/042050).

## Benefits
As a ROOT user, the main benefits of using this extension are:

* you can use cloud storage services for storing your data and transparently use ROOT for accessing them,
* you don't need to install the latest version of ROOT in order to use cloud storage: this extension allows you to continue using legacy ROOT version and exploit your data stored in the cloud,
* you don't need to patch your current production version of ROOT for using this extension: this package can be installed on top of a broad range of ROOT versions,
* you don't need to modify your ROOT macros to use this extension: the names of your remote files just need to be prefixed by a scheme (such as `s3://`) so that ROOT can identify them and use this extension for retrieving their contents,
* this extension is forward compatible: if you upgrade to a recent version of ROOT which has built-in support for cloud storage you won't have to change your file naming scheme,
* the memory footprint of your ROOT-based software is not significantly increased: the binary version of this package is less that 500 KBytes


## How this extension works
The document [How it Works](doc/HowItWorks.md) gives more details on the internals of this software. In addition, it presents the supported versions of ROOT this extension is compatible with, the cloud providers it has been tested against and the operating systems supported.


## How to install
Please refer to the [How to Install](doc/HowToInstall.md) document for all the details on the installation process.

## How to use
The simplest way to use this extension is to provide the URL of your file to the `TFile::Open` method, as below:

```cpp
TFile* f = TFile::Open("s3://s3.amazonaws.com/mybucket/path/to/my/file.root");
if (f != 0) {
   // File is open, show its contents
   f->ls();
}
```

You will find more details on the several ways you can use this extension in the [How to Use](doc/HowToUse.md) document.


## Limitations
The current implementation of this package has some limitations:

* The use of multi-range HTTP requests is not yet supported. Using this feature against the cloud storage implementations that support it is useful to reduce the number of round-trips necessary to retrieve the file contents. This is particularly relevant when accessing the remote files through a wide area network. Please note that neither Amazon S3 not Google Storage currently support this feature. Huawei's UDS storage appliance does though and OpenStack Swift has plans to support this feature.
* Not all cloud storage providers are currently supported: some big players such as Microsoft Azure is not yet supported but may be if really needed.
* This package has not been tested on Windows operating systems and it is unlikely it works without modification
* It is not possible to write a remote cloud file from within a ROOT macro nor to retrieve the list of files stored in a cloud storage container.
* It does only a partial validation of the the server certificate (only checks its validity period but not all the chain of certification).

## Roadmap
There are several features we want to add to this software, such as:

* Add support for multi-range HTTP requests,
* Add support for other cloud storage providers,
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

This work is based in other people's work, including:
* The [ROOT development team](http://root.cern.ch/drupal/content/root-development-team) team,
* The authors of the [libNeon](http://www.webdav.org/neon/) library
* The authors of the [RapidJSON](https://github.com/miloyip/rapidjson) library

Several people have contributed to this software:

* Lu Wang (IHEP computing center)
* Ziyan Deng and Qiumei Ma (IHEP, [BESIII experiment](http://bes3.ihep.ac.cn))
* Sébastien Gadrat (IN2P3/CNRS computing center)

## License
Copyright 2014 Fabio Hernandez

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

[http://www.apache.org/licenses/LICENSE-2.0](http://www.apache.org/licenses/LICENSE-2.0)

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
