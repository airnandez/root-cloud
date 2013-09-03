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
// TSwiftFile                                                           //
//                                                                      //
// A TSwiftFile is a TFile which retrieves its contents from a file     //
// server using the native  OpenStack Swift protocol (as opposed to S3  //
// which can be used to some OpenStack installations).                  //
// The recommended way to open a Swift file is via                      //
// TFile* f=TFile::Open("swift://myserver.domain.com/bucket/myFile.root")//                                                                   //
//////////////////////////////////////////////////////////////////////////


#include "TROOT.h"
#include "TError.h"
#include "TSystem.h"
#include "TPRegexp.h"
#include "TTimeStamp.h"
#include "TSwiftFile.h"


ClassImp(TSwiftFile)

//_____________________________________________________________________________
TSwiftFile::TSwiftFile(const char* url, Option_t* options, const char* ftitle, Int_t compress)
          : THttpFile(url, "NOINIT", ftitle, compress)
{
   // Construct a TSwiftFile object. The url argument is of the form:
   //
   //       swift://host.example.com/bucket/path/to/my/file
   //      swhttp://host.example.com/bucket/path/to/my/file
   //     swhttps://host.example.com/bucket/path/to/my/file
   //
   // The recommended way to create an instance of this class is through
   // TFile::Open, for instance:
   //
   // TFile* f = TFile::Open("swift://host.example.com/bucket/path/to/my/file")
   //
   // The protocol used for retrieving the file contents depends on the scheme
   // used for the file:
   //
   //       Scheme               Protocol
   //       ------               --------
   //       swift                  https
   //       swhttps                https
   //       swhttp                 http
   //
   // The 'options' argument can contain 'NOPROXY' if you want to bypass
   // the HTTP proxy when retrieving this file's contents. As for any THttpFile-derived
   // object, the URL of the web proxy can be specified by setting an environmental
   // variable 'http_proxy'. If this variable is set, we ask that proxy to route our
   // requests HTTP(S) requests to the file server.
   //
   // In addition, you can also use the 'options' argument to provide the user name
   // and API access key to be used for authentication purposes for this file by using a
   // string of the form "AUTH=myUserName:myAPIAccessKey". This may be useful to
   // open several files hosted by different providers in the same program/macro,
   // where the environemntal variables solution is not convenient (see below).
   //
   // If you need to specify both NOPROXY and AUTH separate them by ' '
   // (blank), for instance:
   // "NOPROXY AUTH=myAccessKey:mySecretKey"
   //
   // Examples:
   //    TFile* f1 = TFile::Open("swift://host.example.com/bucket/path/to/my/file",
   //                            "NOPROXY AUTH=myAccessKey:mySecretKey");
   //    TFile* f2 = TFile::Open("swift://host.example.com/bucket/path/to/my/file",
   //                            "AUTH=myAccessKey:mySecretKey");
   //
   // If there is no authentication information in the 'options' argument
   // (i.e. not AUTH="....") the values of the environmental variables
   // SWIFT_API_USER and SWIFT_API_KEY (if set) are expected to contain
   // the user name and API key, respectively. You have
   // been provided with these credentials by your Swift service provider.

   if (!Initialize(TUrl(url), options)) {
      MakeZombie();
      gDirectory = gROOT;
   }
}


//_____________________________________________________________________________
TSwiftFile::~TSwiftFile()
{
}


//_____________________________________________________________________________
Bool_t TSwiftFile::Initialize(const TUrl& url, Option_t* options)
{
   // Initialize this Swift file. Extracts bucket name and object key from the
   // provided URL and parses options looking for the S3 credentials for
   // opening this file using the S3 protocol.

   // Make sure the URL of this Swift file is conformant to what we expect.
   // An accepted Swift path is of the form:
   //           swift://host[:port]/bucket/path/to/my/file
   //          swhttp://host[:port]/bucket/path/to/my/file
   //         swhttps://host[:port]/bucket/path/to/my/file

   TUrl tempUrl(url); // In ROOT v5.24 GetUrl() is not const, so we need a temp URL
   const char* path = tempUrl.GetUrl();
   TPMERegexp rex("^(swift|swhttps?){1}://([^/]+)/([^/]+)/([^/].*)", "i");
   if (rex.Match(TString(path)) != 5) {
      Error("Initialize", "'%s' is not a valid Swift file identifier", path);
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
      (const char*)rex[2],   // host[:port]
      fBucket.Data(),        // bucket
      fObjectKey.Data())     // object key
   );

   // Retrieve the authentication information from 'options' or from the
   // environmental variables
   const char* kSwiftUserName = "SWIFT_API_USER";
   const char* kSwiftApiAccessKey = "SWIFT_API_KEY";
   if (!GetAuthFromOptions(options, fUserName, fApiAccessKey)) {
      // There is not auth information in the options. Check in the environment.
      if (!GetAuthFromEnv(kSwiftUserName, kSwiftApiAccessKey, fUserName, fApiAccessKey)) {
         // Test alternative environmental variables (for compatibility with
         // class TS3File)
         const char* kAccessKeyEnv = "S3_ACCESS_KEY";
         const char* kSecretKeyEnv = "S3_SECRET_KEY";
         GetAuthFromEnv(kAccessKeyEnv, kSecretKeyEnv, fUserName, fApiAccessKey);
      }
      // TODO: handle the case when we cannot find Swift auth info in environmental variables
   }

   // Finalize initialization of the super-class
   if (!THttpFile::Initialize(fullUrl, options)) {
      // There was an error initializing the super-class. If we have not
      // authentication info, show an error message as this may be the cause
      // of the problem (at least for files that do require credentials for
      // accessing them, i.e. those which are not world-readable)
      if (fUserName.IsNull() || fApiAccessKey.IsNull()) {
         Error("Initialize", "could not find authentication info in "\
               "'options' argument nor in environment variables '%s' and '%s'",
               kSwiftUserName, kSwiftApiAccessKey);
      }
      return kFALSE;
   }

   return kTRUE;
}


//_____________________________________________________________________________
TString TSwiftFile::GetFilePath() const
{
   return TString::Format("/%s%s", fBucket.Data(), fObjectKey.Data());
}


//_____________________________________________________________________________
TSwiftFile& TSwiftFile::SetUserName(const TString& userName)
{
   fUserName = userName;
   return *this;
}


//_____________________________________________________________________________
TSwiftFile& TSwiftFile::SetApiAccessKey(const TString& apiAccessKey)
{
   fApiAccessKey = apiAccessKey;
   return *this;
}


//_____________________________________________________________________________
TSwiftSession* TSwiftFile::MakeSession(const TUrl& fileUrl)
{
   // Make a session for sending Swift requests for retrieving this file.
   // Overwrites THttpFile::MakeSession.

   return new TSwiftSession(fileUrl, fUserName, fApiAccessKey);
}
