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
// TSwiftIdentityClient                                                 //
//                                                                      //
// A TSwiftIdentityClient is a client to the OpenStack Identity service.//
// It uses the REST API of the service to identify to the server using  //
// the end user credentials and to retrieve the storage URL and the     //
// storage token to use for subsequent requests to the object storage   //
// service.                                                             //
//////////////////////////////////////////////////////////////////////////

#include <time.h>

#include "TROOT.h"
#include "TError.h"
#include "TSystem.h"
#include "TSwiftIdentityClient.h"
#include "THttpRequest.h"
#include "THttpSession.h"
#include "TCloudExtension.h"

#include "rapidjson/document.h"

ClassImp(TSwiftIdentityClient)


//_____________________________________________________________________________
TSwiftIdentityClient::TSwiftIdentityClient(
   const TUrl& authUrl, const TString& tenantName,
   const TString& userName, const TString& password)
{
   // Create a client for retrieving the storage URL and storage token to
   // use for sending requests to the OpenStack object storage service
   // ports in the specified URL

   fAuthUrl = authUrl;
   fTenantName = tenantName;
   fUserName = userName;
   fPassword = password;
   ResetExpiration();
}


//_____________________________________________________________________________
TSwiftIdentityClient::~TSwiftIdentityClient()
{
}

//_____________________________________________________________________________
void TSwiftIdentityClient::ResetExpiration()
{
   // Set the token expiration 3 years from now. This expiration may be
   // reset according to the server's response
   fExpirationDate = ::time(0) + (3*365*24*3600);
}

//_____________________________________________________________________________
TSwiftIdentityClient& TSwiftIdentityClient::SetAuthUrl(const TUrl& authUrl)
{
   fAuthUrl = authUrl;
   return *this;
}


//_____________________________________________________________________________
TSwiftIdentityClient& TSwiftIdentityClient::SetTenantName(const TString& tenantName)
{
   fTenantName = tenantName;
   return *this;
}


//_____________________________________________________________________________
TSwiftIdentityClient& TSwiftIdentityClient::SetUserName(const TString& userName)
{
   fUserName = userName;
   return *this;
}


//_____________________________________________________________________________
TSwiftIdentityClient& TSwiftIdentityClient::SetPassword(const TString& password)
{
   fPassword = password;
   return *this;
}


//_____________________________________________________________________________
Bool_t TSwiftIdentityClient::IsAuthenticated() const
{
   // Return kTRUE if we have already retrieved the authentication token for
   // this session.

   // Check that the current storage token is still valid for a short time.
   // If this is not the case, we need to re-authenticate to retrieve a newer
   // one.
   const Int_t kGracePeriod = 5 * 60; // Seconds
   ::time_t now = ::time(0);
   if ((now + kGracePeriod) >= fExpirationDate) {
      return kFALSE;
   }

   return (fStorageToken.Length() > 0) ? kTRUE : kFALSE;
}


//_____________________________________________________________________________
void TSwiftIdentityClient::ResetAuthentication()
{
   fStorageUrl = "";
   fStorageToken = "";
   ResetExpiration();
}


//_____________________________________________________________________________
Bool_t TSwiftIdentityClient::Authenticate()
{
   // Contacts the identity server to retrieve the storage token and storage
   // URL.

   // If already authenticated, reset the auth token and storage URLs
   ResetAuthentication();

   // Determine the version of the authentication API from the authentication
   // URL
   Bool_t authenticated = kFALSE;
   TString authPath = fAuthUrl.GetFile();

   if (authPath.BeginsWith("v2.0")) {
      authenticated = AuthenticateV2();
   } else if (authPath.BeginsWith("v3")) {
      authenticated = AuthenticateV3();
   } else if (authPath.BeginsWith("v1.0") || authPath.BeginsWith("auth/v1.0")) {
      authenticated = AuthenticateV1();
   }

   if (!authenticated) {
      Error("Authenticate", "error retrieving authentication token from Swift server");
      Error("Authenticate", "make the authentication URL and your user name and API access key are all correct");
      return kFALSE;
   }

   if (TCloudExtension::fgDebugLevel > 0) {
      Info("Authenticate", "storage URL:      '%s'", fStorageUrl.GetUrl());
      Info("Authenticate", "storage token:    '%s'", fStorageToken.Data());
      Info("Authenticate", "token expiration: '%s'", GetExpirationDateStr().Data());
   }

   return kTRUE;
}

//_____________________________________________________________________________
Bool_t TSwiftIdentityClient::AuthenticateV1()
{
   // Uses OpenStack Identity API v1 to authenticate and retrieve the
   // URL of the storage server and the authentication token to use
   // with subsequent storage requests

   // TODO: test this routine against a V1 authentication service

   // Create a session to the authentication server
   THttpSession* authSession = new THttpSession(fAuthUrl);
   if (authSession == 0) {
      Error("AuthenticateV1", "could not allocate memory for creating HTTP session");
      return kFALSE;
   }

   // Try these paths for authentication. Different instances of Swift use
   // different authentication paths
   const char* authPaths[] = {
      "/auth/v1.0",
      "/v1.0",
   };
   Bool_t authenticated = kFALSE;
   const Int_t kNumPaths = sizeof(authPaths)/sizeof(authPaths[0]);
   for (Int_t i=0; i < kNumPaths; i++) {
      // Try next authentication path
      TString path(authPaths[i]);

      // Create a new request, set the required headers for authenticationg
      // with the Swift server and submit it
      THttpRequest* request = new THttpRequest(authSession, "GET", path);
      request->SetHeader("X-Auth-User", fUserName);
      request->SetHeader("X-Auth-Key", fPassword);
      if (!request->Submit()) {
         // Could not submit the request
         delete request;
         continue;
      }

      if (request->GetResponseStatusClass() != THttpRequest::kClassSuccess) {
         // The request was submitted but did not succeed
         delete request;
         continue;
      }

      // We got a successful response to our authentication request. Extract
      // the relevant information.
      TString xsu = request->GetResponseHeader("X-Storage-Url");
      fStorageToken = request->GetResponseHeader("X-Storage-Token");
      if (xsu.IsNull() || fStorageToken.IsNull()) {
         delete request;
         continue;
      }

      fStorageUrl = TUrl(xsu.Strip(TString::kTrailing, '/').Data());
      authenticated = kTRUE;
      delete request;
      break;
   }

   delete authSession;
   return authenticated;
}

//_____________________________________________________________________________
Bool_t TSwiftIdentityClient::AuthenticateV2()
{
   // Uses OpenStack Identity API v2 to retrieve the
   // URL of the storage server and the authentication token to use
   // with subsequent storage requests

   // Build the authentication request
   TString authPath = TString(fAuthUrl.GetFile());
   if (!authPath.EndsWith("/")) {
      authPath += "/";
   }
   authPath += "tokens";

   const char* bodyFormat =
   "{ \"auth\": { \"tenantName\": \"%s\", \"passwordCredentials\": { \"username\": \"%s\",  \"password\": \"%s\" } } }";
   char* requestBody;
   int bodyLength = ::asprintf(&requestBody, bodyFormat, fTenantName.Data(), fUserName.Data(), fPassword.Data());
   THttpSession* authSession = new THttpSession(fAuthUrl);
   if (authSession == 0) {
      Error("AuthenticateV2", "could not allocate memory for creating HTTP session");
      return kFALSE;
   }
   THttpRequest* authRequest = new THttpRequest(authSession, "POST", authPath);
   if (authRequest == 0) {
      Error("AuthenticateV2", "could not allocate memory for creating HTTP authentication request");
      return kFALSE;
   }
   authRequest->SetBody(requestBody, bodyLength);
   authRequest->SetHeader("Content-Type", "application/json");
   authRequest->SetHeader("Accept", "application/json");
   if (TCloudExtension::fgDebugLevel > 0) {
      Info("AuthenticateV2", "request body: '%s'", requestBody);
   }

   // Send the request
   if (!authRequest->Submit()) {
      // Could not submit the request
      ::free(requestBody);
      delete authRequest;
      delete authSession;
      return kFALSE;
   }

   ::free(requestBody);
   if (authRequest->GetResponseStatusClass() != THttpRequest::kClassSuccess) {
      // The request was submitted but did not succeed
      delete authRequest;
      delete authSession;
      return kFALSE;
   }

   // We got a response. Check we got a response in JSON format
   if (authRequest->GetResponseHeader("Content-Type").CompareTo("application/json") != 0) {
      delete authRequest;
      delete authSession;
      return kFALSE;
   }

   // Parse the response body
   Bool_t authenticated = kFALSE;
   Long_t responseBodyLength = authRequest->GetResponseBodyLength();
   if (responseBodyLength > 0) {
      char* responseBody = new char[responseBodyLength + 1];
      if (responseBody != 0) {
         responseBody[responseBodyLength] = '\0';
         authRequest->GetResponseBody(responseBody, responseBodyLength);
         // Info("AuthenticateV2", "response body: '%s'", responseBody);

         // Parse the JSON response body
         authenticated = ParseResponseV2(responseBody, fStorageUrl, fStorageToken, fExpirationDate);
         delete [] responseBody;
      }
   }

   // Clean up
   delete authRequest;
   delete authSession;

   return authenticated;
}


//_____________________________________________________________________________
Bool_t TSwiftIdentityClient::AuthenticateV3()
{
   // Not implemented yet
   fStorageUrl = TUrl();
   fStorageToken = "";
   return kFALSE;
}


//_____________________________________________________________________________
Bool_t TSwiftIdentityClient::ParseResponseV2(const char* jsonResponse,
   TUrl& storageUrl, TString& storageToken, ::time_t& expirationDate)
{
   // Parse the JSON document sent by the identity service as a response for
   // an authentication request

   // Make sure the response is a valid JSON document
   rapidjson::Document doc;
   doc.Parse(jsonResponse);
   if (!doc.IsObject()) {
      Error("ParseResponseV2", "received malformed JSON document '%s'", jsonResponse);
      return kFALSE;
   }

   // Retrieve the 'access' dictionary
   if (!doc.HasMember("access")) {
      Error("ParseResponseV2", "JSON response does not have 'access' dictionary");
      return kFALSE;
   }

   // Retrieve the 'serviceCatalog' array within 'access' dictionary
   const rapidjson::Value& access = doc["access"];
   if (!access.HasMember("serviceCatalog")) {
      Error("ParseResponseV2", "JSON response does not have 'serviceCatalog' array");
      return kFALSE;
   }

   // Make sure 'serviceCatalog' is an array
   const rapidjson::Value& catalog = access["serviceCatalog"];
   if (!catalog.IsArray()) {
      Error("ParseResponseV2", "'serviceCatalog' array could not be found");
      return kFALSE;
   }

   // Scan the 'serviceCatalog' array looking for a service of type 'object-store'
   // and named 'swift'
   TString serviceUrl;
   for (rapidjson::SizeType i = 0; i < catalog.Size(); i++) {
      if (catalog[i].HasMember("name") && catalog[i].HasMember("type")) {
         if (catalog[i]["name"] == "swift" && catalog[i]["type"] == "object-store") {
            // Found Swift service. Retrieve the first endpoint
            if (catalog[i].HasMember("endpoints")) {
               const rapidjson::Value& endpoints = catalog[i]["endpoints"];
               for (rapidjson::SizeType i = 0; i < endpoints.Size(); i++) {
                  if (endpoints[i].HasMember("publicURL")) {
                     serviceUrl = endpoints[i]["publicURL"].GetString();
                     break;
                  }
               }
            }
         }
      }
   }

   // Did we find the service URL?
   if (serviceUrl.IsNull()) {
      Error("ParseResponseV2", "could not find storage URL in response from identity service");
      return kFALSE;
   }

   // Retrieve the token
   if (!access.HasMember("token")) {
      Error("ParseResponseV2", "could not find storage token in response from identity service");
      return kFALSE;
   }

   // Retrieve the token id and expiration timestamp
   TString expirationTimeStamp;
   const rapidjson::Value& token = access["token"];
   if (token.HasMember("expires")) {
      expirationTimeStamp = token["expires"].GetString();
      ParseDateV2(expirationTimeStamp, expirationDate);
   }
   if (!token.HasMember("id")) {
      Error("ParseResponseV2", "could not find token id in response from identity service");
      return kFALSE;
   }
   storageToken = token["id"].GetString();

   // Clean the storage URL and return
   TString urlStr = TString(serviceUrl).Strip(TString::kTrailing, '/');
   storageUrl = TUrl(urlStr.Data());
   return kTRUE;
}


//_____________________________________________________________________________
Bool_t TSwiftIdentityClient::ParseDateV2(const TString& date, ::time_t& utc)
{
   // Parse a date as returned by identity service API v2. Dates are
   // expected in UTC time.

   // Argument date is of one of the two forms:
   //    "2014-10-24T13:48:28.095790"
   //    "2014-10-25T13:48:28Z"
   struct tm utcTimeStamp;
   char* last = strptime(date.Data(), "%FT%T", &utcTimeStamp);
   if ((last != NULL) && (*last != '.') && (*last != 'Z')) {
      Error("ParseDateV2", "unexpected format for timestamp '%s'", date.Data());
      return kFALSE;
   }

   // Parsed successfully. Convert to UTC time
   utc = timegm(&utcTimeStamp);
   return kTRUE;
}


//_____________________________________________________________________________
TString TSwiftIdentityClient::GetExpirationDateStr() const
{
   struct tm utc;
   ::gmtime_r(&fExpirationDate, &utc);
   char buffer[32];
   ::strftime(buffer, sizeof(buffer), "%FT%TZ", &utc);
   return TString(buffer);
}

