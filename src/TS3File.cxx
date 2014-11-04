// @(#)root/net:$Id$
//
// Author: Fabio Hernandez (fabio@in2p3.fr)
//

/*************************************************************************
 * Copyright (C) 2013 Fabio Hernandez                                    *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms and list of contributors see:                 *
 *                                                                       *
 *              https://github.com/airnandez/root-cloud                  *
 *                                                                       *
 *************************************************************************/


//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TS3File                                                              //
//                                                                      //
// A TS3Fileis a THttpFile which retrieves the file contents from a     //
// web server implementing the REST API of the Amazon S3 protocol. This //
// class is meant to be as generic as possible to be used with files    //
// hosted not only by Amazon S3 servers but also by other providers     //
// implementing the core of the S3 protocol.                            //
//                                                                      //
// The S3 protocol works on top of HTTPS (and HTTP) and imposes that    //
// each HTTP request be signed using a specific convention: the request //
// must include an 'Authorization' header which contains the signature  //
// of a concatenation of selected request fields. For signing the       //
// request, an 'Access Key Id' and a 'Secret Access Key' need to be     //
// known. These keys are used by the S3 servers to identify the client  //
// and to authenticate the request as genuine.                          //
//                                                                      //
// As an end user, you must know the Access Key and Secret Access Key   //
// in order to access each S3 file. They are provided to you by your S3 //
// service provider. Those two keys can be provided to ROOT when        //
// initializing an object of this class by two means:                   //
//                                                                      //
// a) by setting the environmental variables S3_ACCESS_KEY and          //
//    S3_SECRET_KEY to the appropriate values, or                       //
// b) by specifying them as an argument when opening each file.         //
//                                                                      //
// The first method is convenient if all the S3 files you want to       //
// access are hosted by a single provider. The second one is more       //
// flexible as it allows you to specify which credentials to use        //
// on a per-file basis. See the documentation of the constructor of     //
// this class for details on the syntax.                                //
//                                                                      //
// For doing the actual HTTP exchange, this class inherits from         //
// THttpFile which in turns uses libNeon.                               //
//                                                                      //
// For more information on the details of S3 protocol please refer to:  //
// "Amazon Simple Storage Service Developer Guide":                     //
// http://docs.amazonwebservices.com/AmazonS3/latest/dev/Welcome.html   //
//                                                                      //
// "Amazon Simple Storage Service REST API Reference"                   //
//  http://docs.amazonwebservices.com/AmazonS3/latest/API/APIRest.html  //
//////////////////////////////////////////////////////////////////////////


#include "TROOT.h"
#include "TError.h"
#include "TSystem.h"
#include "TPRegexp.h"
#include "TObjString.h"
#include "THttpSession.h"
#include "TS3File.h"


ClassImp(TS3File)


//_____________________________________________________________________________
TS3File::TS3File(const char* url, Option_t* options, const char* ftitle, Int_t compress)
          : THttpFile(url, "NOINIT", ftitle, compress)
{
   // Construct a TS3File object. The url argument is expected to be of one
   // of the following forms:
   //
   //         s3://host.example.com/bucket/path/to/my/file
   //     s3http://host.example.com/bucket/path/to/my/file
   //    s3https://host.example.com/bucket/path/to/my/file
   //
   // For files hosted by Google Storage, use the following forms:
   //
   //        gs://storage.googleapis.com/bucket/path/to/my/file
   //    gshttp://storage.googleapis.com/bucket/path/to/my/file
   //  gsthttps://storage.googleapis.com/bucket/path/to/my/file
   //
   //
   // The recommended way to create an instance of this class is through
   // TFile::Open, for instance:
   //
   //   TFile* f1 = TFile::Open("s3://host.example.com/bucket/path/to/my/file")
   //   TFile* f2 = TFile::Open("gs://storage.googleapis.com/bucket/path/to/my/file")
   //
   // The specified scheme (i.e. s3, s3http, s3https, ...) determines the underlying
   // transport protocol to use for downloading the file contents, namely HTTP or HTTPS.
   // The 's3', 's3https', 'gs' and 'gshttps' schemes imply using HTTPS as the transport
   // protocol. The 's3http', 'as3' and 'gshttp' schemes imply using HTTP as the transport
   // protocol.
   //
   // Your user credentials are extracted from the environment variables
   // S3_ACCESS_KEY and S3_SECRET_KEY. If set, those values are used for
   // opening all files with scheme 's3*:' or 'gs*:'.
   //
   // You can also specify the credentials to use on a per-file basis by using
   // the 'options' argument. For instance:
   //
   //    TFile* f=TFile::Open("s3://host.example.com/bucket/path/to/my/file",
   //                         "S3_ACCESS_KEY=AWSACCESSKEY S3_SECRET_KEY=1234567890WECUBIPO")
   //
   // The 'options' argument can also contain 'NOPROXY' if you want to bypass
   // the HTTP proxy when retrieving this file's contents. As for any THttpFile-derived
   // object, the URL of the web proxy can be specified by setting an environmental
   // variable 'http_proxy'. If this variable is set, we ask that proxy to route our
   // requests HTTP(S) requests to the file server.
   //
   // If the user credentials are neither provided in the 'options' argument
   // nor via the environment variables, we try to open the file
   // without providing any authentication information to the server. This
   // is useful when the file is set an access control that allows for
   // any unidentified user to read the file.

   if (!Initialize(TUrl(url), options)) {
      MakeZombie();
      gDirectory = gROOT;
   }
}


//_____________________________________________________________________________
TS3File::~TS3File()
{
}


//_____________________________________________________________________________
Bool_t TS3File::Initialize(const TUrl& url, Option_t* options)
{
   // Initialize this S3 file. Extracts bucket name and object key from the
   // provided URL and parses options looking for the S3 credentials for
   // opening this file using the S3 protocol.

   // Make sure the URL of this S3 file is conformant to what we expect.
   // An accepted S3 path is of the form:
   //         s3://host[:port]/bucket/path/to/my/file
   //     s3http://host[:port]/bucket/path/to/my/file
   //    s3https://host[:port]/bucket/path/to/my/file
   //         gs://host[:port]/bucket/path/to/my/file
   //     gshttp://host[:port]/bucket/path/to/my/file
   //    gshttps://host[:port]/bucket/path/to/my/file

   TUrl tempUrl(url); // In ROOT v5.24 GetUrl() is not const, so we need a temp URL
   const char* path = tempUrl.GetUrl();
   TPMERegexp rex("^(s3|s3https?|gs|gshttps?){1}://([^/]+)/([^/]+)/([^/].*)", "i");
   if (rex.Match(TString(path)) != 5) {
      Error("Initialize", "'%s' is not a valid S3 file identifier", path);
      return kFALSE;
   }

   // Save the bucket and object key. Note that we store the object key
   // starting with "/".
   fBucket = rex[3];
   fObjectKey = TString::Format("/%s", (const char*)rex[4]);

   // Build URL of this object
   TString protocol = (rex[1].EndsWith("http", TString::kIgnoreCase)) ? "http"
                                                                      : "https";
   TUrl fullUrl(TString::Format("%s://%s/%s%s",
      protocol.Data(),
      (const char*)rex[2],      // host[:port]
      fBucket.Data(),
      fObjectKey.Data())
   );

   // Retrieve the authentication information from 'options' or from the
   // environmental variables
   if (!GetCredentialsFromOptions(TString(options), fAccessKey, fSecretKey)) {
      // There is not auth information in the options. Check in the environment.
      GetCredentialsFromEnv(fAccessKey, fSecretKey);
   }

   // Determine what flavor of S3 authentication do we need to use with
   // this S3 file: Amazon, Google, ...
   if (fAccessKey.IsNull() || fSecretKey.IsNull()) {
      // We have no authentication information, neither in the options
      // nor in the enviromental variables. So may be this is a
      // world-readable file, so let's continue and see if
      // we can open it.
      fAuthType = TS3Session::kNoAuth;
   } else {
      // Set the authentication information we need to use for this file
      fAuthType = (rex[1].BeginsWith("gs")) ? TS3Session::kGoogle
                                            : TS3Session::kAmazon;
   }

   // Assume this server does not serve multi-range HTTP GET requests. We
   // will detect this when the HTTP headers of this files are retrieved
   // later in the initialization process
   fUseMultiRange = kFALSE;

   // Finalize initialization of the super-class
   if (!THttpFile::Initialize(fullUrl, options)) {
      // There was an error initializing the super-class. If we have not
      // authentication info, show an error message as this may be the cause
      // of the problem (at least for files that do require credentials for
      // accessing them, i.e. those which are not world-readable)
      if (fAccessKey.IsNull() || fSecretKey.IsNull()) {
         Error("TS3File", "could not find authentication info in "\
            "'options' argument nor in environmental variables 'S3_ACCESS_KEY' and 'S3_SECRET_KEY'");
      }
      return kFALSE;
   }

   return kTRUE;
}


//_____________________________________________________________________________
TString TS3File::GetFilePath() const
{
   // Returns the absolute file path, which includes the bucket and the
   // object key.

   return TString::Format("/%s%s", fBucket.Data(), fObjectKey.Data());
}


//_____________________________________________________________________________
TS3File& TS3File::SetAccessKey(const TString& accessKey)
{
   // Set the access key for authenticating S3 requests for accessing this
   // file contents.

   fAccessKey = accessKey;
   return *this;
}


//_____________________________________________________________________________
TS3File& TS3File::SetSecretKey(const TString& secretKey)
{
   // Set the secret key for authenticating S3 requests for accessing this
   // file contents.

   fSecretKey = secretKey;
   return *this;
}


//_____________________________________________________________________________
TS3Session* TS3File::MakeSession(const TUrl& fileUrl)
{
   // Make a session for sending S3 requests for retrieving this file.
   // Overwrites THttpFile::MakeSession.

   return new TS3Session(fileUrl, fAuthType, fAccessKey, fSecretKey);
}


//_____________________________________________________________________________
Bool_t TS3File::GetCredentialsFromEnv(TString& accessKey, TString& secretKey)
{
   accessKey = gSystem->Getenv("S3_ACCESS_KEY");
   secretKey = gSystem->Getenv("S3_SECRET_KEY");
   return !accessKey.IsNull() && !secretKey.IsNull();
}


//_____________________________________________________________________________
Bool_t TS3File::GetCredentialsFromOptions(const TString& options,
   TString& accessKey, TString& secretKey)
{
   // Retrieve the credentials from the options specified at open time

   // The options string is expected to be of the forms:
   //   S3_ACCESS_KEY=AWSACCESSKEY  S3_SECRET_KEY=1234567890WECUBIPO
   //   S3_ACCESS_KEY="AWSACCESSKEY" S3_SECRET_KEY="1234567890WECUBIPO"
   TObjArray* a = options.Tokenize(" ");
   if (a == 0) {
      return kFALSE;
   }

   accessKey = secretKey = "";
   for (int i=0; i < a->GetEntries(); i++) {
      TObjString* t = (TObjString*)a->UncheckedAt(i);
      TString token = t->GetString();
      if (token.BeginsWith("S3_ACCESS_KEY=")) {
         accessKey = token.Remove(0, ::strlen("S3_ACCESS_KEY="));
         accessKey.Remove(TString::kBoth, '"');
      } else if (token.BeginsWith("S3_SECRET_KEY=")) {
         secretKey = token.Remove(0, ::strlen("S3_SECRET_KEY="));
         secretKey.Remove(TString::kBoth, '"');
      }
   }

   delete a;
   return !accessKey.IsNull() && !secretKey.IsNull();
}



//_____________________________________________________________________________
ClassImp(TS3FilePlugin)

//_____________________________________________________________________________
TS3FilePlugin::TS3FilePlugin(const char* url, Option_t* options)
   : TS3File(url, options, "", 1)
{
}

//_____________________________________________________________________________
TS3FilePlugin::~TS3FilePlugin()
{
}

