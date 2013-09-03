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

#ifndef ROOT_TCloudExtension
#define ROOT_TCloudExtension

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCloudExtension                                                      //
//                                                                      //
// A TCloudExtension encapsulates all the information about this        //
// ROOT extension needed by other classes.                              //
//////////////////////////////////////////////////////////////////////////


class TCloudExtension: public TObject {

public:
   // Data members: don't use a selector for retrieving the value of the
   // debug level so to avoid provoking a context switch every time we want
   // to retrieve the debug level. This is important when we are running in
   // production mode (i.e. not debug mode) to not waste cycles.
   // Call the static method InitDebugLevel() to initialize this data member
   // to its appropriate value.
   static Int_t fgDebugLevel;    // Extension debug level

   // Modifiers
   static void InitDebugLevel();

   ClassDef(TCloudExtension, 0)  // Cloud extension
};

#endif // ROOT_TCloudExtension
