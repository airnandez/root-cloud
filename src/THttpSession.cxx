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
// THttpSession                                                         //
//                                                                      //
// A THttpSession represents a persisten network connection with an     //
// HTTP server. It is modeled on top of libNeon.                        //
//////////////////////////////////////////////////////////////////////////


#include "TROOT.h"
#include "TError.h"
#include "TSystem.h"
#include "THttpSession.h"
#include "THttpRequest.h"
#include "TCloudExtension.h"

// libNeon include files
#include "ne_request.h"

// Declaration of static members and variables
static const char *gUserAgent = "ROOT-THttpSession";

ClassImp(THttpSession)


//_____________________________________________________________________________
static int VerifyServerCertCallback(void*, int failures, const ne_ssl_certificate *cert)
{
   // Callback to verify the server certificate in case of an HTTP session.

   if (TCloudExtension::fgDebugLevel > 1)
      Info("VerifyServerCertCallback", "Verifying certificate for server '%s'",
         ne_ssl_cert_identity(cert));

   // In MacOS X, the system-level trusted certificates are kept in Keychain.
   // libNeon cannot find them so it says the certificate is not trusted.
   // We trust it anyway, which is equivalent to not validate the server
   // certificate.
   if (failures & NE_SSL_UNTRUSTED) {
      // Only in this case we trust the server certificate
      return 0;
   }

   // In any other case, don't trust this server certificate:
   //
   // NE_SSL_NOTYETVALID
   // NE_SSL_IDMISMATCH
   // NE_SSL_BADCHAIN
   // NE_SSL_REVOKED
   // NE_SSL_EXPIRED
   return 1;
}


extern Bool_t InitLibNeon();

//_____________________________________________________________________________
THttpSession::THttpSession()
   : fRawSession(0)
{
   // Create a HTTP session. Initialize the underlying HTTP handling library.

   if (!InitLibNeon()) {
      Error("THttpSession", "could not initialize libNeon");
      return;
   }
}


//_____________________________________________________________________________
THttpSession::THttpSession(const TUrl& url)
   : fRawSession(0)
{
   // Create a HTTP session. Initialize the underlying HTTP handling library.

   if (!InitLibNeon()) {
      Error("THttpSession", "could not initialize libNeon");
      return;
   }

   // Initialize the URL of the server with which we will create an HTTP(S)
   // session
   SetServerUrl(url);
}


//_____________________________________________________________________________
THttpSession::~THttpSession()
{
   Terminate();
}


//_____________________________________________________________________________
void THttpSession::Terminate()
{
   if (fRawSession != 0) {
      ne_close_connection(fRawSession);
      ne_session_destroy(fRawSession);
      fRawSession = 0;
   }
}


//_____________________________________________________________________________
void THttpSession::SetServerUrl(const TUrl& url)
{
   // Set the remote HTTP server URL. Supported protocols are HTTP and HTTPS.
   // HTTP is enforced if a different protocol is specified in the provided URL.

   // In case there is already an open connection to the server and the new
   // server URL implies a connection to the same host, port and scheme, we
   // don't need to reset the connection.
   if (fRawSession != 0) {
      if ((strcmp(fServerUrl.GetProtocol(), url.GetProtocol()) == 0) &&
          (strcmp(fServerUrl.GetHost(), url.GetHost()) == 0) &&
          (fServerUrl.GetPort() == url.GetPort())) {
         // Store the value of the new URL: even if the scheme, host and port
         // are the same, the path may not be the same and the path is needed
         // by some subclasses (TSwiftSession for instance.)
         fServerUrl = url;
         return;
      }
   }

   fServerUrl = url;
   const char* scheme = fServerUrl.GetProtocol();
   if ((strcmp(scheme, "http") != 0) && (strcmp(scheme, "https") != 0)) {
      scheme = "http";
      fServerUrl.SetProtocol(scheme);
   }
   const char* hostName = fServerUrl.GetHost();
   unsigned int port = fServerUrl.GetPort();
   if (TCloudExtension::fgDebugLevel > 0)
      Info("THttpSession", "creating HTTP session for %s://%s:%d", scheme,
         hostName, port);

   // Terminate the current session and create a new one
   Terminate();
   fRawSession = ne_session_create(scheme, hostName, port);

   // Set the user agent and the connect timeout
   ne_set_useragent(fRawSession, gUserAgent);
   ne_set_connect_timeout(fRawSession, 5); // in seconds TODO: is that enough?

   // Set the callback for verifying the server certificate
   if (strcmp(scheme, "https") == 0) {
      ne_ssl_set_verify(fRawSession, VerifyServerCertCallback, 0);
   }
}


//_____________________________________________________________________________
Bool_t THttpSession::SetProxyUrl(const TUrl& proxyUrl)
{
   // Set the proxy to be used by this session. The proxy must use the HTTP
   // protocol, so its scheme must be 'http'.

   if (!proxyUrl.IsValid())
      return kFALSE;

   if (strcmp(proxyUrl.GetProtocol(), "http") != 0) {
      ::Error("THttpSession::SetProxy", "protocol must be HTTP in proxy URL %s",
         TUrl(proxyUrl).GetUrl());
      return kFALSE;
   }

   fProxyUrl = proxyUrl;
   ne_session_proxy(fRawSession, fProxyUrl.GetHost(), fProxyUrl.GetPort());
   if (TCloudExtension::fgDebugLevel > 0)
      Info("SetProxyUrl", "setting proxy for this session to %s", fProxyUrl.GetUrl());
   return kTRUE;
}


//_____________________________________________________________________________
const TUrl& THttpSession::GetProxyUrl() const
{
   // Returns the proxy URL used by this session, if any. May return an invalid
   // URL if there is no proxy in use. Use the TUrl::IsValid() to determine if
   // this is a valid proxy.

   return fProxyUrl;
}


//_____________________________________________________________________________
const TUrl& THttpSession::GetServerUrl() const
{
   // Returns the server URL used by this session, if any. May return an invalid
   // URL if the server URL is not yet set. Use the TUrl::IsValid() to determine
   // if this is a valid URL.

   return fServerUrl;
}


//_____________________________________________________________________________
TString THttpSession::GetServerHost() const
{
   // Return the name of the host this session is opened with.

   if (!fServerUrl.IsValid())
      return "";

   return fServerUrl.GetHost();
}


//_____________________________________________________________________________
TString THttpSession::GetServerHostAndPort() const
{
   // Return the name of the host and the port number this session is opened
   // with, using the format host:port

   if (!fServerUrl.IsValid())
      return "<none>:<0000>";

   return TString::Format("%s:%d", fServerUrl.GetHost(), fServerUrl.GetPort());
}



//_____________________________________________________________________________
Int_t THttpSession::GetServerPort() const
{
   // Return the port number used by the HTTP server.

   if (!fServerUrl.IsValid())
      return 0;

   return fServerUrl.GetPort();
}


//_____________________________________________________________________________
TString THttpSession::GetScheme() const
{
   // Return the scheme of this connection, i.e. 'http' or 'https'

   if (!fServerUrl.IsValid())
      return "";

   return fServerUrl.GetProtocol();
}


//_____________________________________________________________________________
TString THttpSession::GetError() const
{
   // Return the last error recorded by this session

   return TString(ne_get_error(fRawSession));
}


//_____________________________________________________________________________
THttpRequest* THttpSession::MakeRequest(const TString& verb, const TString& path,
   const TString& query)
{
   // Build a new request to be used by this session.
   // Returns a new request created in the heap. It is the responsibility of
   // the caller to delete the returned request.
   // WARNING: there is strong coupling between the HTTP session object through
   // which the request is created and the request itself. You should not
   // destroy a session object if there are pending requests, as the result
   // is undefined and may crash your program. You must first delete the
   // requests and then the session.

   // Build an HTTP request object
   return new THttpRequest(this, verb, path, query);
}


//_____________________________________________________________________________
void* THttpSession::MakeRawRequest(const TString& verb, const TString& path, const TString& query)
{
   // Make a raw Neon request which will use this session. This method is
   // intended to be called by THttpRequest and subclasses.

   TString fullPath(path);
   if (query.Length() > 0)
      fullPath += "?" + query;

   ne_request* rawRequest = ne_request_create(fRawSession, verb.Data(), fullPath.Data());
   return (void*)rawRequest;
}


//_____________________________________________________________________________
Bool_t THttpSession::CanUseHttps() const
{
   // Can this session use HTTPS? The return value depends on whether libNeon
   // was compiled with support for SSL.

   return ne_has_support(NE_FEATURE_SSL) ? kTRUE : kFALSE;
}


