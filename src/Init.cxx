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


#include "TROOT.h"
#include "TError.h"
#include "TCloudExtension.h"

// libNeon include files
#include "ne_request.h"


//_____________________________________________________________________________
Bool_t InitLibNeon()
{
   // Initialize the libNeon library. It needs to be called only once but can
   // be called more.

   // If already called, return immediately
   static Bool_t neonInitialized = kFALSE;
   if (neonInitialized)
      return kTRUE;

   // Actually initialize libNeon
   if (ne_sock_init() != 0) {
      ::Error("InitLibNeon", "could not initialize libNeon library");
      return kFALSE;
   }

   // Done
   neonInitialized = kTRUE;
   return kTRUE;
}


//_____________________________________________________________________________
void __attribute__ ((constructor)) InitCloudExtension(void)
{
   // Initialization routine for the ROOT cloud extension dynamic library. It
   // is called once when the library is first loaded.

   // Initialize libNeon. Called here to make sure that it is executed only
   // once at the time the dynamic library is loaded.
   InitLibNeon();

   // Initialize the debug level to be used by the classes of this extension
   TCloudExtension::InitDebugLevel();
}
