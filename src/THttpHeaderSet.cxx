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
// THttpHeaderSet                                                       //
//                                                                      //
// Convenient class to model a set of HTTP headers in a HTTP request    //
// or response.                                                         //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TROOT.h"
#include "THttpHeaderSet.h"


ClassImp(THttpHeaderSet)


//_____________________________________________________________________________
THttpHeaderSet::THttpHeaderSet(Int_t initialSize)
            : fHeaders(initialSize)
{
   // Construct a header set of the specified initial size. It will grow
   // if necessary by adding more headers to the set.

   fHeaders.SetOwner(kTRUE);
}


//_____________________________________________________________________________
Bool_t THttpHeaderSet::HasHeader(const TString& key) const
{
   // Returns kTRUE if there is already a header with the specified key in
   // the set. kFALSE otherwise. It makes a case-insensitive comparison.

   return Find(key) >= 0;
}


//_____________________________________________________________________________
const THttpHeader* THttpHeaderSet::GetHeaderAt(Int_t pos) const
{
   // Returns a pointer to a header in position pos. May return 0 if there is
   // no header in that position. This method is intended for iterating over
   // a header set.

   return (THttpHeader*)fHeaders[pos];
}


//_____________________________________________________________________________
const TString& THttpHeaderSet::GetValueForHeader(const TString& key) const
{
   // Returns the value associated to the header with the specified key. If
   // there is no header with that key in the set (or the set is empty) returns
   // a zero-length string.

   Int_t pos = Find(key);
   if (pos >= 0) {
      THttpHeader* h = (THttpHeader*)fHeaders[pos];
      return h->GetValue();
   }

   static TString empty;
   return empty;
}


//_____________________________________________________________________________
Bool_t THttpHeaderSet::IsEmpty() const
{
   // Returns kTRUE if the set is empty.

   return (fHeaders.GetEntries() == 0) ? kTRUE : kFALSE;
}


//_____________________________________________________________________________
void THttpHeaderSet::Add(const TString& key, const TString& value)
{
   // Add a header with the specified key. If there is already a header with
   // that key its value will be replaced with the specified one.

   Int_t pos = Find(key);
   if (pos >= 0) {
      // There is already a header with this key. Replace its value.
      THttpHeader* h = (THttpHeader*)fHeaders[pos];
      h->SetValue(value);
   } else {
      // There is no header with this key in the set. Add a new one
      fHeaders.AddLast(new THttpHeader(key, value));
   }
}


//_____________________________________________________________________________
Bool_t THttpHeaderSet::Remove(const TString& key)
{
   // Remove the header with the specified key from the set. It deletes the
   // removed header. Returns kTRUE if there was a header with that key that
   // was successfully remove. kFALSE otherwise.

   Int_t pos = Find(key);
   if (pos < 0)
      return kFALSE;

   THttpHeader* h = (THttpHeader*)fHeaders.RemoveAt(pos);
   delete h;
   fHeaders.Compress(); // Remove empty slots from the underlying array
   return kTRUE;
}


//_____________________________________________________________________________
void THttpHeaderSet::Clear(Option_t*)
{
   // Remove all the headers in the set. After exeucting this operation the
   // header set is empty.

   fHeaders.Clear();
}


//_____________________________________________________________________________
Int_t THttpHeaderSet::Find(const TString& key) const
{
   // Helper function to find a header in the set given a key. It makes a
   // case-insensitive comparison of the keys. Returns -1 if no header could
   // be found or an integer >= 0 which represents the position of the found
   // header in the underlying array.

   for (int i=0; i < fHeaders.GetEntries(); i++) {
      THttpHeader* h = (THttpHeader*)fHeaders[i];
      if (key.CompareTo(h->GetKey(), TString::kIgnoreCase) == 0)
         return i;
   }
   return -1;
}




