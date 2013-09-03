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
// TCloudExtension                                                      //
//                                                                      //
// A TCloudExtension encapsulates all the information about this        //
// ROOT extension used by other classes.                                //
//////////////////////////////////////////////////////////////////////////


#include "TROOT.h"
#include "TEnv.h"
#include "TError.h"
#include "TCloudExtension.h"


ClassImp(TCloudExtension)

Int_t TCloudExtension::fgDebugLevel = 0;

//_____________________________________________________________________________
void TCloudExtension::InitDebugLevel()
{
   // Initialize the debug level according to the value of the global ROOT
   // debug level and the specific debug level set for this extension in
   // the environment of this ROOT session.

   // Retrieve the debug level specific for this extension (if any) from the
   // ROOT config files.
   fgDebugLevel = gEnv->GetValue("CloudStorageExtension.Root.Debug", 0);

   // If the global ROOT debug level is higher than the extension-specific
   // debug level, use the global value.
   if (gDebug > fgDebugLevel)
      fgDebugLevel = gDebug;

   if (fgDebugLevel > 0)
      ::Info("InitDebugLevel", "cloud storage extension debug level set to %d", fgDebugLevel);
}
