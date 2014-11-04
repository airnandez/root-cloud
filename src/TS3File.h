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

#ifndef ROOT_TS3File
#define ROOT_TS3File

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TS3File                                                              //
//                                                                      //
// A TS3Fileis a THttpFile which retrieves the file contents from a     //
// web server implementing the REST API of the Amazon S3 protocol. This //
// class is meant to be as generic as possible to be used with files    //
// hosted not only by Amazon S3 servers but also by other providers     //
// implementing the core of the S3 protocol.                            //
//                                                                      //
// The S3 protocol works on top of HTTPS (and HTTP) and imposes that    //
// each HTTP request be signed using a specific convention: the request //
// must include an 'Authorization' header which contains the signature  //
// of a concatenation of selected request fields. For signing the       //
// request, an 'Access Key Id' and a 'Secret Access Key' need to be     //
// known. These keys are used by the S3 servers to identify the client  //
// and to authenticate the request as genuine.                          //
//                                                                      //
// As an end user, you must know the Access Key and Secret Access Key   //
// in order to access each S3 file. They are provided to you by your S3 //
// service provider. Those two keys can be provided to ROOT when        //
// initializing an object of this class by two means:                   //
//                                                                      //
// a) by setting the environmental variables S3_ACCESS_KEY and          //
//    S3_SECRET_KEY to the appropriate values, or                       //
// b) by specifying them as an argument when opening each file.         //
//                                                                      //
// The first method is convenient if all the S3 files you want to       //
// access are hosted by a single provider. The second one is more       //
// flexible as it allows you to specify which credentials to use        //
// on a per-file basis. See the documentation of the constructor of     //
// this class for details on the syntax.                                //
//                                                                      //
// For doing the actual HTTP exchange, this class inherits from         //
// THttpFile which in turns uses libNeon.                               //
//                                                                      //
// For more information on the details of S3 protocol please refer to:  //
// "Amazon Simple Storage Service Developer Guide":                     //
// http://docs.amazonwebservices.com/AmazonS3/latest/dev/Welcome.html   //
//                                                                      //
// "Amazon Simple Storage Service REST API Reference"                   //
//  http://docs.amazonwebservices.com/AmazonS3/latest/API/APIRest.html  //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_THttpFile
#include "THttpFile.h"
#endif

#ifndef ROOT_TS3Session
#include "TS3Session.h"
#endif


class TS3File: public THttpFile {

private:
   // Data members
   TString                 fAccessKey;       // Access key
   TString                 fSecretKey;       // Secret key
   TString                 fBucket;          // Bucket name
   TString                 fObjectKey;       // File object key (within the bucket)
   TS3Session::EAuthType   fAuthType;        // S3 authentication type
   Bool_t                  fUseMultiRange;   // Is the S3 server capable of serving multirange requests?

   // Avoid compiler generation of default constructor
   TS3File();

   // Helpers
   Bool_t GetCredentialsFromEnv(TString& accessKey, TString& secretKey);
   Bool_t GetCredentialsFromOptions(const TString& options, TString& accessKey, TString& secretKey);


protected:
   virtual TS3Session*  MakeSession(const TUrl& fileUrl);
   Bool_t               Initialize(const TUrl& url, Option_t* options);

public:
   // Constructors & Destructor
   TS3File(const char* url, Option_t* options="", const char* ftitle="", Int_t compress=1);
   virtual ~TS3File();

   // Selectors
   const TString&  GetAccessKey() const { return fAccessKey; }
   const TString&  GetSecretKey() const { return fSecretKey; }
   const TString&  GetBucket() const { return fBucket; }
   const TString&  GetObjectKey() const { return fObjectKey; }
   TString         GetFilePath() const;

   // Modifiers
   TS3File& SetAccessKey(const TString& accessKey);
   TS3File& SetSecretKey(const TString& secretKey);

   ClassDef(TS3File, 0)  // Read a ROOT file from a S3 server
};


// Wrapper class to act as a plugin for handling S3 files when a class with
// a constructor with 2 arguments is required.
class TS3FilePlugin: public TS3File {

public:
   // Constructors & Destructor
   TS3FilePlugin(const char* url, Option_t* options="");
   virtual ~TS3FilePlugin();

   ClassDef(TS3FilePlugin, 0)  // Read a ROOT file from a S3 server (plugable)
};

#endif // ROOT_TS3File
