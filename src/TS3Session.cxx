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
// TS3Session                                                           //
//                                                                      //
// A TS3Session represents an HTTP session with an S3 server.           //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TROOT.h"
#include "TError.h"
#include "TSystem.h"
#include "TS3Request.h"
#include "TS3Session.h"


ClassImp(TS3Session)


//_____________________________________________________________________________
TS3Session::TS3Session(const TUrl& url, EAuthType authType,
              const TString& accessKey, const TString& secretKey)
   : THttpSession(url)
{
   // Create a S3 session for sending S3 requests with the host and ports in
   // the specified URL

   fAuthType = authType;
   fAccessKey = accessKey;
   fSecretKey = secretKey;
}


//_____________________________________________________________________________
TS3Session::~TS3Session()
{
}


//_____________________________________________________________________________
TS3Session& TS3Session::SetAccessKey(const TString& accessKey)
{
   // Set the access key to be used with all the S3 requests sent though this
   // S3 session.

   fAccessKey = accessKey;
   return *this;
}

//_____________________________________________________________________________
TS3Session& TS3Session::SetSecretKey(const TString& secretKey)
{
   // Set the secret key to be used with all the S3 requests sent though this
   // S3 session.

   fSecretKey = secretKey;
   return *this;
}

//_____________________________________________________________________________
TS3Session& TS3Session::SetAuthType( EAuthType authType)
{
   // Set the authentication type this S3 session needs to use. The S3 requests
   // sent thtough this session may use some variants of the authentication
   // mechanism accoding to the type of authentication. For instance, Google
   // and Amazon use slightly different mechanisms for signing S3 requests.

   fAuthType = authType;
   return *this;
}


//_____________________________________________________________________________
TS3Request* TS3Session::MakeRequest(const TString& verb, const TString& path, const TString& query)
{
   // Build a new S3 request to be used by this session.
   // Returns a new request created in the heap. It is the responsibility of
   // the caller to delete the returned request.
   // WARNING: there is strong coupling between the HTTP session object through
   // which the request is created and the request itself. You should not
   // destroy a session object if there are pending requests, as the result
   // is undefined and may crash your program. You must first explicitely
   // delete the requests and then the destroy session.

   return new TS3Request(this, verb, path, query);
}

