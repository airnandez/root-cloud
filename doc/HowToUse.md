## How to use `root-cloud`

For opening a cloud file from within a ROOT macro, you have to specify two pieces of information:

1. the location of your file in the form of a URL,
2. the authentication information provided to you by your storage provider, i.e. your user credentials.

The **URL of your file** contains the name of the host ROOT has to connect to retrieve the contents of your file, the name of the container where your file is located (a.k.a. bucket) and the full path (a.k.a. object key) of your file. The name of the host depends on the service provider you use. Typical URLs for several providers are presented in the table below:

| Provider          | Example file URL |
| ----------------- |  ------------ |
| Amazon S3         |  `s3://s3.amazonaws.com/myBucket/path/to/my/file.root` |
| Google Storage    |  `gs://storage.googleapis.com/myBucket/path/to/my/file.root` |
| Rackspace         |  `swift://myBucket/path/to/my/file.root` |
| Self-hosted OpenStack Swift   |  `swift://myBucket/path/to/my/file.root` |
| HTTP(S) server    |  `http://www.mylab.org/path/to/my/file.root` |

The **authentication information** is composed of several pieces, depending on the kind of cloud storage provider where the file is hosted.

### How to use with Amazon S3 and Google Storage

For Amazon S3 and Google Storage, two pieces are needed: an *access key* and a *secret key*. This key pair is used by this ROOT extension for sending information to the storage server so it can authenticate the requests as genuine. Please note that your secret key is never sent to the storage server, it is used for signing the requests ROOT sends to the server.

If all your remote files are hosted by a single provider, the easiest way is to set the authentication information in your environment by using a pair of environmental variables, for example:

```bash
$ export S3_ACCESS_KEY="my access key"
$ export S3_SECRET_KEY="my very long, very secret and impossible to remember secret key"
```

and then, in your C++ ROOT macro you can do:

```cpp
// Since no authentication information is provided to the TFile::Open
// method, ROOT will use the environment variables to retrieve
// the user credentials
TFile* f = TFile::Open("s3://s3.amazonaws.com/myBucket/path/to/my/file.root");
if (f != 0) {
   // This ROOT file is successfully open: inspect its contents
   f->ls();
}
```

However, if from the same ROOT macro you want to read files hosted by several providers, you can specify the  authentication information for each file as a second argument to the `TFile::Open` method, as shown below:

```cpp
// Open one file hosted by Amazon S3 (need to provide my credentials for Amazon AWS)
TFile* f1 = TFile::Open("s3://s3.amazonaws.com/myBucket/path/to/my/file.root",
                        "S3_ACCESS_KEY=MyAWSAccessKey S3_SECRET_KEY=AWS1234567890WECUBIPO");

// Open another file hosted by Google Storage (need to provide my credentials for Google Storage)
TFile* f2 = TFile::Open("gs://storage.googleapis.com/myBucket/path/to/my/file.root",
                        "S3_ACCESS_KEY=myGOOGLEAccessKey S3_SECRET_KEY=GOOG1234567890WECUBIPO");
```

### How to use with OpenStack Swift and Rackspace

If your files are hosted by an instance of OpenStack Swift or by Rackspace, you can set the environment variables with the relevant authentication information provided to you by your storage provider:

```bash
$ export OS_AUTH_URL="https://identity.example.com/v2.0"
$ export OS_TENANT_NAME="tenant"
$ export OS_USERNAME="user"
$ export OS_PASSWORD="secret"
```

and then, from the ROOT macro do:

```cpp
TFile* f = TFile::Open("swift://container/path/to/my/file.root");
if (f != 0) {
   // This ROOT file is successfully open: inspect its contents
   f->ls();
}
```

Note that unlike the Amazon S3 case, in the case of a file served by OpenStack Swift the file URL passed as argument to the `TFile::Open` method does not contain any host name. The host name to use for retrieving the file contents is determined at runtime by issueing some requests to the host pointed to by the variable `OS_AUTH_URL`.

If you need to provide your OpenStack authentication credentials on a per-file basis, you can provide them as a second argument to the `TFile::Open` method, as shown below:

```cpp
// Open a file hosted by a Swift instance. Provide authentication credentials.
TFile* f1 = TFile::Open("swift://container/path/to/my/file.root",
                        "OS_AUTH_URL=https://identity.example.com/v2.0  OS_TENANT_NAME=tenant OS_USERNAME=user OS_PASSWORD=secret");
```

**WARNING:** *although we present it that way for illustration purposes, it is not recommended practice to store your authentication credentials in your source code. In particular, your secret key must be kept secret. One way of providing your authentication credentials to ROOT is storing them in a protected file and reading them by the ROOT macro at run time for computing the second argument just before you need to open the file.*

### What transport is used in which case

For retrieving your file contents, ROOT will decide which code of this extension it needs to execute based on the scheme of your file's URL. In addition, the underlying protocol used to actually download the file contents (i.e. HTTP or HTTPS) also depends on the scheme of your file and on the transports supported by the storage provider. The table below summarizes the possibilities currently supported:


| If you use this scheme…   | with this storage provider…       | this extension will use this protocol… | on top of this transport… |
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
|                           |                                   |                          |       |
| `swift://`                | OpenStack Swift                   | Swift native protocol    | HTTPS |


For security reasons, we recommend to use schemes that imply HTTPS as the underlying transport, in particular if you are accessing your files through the open Internet.