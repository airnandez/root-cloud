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
// THttpFile                                                            //
//                                                                      //
// A THttpFile is a read-only TFile which retrieves the file contents   //
// from a server via HTTP. It is an alternative implementation to the   //
// built-in class TWebFile.                                             //
// It uses libNeon [http://www.webdav.org/neon/] for interacting with   //
// the HTTP server.                                                     //
// In addition, this class is the base class for other classes that use //
// HTTP-based protocols for reading remote files.                       //
// This class is intended to be used for reading files via HTTP and     //
// HTTPS, for instance: "http://server.domain.com/path/to/file.root"    //
// An HTTP proxy can be used for this file (see method SetProxy) or     //
// for all HTTP(S) files by setting the value of the environmental      //
// variable 'http_proxy' to the URL of the HTTP proxy server. For       //
// you can set this variable to 'http://myproxy.domain.com:8080' and    //
// all the HTTP requests for retrieving the file contents will be routed//
// through this proxy server.                                           //
// If the proxy server enviromental variable is set, it will be used    //
// with all files. If for a particular file you need to bypass the proxy//
// set the 'options' argument to the constructor to the value "NOPROXY".//
//
// The recommended usage of this class is via TFile::Open, for instance://
// TFile* f1 = TFile::Open("http://server.domain.com/path/to/file.root")//                                                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TROOT.h"
#include "TError.h"
#include "TSystem.h"
#include "THttpFile.h"
#include "TVirtualPerfStats.h"
#include "TTimeStamp.h"
#include "TPRegexp.h"


#include "THttpSession.h"
#include "THttpRequest.h"
#include "TCloudExtension.h"


// Declaration of static members and variables
TUrl THttpFile::fgProxyUrl;


ClassImp(THttpFile)

//_____________________________________________________________________________
THttpFile::THttpFile(const char* url, Option_t* options, const char* ftitle, Int_t compress)
          : TFile(url, "WEB", ftitle, compress), fHttpSession(0), fSize(0)

{
   // Construct THttpFile object. The 'url' argument is the URL where the file
   // is located and is expected to be of the form
   //     "http://server.domain.com/path/to/file.root"
   // Use the string "NOPROXY" in the options to bypass the HTTP proxy (if any).

   // If the sub-class specified "NOINIT" in the options, delay the initialization.
   // The sub-class will explicitedly call the Initialize() method later.
   TString optionsStr(options);
   if (optionsStr.Contains("NOINIT", TString::kIgnoreCase))
      return;

   // Do initialize this object
   if (!Initialize(TUrl(url), options)) {
      MakeZombie();
      gDirectory = gROOT;
   }
}


//_____________________________________________________________________________
THttpFile::~THttpFile()
{
   if (TCloudExtension::fgDebugLevel > 0)
      Info("~THttpFile", "destroying file '%s'", fUrl.GetUrl());

   Close();
}


//_____________________________________________________________________________
void THttpFile::TerminateSession()
{
   // Terminate the underlying HTTP session being used by this file

   if (TCloudExtension::fgDebugLevel > 0)
      Info("TerminateSession", "terminating HTTP session with host '%s'", fUrl.GetHost());

   if (fHttpSession != 0) {
      delete fHttpSession;
      fHttpSession = 0;
   }
}


//_____________________________________________________________________________
void THttpFile::Close(Option_t* option)
{
   // Close this file

   if (TCloudExtension::fgDebugLevel > 0)
      Info("Close", "closing file '%s'", fUrl.GetUrl());

   // Terminate this HTTP session
   TerminateSession();

   // Call the superclass' Close()
   TFile::Close(option);
}


//_____________________________________________________________________________
Bool_t THttpFile::Initialize(const TUrl& url, Option_t* options)
{
   // Initialize this object. This method is intended to be called by the
   // sub-classes when they are ready to initialize the super-class data
   // members.

   if (TCloudExtension::fgDebugLevel > 0)
      Info("Initialize", "initializing HTTP file %s", TUrl(url).GetUrl());

   // Init data members
   fHttpSession = 0;
   fSize = 0;
   fUrl = url;

   // Make sure the protocol is either HTTP or HTTPS
   TString protocol = fUrl.GetProtocol();
   if (!(protocol.CompareTo("http", TString::kIgnoreCase) == 0) &&
       !(protocol.CompareTo("https", TString::kIgnoreCase) == 0)) {
      Error("Initialize", "'%s' is an unsupported protocol", (const char*)protocol);
      return kFALSE;
   }

   if (fUrl.GetPort() == 0) {
      // ROOT v5.24 does not reset the port number after SetUrl()
      fUrl.SetPort((protocol.CompareTo("http") == 0) ? 80 : 443);
   }

   // Create a HTTP session for retrieving this file
   fHttpSession = MakeSession(fUrl);
   if (fHttpSession == 0) {
      Error("Initialize", "cannot create HTTP session with server %s", fUrl.GetUrl());
      return kFALSE;
   }

   // Make sure libNeon was compiled with support for SSL, so we can use SSL
   // for this session
   if ((protocol.CompareTo("https") == 0) && !fHttpSession->CanUseHttps()) {
      Error("Initialize", "cannot use HTTPS since libNeon was not compiled with support for SSL");
      return kFALSE;
   }

   // Inititalize the HTTP proxy for this file, unless the user explicitely
   // requested not to do so
   TString optStr(options);
   if (!optStr.Contains("NOPROXY", TString::kIgnoreCase)) {
      SetSessionProxy();
   }

   // Send a HTTP HEAD request to retrieve the size of the file
   if (!RetrieveFileSize()) {
      return kFALSE;
   }

   // Initialize the offset of this file
   SetOffset(0);

   // Set the file descriptor so TFile::IsOpen() will return true when in
   // TFile::~TFile (from TWebFile.cxx)
   TFile::Init(kFALSE);
   fD = -2;
   return kTRUE;
}


//_____________________________________________________________________________
THttpSession* THttpFile::MakeSession(const TUrl& fileUrl)
{
   // Make a session for sending HTTP requests for retrieving this file. This
   // method is intended to be overwritten by the subclasses to specialize the
   // behavior of the session and the requests it creates.

   return new THttpSession(fileUrl);
}


//_____________________________________________________________________________
Bool_t THttpFile::RetrieveFileSize()
{
   // Retrieve the remote file size (in bytes). It sends a HTTP HEAD request
   // and inspects the HTTP response headers.

   if (TCloudExtension::fgDebugLevel > 0)
      Info("RetrieveFileSize", "retrieving size of file at %s", fUrl.GetUrl());

   // Build and send an HTTP HEAD request
   TString path = TString::Format("/%s", (const char*)fUrl.GetFile());
   THttpRequest* request = fHttpSession->MakeRequest("HEAD", path, fUrl.GetOptions());
   if (request == 0) {
      // Just return, error message is displayed by the session
      return kFALSE;
   }

   if (!request->Submit()) {
      Error("RetrieveFileSize", "failed to retrieve file size [%s]",
         request->GetError());
      delete request;
      return kFALSE;
   }

   // Analyze the response
   if (request->GetResponseStatusCode() != THttpRequest::kStatusOK) {
      Error("RetrieveFileSize", "failed to retrieve file size [%s]",
         request->GetError());
      delete request;
      return kFALSE;
   }

   // Retrieve and convert the Content-Length header
   Long_t size = request->GetResponseBodyLength();
   if (size < 0) {
      Error("RetrieveFileSize",  "failed to retrieve Content-Length header [%s]",
         request->GetError());
      delete request;
      return kFALSE;
   }

   // Save this file size
   fSize = size;

   if (TCloudExtension::fgDebugLevel > 0) {
      Info("RetrieveFileSize", "file size is %lld", fSize);
   }

   // We are done
   delete request;
   return kTRUE;
}


//_____________________________________________________________________________
Bool_t THttpFile::ReadBuffer(char *buffer, Int_t length)
{
   // Read length bytes into buffer from the remote file. Returns kTRUE in
   // case of failure, so to be compliant with built-in TWebFile class.

   if (TCloudExtension::fgDebugLevel > 0)
      Info("ReadBuffer", "THttpFile::ReadBuffer(char *buffer, %d) starting", length);

   if (length <= 0) {
      // Nothing to do
      return kFALSE;
   }

   if (length > (fSize - fOffset)) {
      // There are no enough remaining bytes in the files to read. This is an
      // error.
      return kTRUE;
   }

   // First search for this buffer in the cache
   Int_t inCache = ReadBufferViaCache(buffer, length);
   if (inCache != 0) {
      // The requested buffer is in the cache
      return (inCache == 1) ? kFALSE : kTRUE;
   }

   Double_t startTime = 0;
   if (gPerfStats)
      startTime = TTimeStamp();

   // Create a HTTP GET request and set its Range header
   TString path = TString::Format("/%s", (const char*)fUrl.GetFile());
   THttpRequest* request = fHttpSession->MakeRequest("GET", path, fUrl.GetOptions());

   TString rangeValue = TString::Format("bytes=%lld-%lld", fOffset, fOffset+length-1);
   request->SetHeader("Range", rangeValue);
   if (TCloudExtension::fgDebugLevel > 0)
      Info("ReadBuffer", "asking for range %s", rangeValue.Data());

   // Submit this request
   if (!request->Submit()) {
      Error("ReadBuffer", "failed to retrieve partial contenf of file [%s]",
         request->GetError());
      delete request;
      return kTRUE;
   }

   // Analyze the response
   if (TCloudExtension::fgDebugLevel > 0)
      request->DumpResponseHeaders();

   Bool_t retCode = kTRUE;
   if (request->GetResponseStatusCode() == THttpRequest::kStatusPartialContent) {
      // Copy the response body to the output buffer and update this file
      // statistics
      if (ReceiveResponseBody(request, buffer, length)) {
         SetOffset(fOffset + length);
         UpdateFileStats(length);
         retCode = kFALSE;  // False means successful ReadBuffer
      }
   } else {
      Error("ReadBuffer", "failed to retrieve partial content of file [%s]",
         request->GetError());
   }

   // Update performance statistics
   if (!retCode && gPerfStats)
      gPerfStats->FileReadEvent(this, length, startTime);

   // Destroy the request
   delete request;
   return retCode;
}


//_____________________________________________________________________________
void THttpFile::UpdateFileStats(Int_t length)
{
   // Update the statistics about this file.

   fBytesRead += length;
   fReadCalls++;

#ifdef R__WIN32
   SetFileBytesRead(GetFileBytesRead() + length);
   SetFileReadCalls(GetFileReadCalls() + 1);
#else
   fgBytesRead += length;
   fgReadCalls++;
#endif
}


//_____________________________________________________________________________
Bool_t THttpFile::ReadBuffer(char *buffer, Long64_t position, Int_t length)
{
   // Read up to 'length' bytes into 'buffer', from file position 'position'.

   if (TCloudExtension::fgDebugLevel > 0)
      Info("ReadBuffer", "THttpFile::ReadBuffer(char *buffer, %lld, %d) starting",
         position, length);

   SetOffset(position);
   return ReadBuffer(buffer, length);
}


//_____________________________________________________________________________
Bool_t THttpFile::ReadBuffers(char *buffer, Long64_t *position, Int_t *length, Int_t numBuffers)
{
   if (TCloudExtension::fgDebugLevel > 0)
      Info("ReadBuffers", "THttpFile::ReadBuffers(char *buffer, Long64_t *position, Int_t *length, %d) starting",
         numBuffers);

   for (int i=0; i < numBuffers; i++) {
      if (ReadBuffer(buffer, position[i], length[i]) == kTRUE)
         return kTRUE;
      buffer += length[i];
   }

   return kFALSE;
}


//_____________________________________________________________________________
TString THttpFile::BuildRange(int numBlocks, Long64_t* from, Int_t* length)
{
   // Build the value of the Range header to be sent in an HTTP request for
   // retrieving chunks of a remote file.
   // 'numBlocks' contains the number of ranges to retrieve. 'from' is an
   // array which contains the initial position of each range and 'length'
   // is an array which contains the amount of bytes to retrieve for this range.
   // The general form of this header is: "bytes:10-20,35-155,800-1546"

   TString result;
   const char* commaSeparator = ",";
   const char* noSeparator = "";
   const char* separator = noSeparator;
   for (int i=0; i < numBlocks; i++) {
      if (i > 0)
         separator = commaSeparator;
      result += TString::Format("%s%lld-%d", separator, from[i], length[i]-1);
   }
   if (result.Length() == 0)
      return result;

   return "bytes=" + result;
}


//_____________________________________________________________________________
void THttpFile::Seek(Long64_t offset, TFile::ERelativeTo pos)
{
   // Set position from where to start reading.

   SetOffset(offset, pos);
}


//_____________________________________________________________________________
void THttpFile::SetOffset(Long64_t offset, TFile::ERelativeTo pos)
{
   // We need to implement this method here because it is not supported in
   // some old version of ROOT, such as v5.00.24b

   switch (pos) {
      case kBeg:
         fOffset = offset + fArchiveOffset;
         break;
      case kCur:
         fOffset += offset;
         break;
      case kEnd:
         // this option is not used currently in the ROOT code
         if (fArchiveOffset)
            Error("Seek", "seeking from end in archive is not (yet) supported");
         fOffset = fEND - fOffset; // is fEND really EOF or logical EOF?
         break;
   }
}


//_____________________________________________________________________________
Long64_t THttpFile::GetSize() const
{
   // Return the size (in bytes) of the remote file

   return fSize;
}


//_____________________________________________________________________________
Bool_t THttpFile::IsOpen() const
{
   // A THttpFile that has been correctly constructed is always considered open.

   return IsZombie() ? kFALSE : kTRUE;
}


//_____________________________________________________________________________
Bool_t THttpFile::ReceiveResponseBody(THttpRequest* request, char* buffer, Int_t length)
{
   // Receives the whole response body and copies it into the provided buffer.

   if (TCloudExtension::fgDebugLevel > 2)
      Info("ReceiveResponseBody", "copying %d bytes from response body into buffer", length);

   // Parse the Content-Length header and make sure that this response's body
   // contains the requested number of bytes
   Long_t contentLength = request->GetResponseBodyLength();
   if (contentLength != length) {
      Error("ReceiveResponseBody", "failed to retrieve %d bytes in HTTP response body [got %ld]",
         length, contentLength);
      return kFALSE;
   }

   // Copy the response body into the buffer
   Long_t copied = request->GetResponseBody(buffer, length);
   if (copied != length) {
      Error("ReceiveResponseBody", "could not read full response body [%s]",
          request->GetError());
      return kFALSE;
   }

   if (TCloudExtension::fgDebugLevel > 2)
      Info("ReceiveResponseBody", "copied %d bytes", length);

   return kTRUE;
}


//_____________________________________________________________________________
void THttpFile::SetProxy(const char *url)
{
   // Static method for setting the global proxy URL.

   if (url == 0)
      return;

   TUrl proxyUrl(url);
   if (strcmp(proxyUrl.GetProtocol(), "http") != 0) {
      ::Error("THttpFile::SetProxy", "protocol must be HTTP in proxy URL %s", url);
      return;
   }
   fgProxyUrl = proxyUrl;
}


//______________________________________________________________________________
const char* THttpFile::GetProxy()
{
   // Static method returning the URL of the global proxy.

   if (fgProxyUrl.IsValid())
      return fgProxyUrl.GetUrl();

   return "";
}


//_____________________________________________________________________________
void THttpFile::SetSessionProxy()
{
   // Sets the proxy to be used for downloading this file contents.

   // First check if there is a global proxy that we should use
   if (fgProxyUrl.IsValid()) {
      fHttpSession->SetProxyUrl(fgProxyUrl);
      return;
   }

   // Check if there is an environmental variable pointing to the proxy
   TString proxyEnv = gSystem->Getenv("http_proxy");
   if (proxyEnv == "")
      return;

   // Make sure the protocol used by the proxy is HTTP
   TUrl proxyUrl(proxyEnv);
   if (strcmp(proxyUrl.GetProtocol(), "http") != 0) {
      Error("SetSessionProxy", "expecting proxy protocol to be 'http'. Ignoring environment variable 'http_proxy'");
      return;
   }

   // Everything is OK. Set the proxy for this session
   fHttpSession->SetProxyUrl(proxyUrl);
}

//_____________________________________________________________________________
Bool_t THttpFile::GetAuthFromOptions(Option_t* options, TString& accessKey, TString& secretKey)
{
   // Extracts the authentication key pair (user name and API secret key)
   // from the options, if present.
   // The authentication credentials can be specified in the options provided
   // to the constructor of this class as a string containing:
   //     "AUTH=<access key>:<secret key>"
   // and can include other options, for instance "NOPROXY" for not using
   // the HTTP proxy for accessing this file's contents.
   //
   // For instance, the options may contain:
   // "NOPROXY AUTH=accessKey:secretKey"

   TString optStr = (const char*)options;
   if (optStr.IsNull()) {
      // No options
      return kFALSE;
   }

   // Look in the options string for the authentication information. We expect
   // options of the following form:
   //    "AUTH=accessKey:secretKey"
   //    "xxxx AUTH=accessKey:secretKey"
   //    "AUTH=accessKey:secretKey xxxx"
   //    "xxxx AUTH=accessKey:secretKey xxxx"
   // Please note that we don't make any assumption on the kind of characters
   // in the user name and API keys (a.k.a. access key and secret key), other
   // than they cannot contain ' ' (blank) nor ':'.
   TPMERegexp rex("(^AUTH=|^.* AUTH=)([^\\s].+?):([^\\s].+?)($|\\s.*$)", "i");
   if (rex.Match(optStr) != 5) {
      // There is not authentication information in the options
      return kFALSE;
   }

   // We found the access and secret key in the options. Store those values in
   // the output arguments.
   accessKey = TString(rex[2]);
   secretKey = TString(rex[3]);
   if (TCloudExtension::fgDebugLevel > 0) {
      Info("GetAuthFromOptions", "using authentication information from 'options' argument");
      Info("GetAuthFromOptions", "accessKey='%s'", accessKey.Data());
      Info("GetAuthFromOptions", "secretKey='%s'", secretKey.Data());
   }

   return kTRUE;
}


//_____________________________________________________________________________
Bool_t THttpFile::GetAuthFromEnv(
      const char* authUrlEnv,
      const char* accessKeyEnv,
      const char* secretKeyEnv,
      TString& outAuthUrl,
      TString& outAccessKey,
      TString& outSecretKey)
{
   // Sets the authentication URL (if any), the access and secret keys
   // from the environmental variables, if they are both set.

   // Check that the environmental variables for user and secret key are
   // both set. Auth URL is not mandatory as some cloud storage services
   // such as Amazon S3 does not have a specific one. Swift does.
   if (authUrlEnv != 0) {
      outAuthUrl = gSystem->Getenv(authUrlEnv);
   }
   TString accessKey = gSystem->Getenv(accessKeyEnv);
   TString secretKey = gSystem->Getenv(secretKeyEnv);
   if (accessKey.IsNull() || secretKey.IsNull()) {
      return kFALSE;
   }

   // Found the auth info in environment. Save it in the output arguments
   outAccessKey = accessKey;
   outSecretKey = secretKey;
   if (TCloudExtension::fgDebugLevel > 0) {
      Info("GetAuthFromEnv", "using authentication information from environmental variables '%s' and '%s'",
            accessKeyEnv, secretKeyEnv);
   }

   return kTRUE;
}


//_____________________________________________________________________________
ClassImp(THttpFilePlugin)

//_____________________________________________________________________________
THttpFilePlugin::THttpFilePlugin(const char* url, Option_t* options)
   : THttpFile(url, options, "", 1)
{
   // This wrapper class is used as a plugin handler for files which scheme is
   // 'http' or 'https' in some versions of ROOT which require that the
   // plugin constructor has 2 arguments. It inherits all its behavior from
   // the super-class.
}

//_____________________________________________________________________________
THttpFilePlugin::~THttpFilePlugin()
{
}

