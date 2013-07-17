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

#ifndef ROOT_TSwiftFile
#define ROOT_TSwiftFile

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TSwiftFile                                                           //
//                                                                      //
// A TSwiftFile is a TFile which retrieves its contents from a file     //
// server using the native  OpenStack Swift protocol (as opposed to S3  //
// which can be used to some OpenStack installations).                  //
// The recommended way to open a Swift file is via                      //
// TFile* f=TFile::Open("swift://myserver.domain.com/bucket/myFile.root")//                                                                   //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_THttpFile
#include "THttpFile.h"
#endif

#ifndef ROOT_TSwiftSession
#include "TSwiftSession.h"
#endif


class TSwiftFile: public THttpFile {

private:
   // Data members
   TString      fUserName;        // Access key id
   TString      fApiAccessKey;    // Secret key id
   TString      fBucket;          // Bucket name
   TString      fObjectKey;       // File object key (within the bucket)

   // Disallow the default constructor
   TSwiftFile();

protected:
   virtual TSwiftSession*  MakeSession(const TUrl& fileUrl);
   Bool_t                  Initialize(const TUrl& url, Option_t* options);

public:
   // Constructors & Destructor
   TSwiftFile(const char* url, Option_t* options="", const char* ftitle="", Int_t compress=1);
   virtual ~TSwiftFile();

   // Selectors
   const TString&  GetUserName() const { return fUserName; }
   const TString&  GetApiAccessKey() const { return fApiAccessKey; }
   const TString&  GetBucket() const { return fBucket; }
   const TString&  GetObjectKey() const { return fObjectKey; }
   TString         GetFilePath() const;

   // Modifiers
   TSwiftFile& SetUserName(const TString& userName);
   TSwiftFile& SetApiAccessKey(const TString& apiAccessKey);

   ClassDef(TSwiftFile, 0)  // Read a ROOT file from a OpenStack Swift server
};

#endif // ROOT_TSwiftFile
