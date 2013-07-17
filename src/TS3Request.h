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

#ifndef ROOT_TS3Request
#define ROOT_TS3Request

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TS3Request                                                           //
//                                                                      //
// A TS3Request is an HTTP request with specific behavior and fields to //
// interact with a server through the S3 protocol.                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef ROOT_TString
#include "TString.h"
#endif

#ifndef ROOT_THttpRequest
#include "THttpRequest.h"
#endif

class TS3Session;


class TS3Request: public THttpRequest {

private:
   // Helpers
   void Sign(const TString& accessKey, const TString& secretKey);
   void SetDateHeader();
   const char* GetAuthPrefix(Int_t authType) const;
   void SetAuthHeaders(const TString& accessKey, const TString& secretKey);
   TString ComputeSignature(const TString& secretKey) const;

protected:
   // Modifiers
   virtual Bool_t PreSubmit();

public:
   // Constructors & Destructor
   TS3Request(TS3Session* session, const TString& verb="GET",
              const TString& path="/", const TString& query="");
   virtual ~TS3Request() {};

   ClassDef(TS3Request, 0)  // S3 request
};

#endif // ROOT_TS3Request
