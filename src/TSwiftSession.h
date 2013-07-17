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

#ifndef ROOT_TSwiftSession
#define ROOT_TSwiftSession

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TSwiftSession                                                        //
//                                                                      //
// A TSwiftSession is a persistent connection with a Swift server though//
// which we can send requests for retrieving file contents.             //
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

#ifndef ROOT_THttpRequest
#include "THttpRequest.h"
#endif


class TSwiftSession: public THttpSession {


private:
   // Data members
   TString      fUserName;     // Access key id
   TString      fApiAccessKey; // Secret key id
   TUrl         fAuthUrl;      // Authentication URL
   TUrl         fStorageUrl;   // Storage URL for this session
   TString      fStorageToken; // Storage token for this session

   TSwiftSession();            // Avoid automatic generation of default constructor

   // Helpers
   void          ResetAuthentication();
   Bool_t        IsAuthenticated() const;
   Bool_t        Authenticate();

public:
   // Constructors & Destructor
   TSwiftSession(const TUrl& authUrl, const TString& userName="", const TString& apiAccessKey="");
   virtual ~TSwiftSession();

   // Modifiers
   virtual THttpRequest*     MakeRequest(const TString& verb, const TString& path, const TString& query="");
   TSwiftSession&            SetUserName(const TString& userName);
   TSwiftSession&            SetApiAccessKey(const TString& apiAccessKey);

   // Selectors
   const TString&         GetUserName() const { return fUserName; }
   const TString&         GetApiAccessKey() const { return fApiAccessKey; }

   ClassDef(TSwiftSession, 0)  // Persistent session with an OpenStack Swift service
};

#endif // ROOT_TSwiftSession
