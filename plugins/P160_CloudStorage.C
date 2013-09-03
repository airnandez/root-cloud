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

//_____________________________________________________________________________
// This file is to be installed in $ROOTSYS/etc/plugins/TFile. It installs the
// appropriate extension plugins for enabling ROOT reading remote files using
// cloud storage protocols such as Amazon S3 and OpenStack Swift.
//_____________________________________________________________________________


void P160_CloudStorage()
{
   // Install handlers for cloud file types, according to this version of ROOT.
   //
   // NOTE: it is important to take into account that, depending of the version
   // of ROOT, files starting with some schemes (for instance 'http') are
   // considered of type WEB (see method TFile::GetType). This means that in
   // this case the plugin handlers expects a plugin with a constructor with 4
   // arguments. For cases of unknown schemes, the plugin handler expects a
   // constructor with 2 arguments.
   //
   // The implication of this is that we need to install the appropriate plugin
   // class according to the version of ROOT we are running on. In some cases
   // we use the class 'TS3File' and in some others the class 'TS3FilePlugin'.

   if (gROOT->GetVersionInt() < 53200) {
      // Before ROOT v5.32.00  no cloud protocol were supported. Here we add
      // plugins for handling the following schemes:
      // https, s3, s3http, s3https, gs, gshttp, gshttps, swift.
      // In addition, we remove the built-in plugin for reading HTTP files
      // and replace it by the one in RootCloudStorage library.
      gPluginMgr->RemoveHandler("TFile", "^http:");

      // HTTP
      gPluginMgr->AddHandler("TFile", "^http:", "THttpFilePlugin", "RootCloudStorage",
         "THttpFilePlugin(const char*,Option_t*)");

      // HTTPS: this scheme was not supported in previous versions of ROOT, so
      // we provide a plugin with 4 arguments.
      gPluginMgr->AddHandler("TFile", "^https:", "THttpFile", "RootCloudStorage",
         "THttpFile(const char*,Option_t*,const char*,Int_t)");

      // S3: Amazon
      gPluginMgr->AddHandler("TFile", "^s3:", "TS3File", "RootCloudStorage",
         "TS3File(const char*,Option_t*,const char*,Int_t)");
      gPluginMgr->AddHandler("TFile", "^s3http:", "TS3File", "RootCloudStorage",
         "TS3File(const char*,Option_t*,const char*,Int_t)");
      gPluginMgr->AddHandler("TFile", "^s3https:", "TS3File", "RootCloudStorage",
         "TS3File(const char*,Option_t*,const char*,Int_t)");

      // S3: Google
      gPluginMgr->AddHandler("TFile", "^gs:", "TS3File", "RootCloudStorage",
         "TS3File(const char*,Option_t*,const char*,Int_t)");
      gPluginMgr->AddHandler("TFile", "^gshttp:", "TS3File", "RootCloudStorage",
         "TS3File(const char*,Option_t*,const char*,Int_t)");
      gPluginMgr->AddHandler("TFile", "^gshttps:", "TS3File", "RootCloudStorage",
         "TS3File(const char*,Option_t*,const char*,Int_t)");

      // OpenStack Swift
      gPluginMgr->AddHandler("TFile", "^swift:", "TSwiftFile", "RootCloudStorage",
         "TSwiftFile(const char*,Option_t*,const char*,Int_t)");
      gPluginMgr->AddHandler("TFile", "^swhttps:", "TSwiftFile", "RootCloudStorage",
         "TSwiftFile(const char*,Option_t*,const char*,Int_t)");
      gPluginMgr->AddHandler("TFile", "^swhttp:", "TSwiftFile", "RootCloudStorage",
         "TSwiftFile(const char*,Option_t*,const char*,Int_t)");
   }
   else if (gROOT->GetVersionInt() < 53403) {
      // From ROOT v5.32.00 up to v5.34.03 only 'as3' and 'gs' were known types.
      // Remove those plugins and the built-in plugin for 'http' and add the
      // ones from the RootCloudStorage library.
      gPluginMgr->RemoveHandler("TFile", "^http:");
      gPluginMgr->RemoveHandler("TFile", "^as3:");
      gPluginMgr->RemoveHandler("TFile", "^gs:");

      // HTTP and HTTPS
      gPluginMgr->AddHandler("TFile", "^http:", "THttpFilePlugin", "RootCloudStorage",
         "THttpFilePlugin(const char*,Option_t*)");
      gPluginMgr->AddHandler("TFile", "^https:", "THttpFile", "RootCloudStorage",
         "THttpFile(const char*,Option_t*,const char*,Int_t)");

      // S3: Amazon
      gPluginMgr->AddHandler("TFile", "^s3:", "TS3File", "RootCloudStorage",
         "TS3File(const char*,Option_t*,const char*,Int_t)");
      gPluginMgr->AddHandler("TFile", "^s3http:", "TS3File", "RootCloudStorage",
         "TS3File(const char*,Option_t*,const char*,Int_t)");
      gPluginMgr->AddHandler("TFile", "^s3https:", "TS3File", "RootCloudStorage",
         "TS3File(const char*,Option_t*,const char*,Int_t)");

      // S3: Google
      gPluginMgr->AddHandler("TFile", "^gs:", "TS3File", "RootCloudStorage",
         "TS3File(const char*,Option_t*,const char*,Int_t)");
      gPluginMgr->AddHandler("TFile", "^gshttp:", "TS3File", "RootCloudStorage",
         "TS3File(const char*,Option_t*,const char*,Int_t)");
      gPluginMgr->AddHandler("TFile", "^gshttps:", "TS3File", "RootCloudStorage",
         "TS3File(const char*,Option_t*,const char*,Int_t)");

      // OpenStack Swift
      gPluginMgr->AddHandler("TFile", "^swift:", "TSwiftFile", "RootCloudStorage",
         "TSwiftFile(const char*,Option_t*,const char*,Int_t)");
      gPluginMgr->AddHandler("TFile", "^swhttps:", "TSwiftFile", "RootCloudStorage",
         "TSwiftFile(const char*,Option_t*,const char*,Int_t)");
      gPluginMgr->AddHandler("TFile", "^swhttp:", "TSwiftFile", "RootCloudStorage",
         "TSwiftFile(const char*,Option_t*,const char*,Int_t)");
   }
   else {
      // From ROOT v5.34.03 on, the following schemes were known types:
      // as3, s3, s3https, s3http, gs, gshttps, gshttp.
      // We remove the default built-in handlers for these schemes and
      // install handlers from the RootCloudStorage library.
      gPluginMgr->RemoveHandler("TFile", "^http[s]?:");
      gPluginMgr->RemoveHandler("TFile", "^[a]?s3:");
      gPluginMgr->RemoveHandler("TFile", "^s3http[s]?:");
      gPluginMgr->RemoveHandler("TFile", "^gs:");
      gPluginMgr->RemoveHandler("TFile", "^gshttp[s]?:");

      // HTTP and HTTPS
      gPluginMgr->AddHandler("TFile", "^http:", "THttpFilePlugin", "RootCloudStorage",
         "THttpFilePlugin(const char*,Option_t*)");
      gPluginMgr->AddHandler("TFile", "^https:", "THttpFilePlugin", "RootCloudStorage",
         "THttpFilePlugin(const char*,Option_t*)");

      // S3: Amazon
      gPluginMgr->AddHandler("TFile", "^s3:", "TS3FilePlugin", "RootCloudStorage",
         "TS3FilePlugin(const char*,Option_t*)");
      gPluginMgr->AddHandler("TFile", "^s3http:", "TS3FilePlugin", "RootCloudStorage",
         "TS3FilePlugin(const char*,Option_t*)");
      gPluginMgr->AddHandler("TFile", "^s3https:", "TS3FilePlugin", "RootCloudStorage",
         "TS3FilePlugin(const char*,Option_t*)");

      // S3: Google
      gPluginMgr->AddHandler("TFile", "^gs:", "TS3FilePlugin", "RootCloudStorage",
         "TS3FilePlugin(const char*,Option_t*)");
      gPluginMgr->AddHandler("TFile", "^gshttps:", "TS3FilePlugin", "RootCloudStorage",
         "TS3FilePlugin(const char*,Option_t*)");
      gPluginMgr->AddHandler("TFile", "^gshttp:", "TS3FilePlugin", "RootCloudStorage",
         "TS3FilePlugin(const char*,Option_t*)");

      // OpenStack Swift
      gPluginMgr->AddHandler("TFile", "^swift:", "TSwiftFile", "RootCloudStorage",
         "TSwiftFile(const char*,Option_t*,const char*,Int_t)");
      gPluginMgr->AddHandler("TFile", "^swhttps:", "TSwiftFile", "RootCloudStorage",
         "TSwiftFile(const char*,Option_t*,const char*,Int_t)");
      gPluginMgr->AddHandler("TFile", "^swhttp:", "TSwiftFile", "RootCloudStorage",
         "TSwiftFile(const char*,Option_t*,const char*,Int_t)");
   }
}
