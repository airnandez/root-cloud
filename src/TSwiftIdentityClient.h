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

#ifndef ROOT_TSwiftIdentityClient
#define ROOT_TSwiftIdentityClient

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TSwiftIdentityClient                                                 //
//                                                                      //
// A TSwiftIdentityClient is a client to the OpenStack Identity service.//
// It uses the REST API of the service to identify to the server using  //
// the end user credentials and to retrieve the storage URL and the     //
// storage token to use for subsequent requests to the object storage   //
// service.                                                             //
//////////////////////////////////////////////////////////////////////////

#include <time.h>


#ifndef ROOT_TUrl
#include "TUrl.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif


class TSwiftIdentityClient: public TObject {


private:
   // Data members
   TString      fTenantName;   // Tenant name
   TString      fUserName;     // User name
   TString      fPassword;     // Password
   TString      fStorageToken; // Storage token
   TUrl         fAuthUrl;      // Authentication URL
   TUrl         fStorageUrl;   // Storage URL
   ::time_t     fExpirationDate; // Expiration date for storage token

   TSwiftIdentityClient();            // Avoid automatic generation of default constructor

   // Helpers
   void          ResetAuthentication();
   Bool_t        AuthenticateV1();
   Bool_t        AuthenticateV2();
   Bool_t        AuthenticateV3();
   Bool_t        ParseResponseV2(const char* jsonResponse, TUrl& storageUrl,
      TString& storageToken, ::time_t& expirationDate);
   Bool_t        ParseDateV2(const TString& date, ::time_t& utc);
   void          ResetExpiration();

public:
   // Constructors & Destructor
   TSwiftIdentityClient(const TUrl& authUrl, const TString& tenantName,
      const TString& userName, const TString& password);
   virtual ~TSwiftIdentityClient();

   // Modifiers
   TSwiftIdentityClient&     SetAuthUrl(const TUrl& authUrl);
   TSwiftIdentityClient&     SetTenantName(const TString& tenantName);
   TSwiftIdentityClient&     SetUserName(const TString& userName);
   TSwiftIdentityClient&     SetPassword(const TString& password);
   Bool_t                    Authenticate();

   // Selectors
   const TString&  GetTenantName() const { return fTenantName; }
   const TString&  GetUserName() const { return fUserName; }
   const TString&  GetPassword() const { return fPassword; }
   const TUrl&     GetAuthUrl() const { return fAuthUrl; }
   const TUrl&     GetStorageUrl() const { return fStorageUrl; }
   const TString&  GetStorageToken() const { return fStorageToken; }
   Bool_t          IsAuthenticated() const;
   TString         GetExpirationDateStr() const;

   ClassDef(TSwiftIdentityClient, 0)  // Client to the OpenStack Identity service
};

#endif // ROOT_TSwiftIdentityClient
