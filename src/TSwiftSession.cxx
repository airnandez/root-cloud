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
// TSwiftSession                                                        //
//                                                                      //
// A TSwiftSession is a persistent connection with a Swift server though//
// which we can send requests for retrieving file contents.             //
//////////////////////////////////////////////////////////////////////////


#include "TROOT.h"
#include "TError.h"
#include "TSystem.h"
#include "TSwiftSession.h"


ClassImp(TSwiftSession)


//_____________________________________________________________________________
TSwiftSession::TSwiftSession(const TUrl& authUrl, const TString& userName, const TString& apiAccessKey)
   : THttpSession(authUrl)
{
   // Create a Swift session for sending storage requests with the host and
   // ports in the specified URL

   fUserName = userName;
   fApiAccessKey = apiAccessKey;
   fAuthUrl = authUrl;
}


//_____________________________________________________________________________
TSwiftSession::~TSwiftSession()
{
}


//_____________________________________________________________________________
TSwiftSession& TSwiftSession::SetUserName(const TString& userName)
{
   fUserName = userName;
   return *this;
}


//_____________________________________________________________________________
TSwiftSession& TSwiftSession::SetApiAccessKey(const TString& apiAccessKey)
{
   fApiAccessKey = apiAccessKey;
   return *this;
}


//_____________________________________________________________________________
Bool_t TSwiftSession::IsAuthenticated() const
{
   // Return kTRUE if we have already retrieved the authentication token for
   // this session.

   return (fStorageToken.Length() > 0) ? kTRUE : kFALSE;
}


//_____________________________________________________________________________
void TSwiftSession::ResetAuthentication()
{
   fStorageUrl = "";
   fStorageToken = "";
}


//_____________________________________________________________________________
Bool_t TSwiftSession::Authenticate()
{
   // Contacts the server to retrieve the authentication information for this
   // session. All the subsequent requests sent through this session will
   // use the same authentication token.

   // If already authenticated, reset the auth token and storage URLs
   if (IsAuthenticated())
      ResetAuthentication();

   // Build an authentication HTTP request to be sent to the server
   SetServerUrl(fAuthUrl);

   // Try these paths for authentication. Different instances of Swift use
   // different authentication paths
   TString storageUrl;
   TString storageToken;
   const char* authPaths[] = {
      "/auth/v1.0",
      "/v1.0",
   };
   const Int_t kNumPaths = sizeof(authPaths)/sizeof(authPaths[0]);
   for (Int_t i=0; i < kNumPaths; i++) {
      // Try next authentication path
      TString path(authPaths[i]);

      // Create a new request, set the required headers for authenticationg
      // with the Swift server and submit it
      THttpRequest* request = new THttpRequest(this, "GET", path);
      request->SetHeader("X-Auth-User", fUserName);
      request->SetHeader("X-Auth-Key", fApiAccessKey);
      if (!request->Submit()) {
         // Could not submit the request
         delete request;
         continue;
      }

      if (request->GetResponseStatusClass() != THttpRequest::kClassSuccess) {
         // The request was submitted but did not succeed
         delete request;
         continue;
      }

      // We got a successful response to our authentication request. Extract
      // the relevant information.
      storageUrl = request->GetResponseHeader("X-Storage-Url");
      storageUrl = storageUrl.Strip(TString::kTrailing, '/');
      storageToken = request->GetResponseHeader("X-Storage-Token");
      delete request;
      break;
   }

   if ((storageUrl.Length() == 0) || (storageToken.Length() == 0)) {
      Error("Authenticate", "error retrieving authentication token from Swift server");
      Error("Authenticate", "make sure your user name and API access key are correct");
      return kFALSE;
   }

   if (gDebug > 0) {
      Info("Authenticate", "storage URL: '%s'", storageUrl.Data());
      Info("Authenticate", "storage token: '%s'", storageToken.Data());
   }

   fStorageToken = storageToken;
   fStorageUrl = TUrl(storageUrl);

   // Set the URL of the storage server. The host may be different than the
   // one used for authentication, so the underlying network connection may
   // be terminated, since all subsequent requests need to be sent to the
   // Swift storage server.
   SetServerUrl(fStorageUrl);
   return kTRUE;
}


//_____________________________________________________________________________
THttpRequest* TSwiftSession::MakeRequest(const TString& verb, const TString& path, const TString& query)
{
   // Build a new Swift request to be used by this session.
   // Returns a new request created in the heap. It is the responsibility of
   // the caller to delete the returned request.
   // WARNING: there is strong coupling between the HTTP session object through
   // which the request is created and the request itself. You should not
   // destroy a session object if there are pending requests, as the result
   // is undefined and may crash your program. You must first delete the
   // requests and then the session.

   if (!IsAuthenticated() && !Authenticate()) {
      return 0;
   }

   // Build the path for this request. The 'path' argument must be concatenated
   // to the path of the storage URL used by this session, if any.
   TString fullPath = fStorageUrl.GetFile();
   if (fullPath.Length() == 0) {
      fullPath = path;
   } else {
      const char* slash = path.BeginsWith("/") ? "" : "/";
      fullPath += TString::Format("%s%s", slash, path.Data());
   }
   if (!fullPath.BeginsWith("/")) {
      fullPath.Insert(0, "/");
   }

   // Build the request
   THttpRequest* request = new THttpRequest(this, verb, fullPath, query);

   // Add the authentication headers to this new request
   request->SetHeader("X-Auth-Token", fStorageToken);
   return request;
}

