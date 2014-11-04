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
#include "TObjString.h"
#include "TSwiftFile.h"
#include "TCloudExtension.h"


ClassImp(TSwiftFile)

//_____________________________________________________________________________
TSwiftFile::TSwiftFile(const char* url, Option_t* options, const char* ftitle, Int_t compress)
          : THttpFile(url, "NOINIT", ftitle, compress), fIdClient(0)
{
   // Construct a TSwiftFile object. The url argument is of the form:
   //
   //       swift://container/path/to/my/file
   //
   // The recommended way to create an instance of this class is through
   // TFile::Open, for instance:
   //
   //    TFile* f = TFile::Open("swift://container/path/to/my/file");
   //
   // The protocol used for retrieving the file contents depends depends on
   // how the Swift service is configured. It can be either HTTP or HTTPS.
   // HTTPS is typically used for most installations.
   //
   // The credentials and identity service URL need to be provided so that
   // this extension can retrieve the intended file contents. There are two
   // ways to pass this information: using environment variables or using
   // the 'options' argument.
   //
   // Using environment variables is a simple way to passing authentication
   // information. Before opening a Swift file, you can set the following
   // environment variables, for instance:
   //
   //    OS_AUTH_URL=https://identity.example.com:5000
   //    OS_TENANT_NAME=tenant
   //    OS_USERNAME=user
   //    OS_PASSWORD=AbCdEfG1234567
   //
   // This information is provided to you by your Swift service provider.
   //
   // If you need to open files hosted by several different Swift providers you
   // can specify the authentication information on a per-file basis using
   // the 'options' argument. For instance:
   //
   // const char* options = "OS_AUTHURL=https://identity.example.com:5000  OS_TENANT_NAME=tenant OS_USERNAME=user OS_PASSWORD=AbCdEfG1234567";
   // TFile* f = TFile::Open("swift://container/path/to/my/file", options);
   //
   // In addition, the 'options' argument can also contain the word 'NOPROXY' if you
   // want to bypass the HTTP proxy when retrieving this file's contents. As for
   // any THttpFile-derived object, the URL of the web proxy can be specified by
   // setting an environmental variable 'http_proxy'. If this variable is set,
   // we use that proxy to route our HTTP(S) requests to the file server.
   //

   if (!Initialize(TUrl(url), options)) {
      MakeZombie();
      gDirectory = gROOT;
   }
}


//_____________________________________________________________________________
TSwiftFile::~TSwiftFile()
{
   if (fIdClient != 0) {
      delete fIdClient;
   }
}


//_____________________________________________________________________________
Bool_t TSwiftFile::Initialize(const TUrl& url, Option_t* options)
{
   // Initialize this Swift file. Extracts bucket name and object key from the
   // provided URL and parses options looking for the OpenStack credentials for
   // opening this file using the Swift REST API.

   // Make sure the URL of this Swift file is conformant to what we expect.
   // An accepted Swift path is of the form:
   //           swift://bucket/path/to/my/file

   TUrl tempUrl(url); // In ROOT v5.24 GetUrl() is not const, so we need a temp URL
   const char* path = tempUrl.GetUrl();
   TPMERegexp rex("^(swift://)([^/]+)/([^/].*)$", "i");
   if (rex.Match(TString(path)) != 4) {
      Error("Initialize", "'%s' is not a valid Swift file identifier", path);
      return kFALSE;
   }

   // Save the bucket and object key. Note that we store the object key
   // starting with "/".
   fBucket = rex[2];
   fObjectKey = TString::Format("/%s", (const char*)rex[3]);

   // Retrieve the authentication information from 'options' or from the
   // environmental variables
   TString authUrl;
   TString tenantName;
   TString userName;
   TString password;
   if (!GetSwiftCredentialsFromOptions(TString(options), authUrl, tenantName, userName, password)) {
      // There is not auth information in the options. Check in the environment.
      if (!GetSwiftCredentialsFromEnv(authUrl, tenantName, userName, password)) {
         Error("Initialize", "could not find authentication and credential info in "\
               "'options' argument nor in environment variables");
         return kFALSE;
      }
   }

   // Contact the identity service to retrieve the storage URL and the storage
   // token we must use for retrieving this file's contents
   fIdClient = new TSwiftIdentityClient(TUrl(authUrl), tenantName, userName, password);
   if (fIdClient == 0) {
      Error("Initialize", "could not allocate memory for identity client");
      return kFALSE;
   }

   if (!fIdClient->Authenticate()) {
      Error("Initialize", "could not authenticate to identity service at %s", authUrl.Data());
      return kFALSE;
   }

   // Build the file URL and initialize the super-class
   TString fileUrl = TString(fIdClient->GetStorageUrl().GetUrl()) + GetFilePath();

   if (TCloudExtension::fgDebugLevel > 0) {
      Info("Initialize", "Authentication succeeded");
      Info("Initialize", "  Storage URL=%s", fIdClient->GetStorageUrl().GetUrl());
      Info("Initialize", "Storage Token=%s", fIdClient->GetStorageToken().Data());
      Info("Initialize", "     File URL=%s", fileUrl.Data());
   }

   if (!THttpFile::Initialize(fileUrl.Data(), options)) {
      // There was an error initializing the super-class.
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
TSwiftSession* TSwiftFile::MakeSession(const TUrl&)
{
   // Make a session for sending Swift requests for retrieving this file.
   // Overwrites THttpFile::MakeSession.

   return new TSwiftSession(fIdClient);
}


//_____________________________________________________________________________
Bool_t TSwiftFile::GetSwiftCredentialsFromEnv(TString& authUrl, TString& tenantName,
      TString& userName, TString& password)
{
   // Retrieve from the environment the Swift credentials and identity service
   // URL to use for this file.

   authUrl = gSystem->Getenv("OS_AUTH_URL");
   if (authUrl.IsNull()) {
      Error("GetSwiftCredentialsFromEnv", "environment variable OS_AUTH_URL not set");
      return kFALSE;
   }

   tenantName = gSystem->Getenv("OS_TENANT_NAME");
   if (tenantName.IsNull()) {
      Error("GetSwiftCredentialsFromEnv", "environment variable OS_TENANT_NAME not set");
      return kFALSE;
   }

   userName = gSystem->Getenv("OS_USERNAME");
   if (userName.IsNull()) {
      Error("GetSwiftCredentialsFromEnv", "environment variable OS_USERNAME not set");
      return kFALSE;
   }

   password = gSystem->Getenv("OS_PASSWORD");
   if (password.IsNull()) {
      Error("GetSwiftCredentialsFromEnv", "environment variable OS_PASSWORD not set");
      return kFALSE;
   }

   return kTRUE;
}

//_____________________________________________________________________________
Bool_t TSwiftFile::GetSwiftCredentialsFromOptions(const TString& options,
      TString& authUrl, TString& tenantName,
      TString& userName, TString& password)
{
   // Retrieve the Swift credentials and identity service URL to use for this
   // file from the options specified at open time

   // The options string is expected to be of the forms:
   //   OS_AUTHURL=https://host.example.com:8443/auth/v1.0  OS_TENANT_NAME=tenant OS_USERNAME=user OS_PASSWORD=password
   //   OS_AUTHURL="https://host.example.com:8443/auth/v1.0"  OS_TENANT_NAME="tenant" OS_USERNAME="user" OS_PASSWORD="password"
   TObjArray* a = options.Tokenize(" ");
   if (a == 0) {
      return kFALSE;
   }

   authUrl = tenantName = userName = password = "";
   for (int i=0; i < a->GetEntries(); i++) {
      TObjString* t = (TObjString*)a->UncheckedAt(i);
      TString token = t->GetString();
      if (token.BeginsWith("OS_AUTH_URL=")) {
         authUrl = token.Remove(0, ::strlen("OS_AUTH_URL="));
         authUrl.Remove(TString::kBoth, '"');
      } else if (token.BeginsWith("OS_TENANT_NAME=")) {
         tenantName = token.Remove(0, ::strlen("OS_TENANT_NAME="));
         tenantName.Remove(TString::kBoth, '"');
      } else if (token.BeginsWith("OS_USERNAME=")) {
         userName = token.Remove(0, ::strlen("OS_USERNAME="));
         userName.Remove(TString::kBoth, '"');
      } else if (token.BeginsWith("OS_PASSWORD=")) {
         password = token.Remove(0, ::strlen("OS_PASSWORD="));
         password.Remove(TString::kBoth, '"');
      }
   }

   delete a;
   return !authUrl.IsNull() && !tenantName.IsNull() &&
          !userName.IsNull() && !password.IsNull();
}

