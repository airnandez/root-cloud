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

#ifndef ROOT_THttpFile
#define ROOT_THttpFile

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

#ifndef ROOT_TFile
#include "TFile.h"
#endif

#ifndef ROOT_TUrl
#include "TUrl.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif


class THttpSession;
class THttpRequest;


class THttpFile: public TFile {

private:
   // Avoid compiler generation of default constructor
   THttpFile();
   // Helper functions
   Bool_t            RetrieveFileSize();
   void              UpdateFileStats(Int_t length);
   TString           BuildRange(int numBlocks, Long64_t* from, Int_t* length);
   Bool_t            ReceiveResponseBody(THttpRequest* request, char* buffer, Int_t length);
   void              SetSessionProxy();
   void              TerminateSession();


protected:
   // Data members
   TUrl           fUrl;             // File URL
   THttpSession*  fHttpSession;     //! HTTP session
   Long64_t       fSize;            // File size in bytes
   static TUrl    fgProxyUrl;       // Globally set proxy URL

   // Helpers (to be used by subclasses)
   Bool_t GetAuthFromOptions(Option_t* options, TString& accessKey, TString& secretKey);
   Bool_t GetAuthFromEnv(const char* accessKeyEnv, const char* secretKeyEnv,
                         TString& outAccessKey, TString& outSecretKey);

   // Modifiers
   virtual THttpSession*  MakeSession(const TUrl& fileUrl);
   Bool_t                 Initialize(const TUrl& url, Option_t* options);

public:
   // Constructors & Destructor
   THttpFile(const char* url, Option_t* options="", const char* ftitle="", Int_t compress=1);
   virtual ~THttpFile();

   // Modifiers
   virtual Bool_t       ReadBuffer(char *buffer, Int_t length);
   virtual Bool_t       ReadBuffer(char *buffer, Long64_t position, Int_t length);
   virtual Bool_t       ReadBuffers(char *buffer, Long64_t *position, Int_t *length, Int_t numBuffers);
   virtual void         Seek(Long64_t offset, TFile::ERelativeTo pos=kBeg);
   virtual void         SetOffset(Long64_t offset, TFile::ERelativeTo pos = kBeg);
   virtual void         Close(Option_t* option="");

   // Selectors
   virtual Long64_t     GetSize() const;
   virtual Bool_t       IsOpen() const;

   // Class methods
   static void          SetProxy(const char *url);
   static const char*   GetProxy();

   ClassDef(THttpFile, 0)  // Read a ROOT file from a HTTP server
};


// Wrapper class to be used as a TFile plugin when a constructor with 2
// arguments is required.
class THttpFilePlugin: public THttpFile {

public:
   // Constructors & Destructor
   THttpFilePlugin(const char* url, Option_t* options="");
   virtual ~THttpFilePlugin();

   ClassDef(THttpFilePlugin, 0)  // Read a ROOT file from a HTTP server (plugable)
};

#endif // ROOT_THttpFile
