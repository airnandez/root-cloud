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

#ifndef ROOT_TS3Session
#define ROOT_TS3Session

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TS3Session                                                           //
//                                                                      //
// A TS3Session represents an HTTP session with an S3 server.           //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef ROOT_TUrl
#include "TUrl.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif

#ifndef ROOT_THttpSession
#include "THttpSession.h"
#endif

#ifndef ROOT_TS3Request
#include "TS3Request.h"
#endif


class TS3Session: public THttpSession {

public:
   enum EAuthType { kNoAuth, kAmazon, kGoogle };

private:
   // Data members
   TString      fAccessKey;    // Access key id
   TString      fSecretKey;    // Secret key id
   EAuthType    fAuthType;     // Authentication type

   TS3Session();

public:
   // Constructors & Destructor
   TS3Session(const TUrl& url, EAuthType authType=kAmazon,
              const TString& accessKey="", const TString& secretKey="");
   virtual ~TS3Session();

   // Modifiers
   virtual TS3Request*    MakeRequest(const TString& verb, const TString& path, const TString& query="");
   TS3Session&            SetAccessKey(const TString& accessKey);
   TS3Session&            SetSecretKey(const TString& secretKey);
   TS3Session&            SetAuthType(EAuthType authType);

   // Selectors
   const TString&         GetAccessKey() const { return fAccessKey; }
   const TString&         GetSecretKey() const { return fSecretKey; }
   EAuthType              GetAuthType() const { return fAuthType; }

   ClassDef(TS3Session, 0)  // Persistent S3 session
};

#endif // ROOT_TS3Session
