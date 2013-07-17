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

#ifndef ROOT_THttpSession
#define ROOT_THttpSession

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// THttpSession                                                         //
//                                                                      //
// A THttpSession represents a persisten network connection with an     //
// HTTP server. It is modeled on top of libNeon.                        //
//////////////////////////////////////////////////////////////////////////


#ifndef ROOT_TUrl
#include "TUrl.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif

#ifdef __CINT__
// CINT fails compiling libNeon include files
typedef void* ne_session;
#else
#include "ne_session.h"
#endif


class THttpRequest;

class THttpSession: public TObject {

protected:
   // Data members
   TUrl         fServerUrl;   // Session URL
   TUrl         fProxyUrl;    // Proxy URL used by this session
   ne_session*  fRawSession;  //! Underlying HTTP session (using libNeon)

   // Helpers
   friend class THttpRequest;
   void*        MakeRawRequest(const TString& verb, const TString& path, const TString& query);

public:
   // Constructors & Destructor
   THttpSession();
   THttpSession(const TUrl& url);
   virtual ~THttpSession();

   // Modifiers
   virtual THttpRequest*  MakeRequest(const TString& verb="GET", const TString& path="/", const TString& query="");
   void                   SetServerUrl(const TUrl& url);
   Bool_t                 SetProxyUrl(const TUrl& url);
   void                   Terminate();

   // Selectors
   const TUrl& GetProxyUrl() const;
   const TUrl& GetServerUrl() const;
   TString     GetServerHost() const;
   Int_t       GetServerPort() const;
   TString     GetServerHostAndPort() const;
   TString     GetScheme() const;
   TString     GetError() const;
   Bool_t      CanUseHttps() const;

   ClassDef(THttpSession, 0)  // Persistent HTTP session
};

#endif // ROOT_THttpSession
