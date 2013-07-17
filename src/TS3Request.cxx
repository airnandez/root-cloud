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
// TS3Request                                                           //
//                                                                      //
// A TS3Request is an HTTP request with specific behavior and fields to //
// interact with a server through the S3 protocol.                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TROOT.h"
#include "TError.h"
#include "TSystem.h"
#include "TS3Request.h"
#include "TS3Session.h"
#include "TBase64.h"
#if defined(MAC_OS_X_VERSION_10_7)
#include <CommonCrypto/CommonHMAC.h>
#define SHA_DIGEST_LENGTH 20
#else
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#endif


ClassImp(TS3Request)


//_____________________________________________________________________________
TS3Request::TS3Request(TS3Session* session, const TString& verb,
              const TString& path, const TString& query)
           : THttpRequest(session, verb, path, query)
{
}


//_____________________________________________________________________________
Bool_t TS3Request::PreSubmit()
{
   // Prepare this request before submitting it to the server

   // Set the date header. The value of the date header is required for
   // computing the signature of the request.
   SetDateHeader();

   // Sign the request
   const TS3Session* s3Session = (const TS3Session*)fSession;
   Sign(s3Session->GetAccessKey(), s3Session->GetSecretKey());

   // We are read for submission
   return kTRUE;
}


//_____________________________________________________________________________
void TS3Request::Sign(const TString& accessKey, const TString& secretKey)
{
   // Sign this request according to the S3 protocol specification.
   // Refs:
   //    AMAZON  http://awsdocs.s3.amazonaws.com/S3/latest/s3-qrc.pdf
   //    GOOGLE: http://code.google.com/apis/storage/docs/reference/v1/developer-guidev1.html#authentication

   if ((accessKey.Length() == 0) || (secretKey.Length() == 0))
      return;

   // Set the authentication headers
   SetAuthHeaders(accessKey, secretKey);
}


//_____________________________________________________________________________
void TS3Request::SetDateHeader()
{
   // Sets this request's "Date" header according to:
   //   http://code.google.com/apis/storage/docs/reference-headers.html#date

   time_t now = time(NULL);
   char result[128];
#ifdef _REENTRANT
   struct tm dateFormat;
   strftime(result, sizeof(result), "%a, %d %b %Y %H:%M:%S GMT",
      gmtime_r(&now, &dateFormat));
#else
   strftime(result, sizeof(result), "%a, %d %b %Y %H:%M:%S GMT",
      gmtime(&now));
#endif

   SetHeader("Date", result);
}


//______________________________________________________________________________
const char* TS3Request::GetAuthPrefix(Int_t authType) const
{
   // Returns the authentication prefix according to the type of authentication
   // of the S3 session this request is associated to

   switch (authType) {
      case TS3Session::kNoAuth: return "";
      case TS3Session::kGoogle: return "GOOG1";
      case TS3Session::kAmazon:
      default:                  return "AWS";
   }
}


//______________________________________________________________________________
void TS3Request::SetAuthHeaders(const TString& accessKey, const TString& secretKey)
{
   // Sets the authentication headers for this S3 request according to the
   // authentication type of the S3 session

   const TS3Session* s3Session = (const TS3Session*)fSession;
   TS3Session::EAuthType authType = s3Session->GetAuthType();
   if (authType == TS3Session::kNoAuth)
      return;

   // Compute the value of the "Authorization" header and set it for this
   // request
   TString authValue = TString::Format("%s %s:%s", GetAuthPrefix(authType),
      (const char*)accessKey, (const char*)ComputeSignature(secretKey));
   SetHeader("Authorization", authValue);

   if (authType == TS3Session::kGoogle)
      SetHeader("x-goog-api-version", "1");
}


//______________________________________________________________________________
TString TS3Request::ComputeSignature(const TString& secretKey) const
{
   // Compute the signature for this request.

   // Retrieve the value of the Date header for this request.
   const TString& date = fReqHeaderSet.GetValueForHeader("Date");

   // Please note, the order of the fields used for computing
   // the signature is important. Make sure that the changes you
   // make are compatible with the reference documentation.
   TString toSign = TString::Format("%s\n\n\n%s\n",  // empty Content-MD5 and Content-Type
                                    (const char*)fVerb,
                                    (const char*)date);

   const TS3Session* s3Session = (const TS3Session*)fSession;
   if (s3Session->GetAuthType() == TS3Session::kGoogle) {
      // Must use API version 1. Google Storage API v2 only
      // accepts OAuth authentication.
      // This header is not strictly needed but if used for computing
      // the signature, the request must contain it as a header
      // (see method MakeAuthHeader)
      // Ref: https://developers.google.com/storage/docs/reference/v1/apiversion1
      toSign += "x-goog-api-version:1\n"; // Lowercase, no spaces around ':'
   }

   toSign += fPath;

   if (gDebug > 2)
      Info("ComputeSignature", "toSign=\n'%s'", toSign.Data());

   unsigned char digest[SHA_DIGEST_LENGTH] = {0};
#if defined(MAC_OS_X_VERSION_10_7)
   CCHmac(kCCHmacAlgSHA1, secretKey.Data(), secretKey.Length() , (unsigned char *)toSign.Data(), toSign.Length(), digest);
#else
   unsigned int *sd = NULL;
   HMAC(EVP_sha1(), secretKey.Data(), secretKey.Length() , (unsigned char *)toSign.Data(), toSign.Length(), digest, sd);
#endif

   return TBase64::Encode((const char *)digest, SHA_DIGEST_LENGTH);
}

