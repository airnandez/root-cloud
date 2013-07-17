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
// THttpHeader                                                          //
//                                                                      //
// A THttpHeader is an object representing a header in HTTP requests    //
// and responses. It is composed of a key and a value.                  //
//////////////////////////////////////////////////////////////////////////


#include "TROOT.h"
#include "THttpHeader.h"


ClassImp(THttpHeader)


//_____________________________________________________________________________
THttpHeader::THttpHeader(const TString& key, const TString& value)
            : fKey(key), fValue(value)
{
   // Construct a header from a (key,value) pair. As HTTP headers are case
   // insensitive, we store the keys in lower case and provide an accessor
   // method (GetCanonicalKey) to retrieve a canonicalized form for the header
   // key.

   fKey.ToLower();
}


//_____________________________________________________________________________
Long64_t THttpHeader::GetValueAsLong() const
{
   // Return the value of this header as a long. If the value is not
   // numerical it returns 0.

   return fValue.Atoll();
}


//_____________________________________________________________________________
TString THttpHeader::GetCanonicalKey() const
{
   // Returns a canonicalized form of the header. For instance, something of
   // the form "Content-Length" (note the upper case). This is a a convenient
   // method for printing the header key in a human-friendly way.

   TString canonical(fKey);
   canonical.ToLower();
   Ssiz_t length = canonical.Length();
   if (length > 0) {
      canonical[0] = toupper(canonical[0]);
      Ssiz_t pos = canonical.Index("-", 1, 0, TString::kExact);
      while (pos != kNPOS) {
         if ((pos+1) < length) {
            canonical[pos+1] = toupper(canonical[pos+1]);
         }
         pos = canonical.Index("-", 1, pos+1, TString::kExact);
      }
   }
   return canonical;
}
