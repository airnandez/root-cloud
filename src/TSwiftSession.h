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
// A TSwiftSession is a persistent connection with a Swift object       //
// storage server to send requests for retrieving file contents.        //
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

#ifndef ROOT_TSwiftIdentityClient
#include "TSwiftIdentityClient.h"
#endif


class TSwiftSession: public THttpSession {

private:
   // Data members
   TSwiftIdentityClient* fIdentityClient;

   TSwiftSession();            // Avoid automatic generation of default constructor

public:
   // Constructors & Destructor
   TSwiftSession(TSwiftIdentityClient* identityClient);
   virtual ~TSwiftSession();

   // Modifiers
   virtual THttpRequest*     MakeRequest(const TString& verb, const TString& path, const TString& query="");

   // Selectors

   ClassDef(TSwiftSession, 0)  // Persistent session with an OpenStack Swift service
};

#endif // ROOT_TSwiftSession
