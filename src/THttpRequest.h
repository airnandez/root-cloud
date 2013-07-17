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

#ifndef ROOT_THttpRequest
#define ROOT_THttpRequest

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// THttpRequest                                                         //
//                                                                      //
// A THttpRequest is a HTTP request with its associated response. It is //
// strongly coupled with class THttpSession. Both of them use libNeon.  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef ROOT_TUrl
#include "TUrl.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif

#ifndef ROOT_THttpHeaderSet
#include "THttpHeaderSet.h"
#endif

#ifdef __CINT__
// CINT fails compiling libNeon include files
typedef void* ne_request;
#else
#include "ne_request.h"
#endif

class THttpSession;


class THttpRequest: public TObject {

public:
   // Constants
   enum EHttpStatusCode {
      kStatusOK                   = 200,
      kStatusCreated              = 201,
      kStatusAccepted             = 202,
      kStatusNonAuthoritativeInfo = 203,
      kStatusNoContent            = 204,
      kStatusResetContent         = 205,
      kStatusPartialContent       = 206,

      kStatusMultipleChoices   = 300,
      kStatusMovedPermanently  = 301,
      kStatusFound             = 302,
      kStatusSeeOther          = 303,
      kStatusNotModified       = 304,
      kStatusUseProxy          = 305,
      kStatusTemporaryRedirect = 307,

      kStatusBadRequest                   = 400,
      kStatusUnauthorized                 = 401,
      kStatusPaymentRequired              = 402,
      kStatusForbidden                    = 403,
      kStatusNotFound                     = 404,
      kStatusMethodNotAllowed             = 405,
      kStatusNotAcceptable                = 406,
      kStatusProxyAuthRequired            = 407,
      kStatusRequestTimeout               = 408,
      kStatusConflict                     = 409,
      kStatusGone                         = 410,
      kStatusLengthRequired               = 411,
      kStatusPreconditionFailed           = 412,
      kStatusRequestEntityTooLarge        = 413,
      kStatusRequestURITooLong            = 414,
      kStatusUnsupportedMediaType         = 415,
      kStatusRequestedRangeNotSatisfiable = 416,
      kStatusExpectationFailed            = 417,
      kStatusTeapot                       = 418,

      kStatusInternalServerError     = 500,
      kStatusNotImplemented          = 501,
      kStatusBadGateway              = 502,
      kStatusServiceUnavailable      = 503,
      kStatusGatewayTimeout          = 504,
      kStatusHTTPVersionNotSupported = 505
   };

   enum EHttpStatusClass {
      kClassInformationalResponse  = 1,
      kClassSuccess                = 2,
      kClassRedirection            = 3,
      kClassClientError            = 4,
      kClassServerError            = 5
   };

private:
   // Helpers
   void RetreiveResponseHeaders();
   // Disallow default constructor
   THttpRequest();

protected:
   // Data members
   THttpSession*        fSession;       // Session this request will be submited through
   TString              fVerb;          // HTTP verb
   TString              fPath;          // Path
   TString              fQuery;         // Query
   Bool_t               fSubmitted;     // Was this request successfully sent to the server?
   THttpHeaderSet       fReqHeaderSet;  // Request's header set
   THttpHeaderSet       fRespHeaderSet; // Response's header set
   ne_request*          fRawRequest;    //! Underlying HTTP request (using libNeon)

   // Modifiers
   virtual Bool_t      PreSubmit();
   virtual void        PostSubmit();

public:
   // Constructors & Destructor
   THttpRequest(THttpSession* session, const TString& verb="GET",
                const TString& path="/", const TString& query="");
   virtual ~THttpRequest();

   // Selectors
   Bool_t              IsSubmitted() const { return fSubmitted; }
   const TString&      GetVerb() const { return fVerb; }
   const TString&      GetPath() const { return fPath; }
   TString             GetFullPath() const;
   const TString&      GetQuery() const { return fQuery; }
   const char*         GetError() const;
   virtual Int_t       GetResponseStatusCode() const;
   virtual Int_t       GetResponseStatusClass() const;
   const TString&      GetResponseHeader(const TString& header) const;
   Long64_t            GetResponseBodyLength() const;
   void                DumpResponseHeaders() const;
   void                DumpRequestHeaders() const;

   // Modifiers
   Bool_t              Submit();
   THttpRequest&       SetHeader(const TString& header, const TString& value);
   THttpRequest&       SetVerb(const TString& verb);
   THttpRequest&       SetPath(const TString& path);
   THttpRequest&       SetQuery(const TString& query);
   Long_t              GetResponseBody(char* buffer, Long_t length);

   ClassDef(THttpRequest, 0)  // HTTP request
};

#endif // ROOT_THttpRequest
