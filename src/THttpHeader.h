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

#ifndef ROOT_THttpHeader
#define ROOT_THttpHeader

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// THttpHeader                                                          //
//                                                                      //
// A THttpHeader is an object representing a header in HTTP requests    //
// and responses. It is composed of a key and a value.                  //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TUrl
#include "TUrl.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif


class THttpHeader: public TObject {

private:
   // Data members
   TString  fKey;     // Header key
   TString  fValue;   // Header value

public:
   // Constructors & Destructor
   THttpHeader(const TString& key="", const TString& value="");
   virtual ~THttpHeader() {};

   // Selectors
   const TString&      GetKey() const { return fKey; }
   TString             GetCanonicalKey() const;
   const TString&      GetValue() const { return fValue; }
   Long64_t            GetValueAsLong() const;

   // Modifiers
   virtual void        SetKey(const TString& key) { fKey = key; }
   virtual void        SetValue(const TString& value) { fValue = value; }

   ClassDef(THttpHeader, 0)  // Persistent HTTP session
};

#endif // ROOT_THttpHeader
