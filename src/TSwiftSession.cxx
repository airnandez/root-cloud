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
// A TSwiftSession is a persistent connection with a Swift object       //
// storage server to send requests for retrieving file contents.        //
//////////////////////////////////////////////////////////////////////////

#include "TROOT.h"
#include "TError.h"
#include "TSystem.h"
#include "TSwiftSession.h"
#include "TCloudExtension.h"


ClassImp(TSwiftSession)


//_____________________________________________________________________________
TSwiftSession::TSwiftSession(TSwiftIdentityClient* identityClient)
   : fIdentityClient(identityClient)
{
   // Create a Swift session for sending storage requests with the host and
   // ports in the specified URL

   // Note that at creation time the URL of the HTTP server this session must
   // be established with may not be set.
   if (fIdentityClient != 0) {
      SetServerUrl(fIdentityClient->GetStorageUrl());
   }

}


//_____________________________________________________________________________
TSwiftSession::~TSwiftSession()
{
   // Note that we don't delete the identity client, since it is owned by
   // the file TSwiftFile
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

   if (!fIdentityClient->IsAuthenticated()) {
      // Need to re-authenticate
      if (!fIdentityClient->Authenticate()) {
         Error("MakeRequest", "could not authenticate with server %s", fIdentityClient->GetAuthUrl().GetUrl());
         return 0;
      }

      // Set this session's HTTP server, which may be different than the
      // one we had used so far since we re-authenticated
      SetServerUrl(fIdentityClient->GetStorageUrl());
   }

   // Build the request and add the authentication header
   TString fullPath = path.BeginsWith("/") ? path : TString("/") + path;
   THttpRequest* request = new THttpRequest(this, verb, fullPath, query);
   if (request == 0) {
      Error("MakeRequest", "could not allocate memory for new HTTP request");
   } else {
      request->SetHeader("X-Auth-Token", fIdentityClient->GetStorageToken());
   }
   return request;
}

