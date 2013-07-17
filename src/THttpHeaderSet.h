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

#ifndef ROOT_THttpHeaderSet
#define ROOT_THttpHeaderSet

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// THttpHeaderSet                                                       //
//                                                                      //
// Convenience class to model a set of HTTP headers in a HTTP request   //
// or response.                                                         //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef ROOT_TUrl
#include "TUrl.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif

#ifndef ROOT_TObjArray
#include "TObjArray.h"
#endif

#ifndef ROOT_THttpHeader
#include "THttpHeader.h"
#endif


class THttpHeaderSet: public TObject {

private:
   // Helpers
   Int_t Find(const TString& key) const;

   // Data members
   TObjArray  fHeaders;     // Set of HTTP headers

public:
   // Constructors & Destructor
   THttpHeaderSet(Int_t initialSize = 20);
   virtual ~THttpHeaderSet() {};

   // Selectors
   Bool_t              HasHeader(const TString& key) const;
   const THttpHeader*  GetHeaderAt(Int_t pos) const;
   const TString&      GetValueForHeader(const TString& key) const;
   Int_t               GetNumHeaders() const { return fHeaders.GetEntries(); }
   Bool_t              IsEmpty() const;

   // Modifiers
   void                Add(const TString& key, const TString& value);
   Bool_t              Remove(const TString& key);
   virtual void        Clear(Option_t* opt="");

   ClassDef(THttpHeaderSet, 0)  // HTTP header set
};

#endif // ROOT_THttpHeaderSet
