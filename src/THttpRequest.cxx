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
// THttpRequest                                                         //
//                                                                      //
// A THttpRequest is a HTTP request with its associated response. It is //
// strongly coupled with class THttpSession. Both of them use libNeon.  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////



#include "TROOT.h"
#include "TError.h"
#include "TSystem.h"
#include "THttpRequest.h"
#include "THttpSession.h"
#include "TCloudExtension.h"

// libNeon include files
#include "ne_session.h"
#include "ne_request.h"


ClassImp(THttpRequest)


//_____________________________________________________________________________
THttpRequest::THttpRequest(THttpSession* session, const TString& verb,
                const TString& path, const TString& query)
{
   fSession = session;
   fRawRequest = 0;
   fBody = 0;
   fBodyLength = 0;
   fSubmitted = kFALSE;
   SetVerb(verb);
   SetPath(path);
   SetQuery(query);
}


//_____________________________________________________________________________
THttpRequest::~THttpRequest()
{
   if (fRawRequest != 0) {
      if (fSubmitted) {
         // Discard any non-consumed response part
         ne_discard_response(fRawRequest);
         // Terminate this request
         ne_end_request(fRawRequest);
      }
      // Destroy the underlying raw request request
      ne_request_destroy(fRawRequest);
   }
}


//_____________________________________________________________________________
THttpRequest& THttpRequest::SetHeader(const TString& header, const TString& value)
{
   // Set a header for this request.

   fReqHeaderSet.Add(header, value);
   return *this;
}

//_____________________________________________________________________________
THttpRequest& THttpRequest::SetBody(const char* body, Int_t length)
{
   // Set the request body

   fBody = body;
   fBodyLength = (fBody == 0) ? 0 : length;
   return *this;
}


//_____________________________________________________________________________
Bool_t THttpRequest::PreSubmit()
{
   // This method is called before the request is sent to the server. It is
   // intended to be overwritten by the subclasses for preparing each request
   // before sending it.
   // Must return kTRUE if the request is ready to be sent, kFALSE otherwise.

   // Default behavior: do nothing
   return kTRUE;
}


//_____________________________________________________________________________
void THttpRequest::PostSubmit()
{
   // This method is called after the request is sent to the server and
   // we got a response from it. It is intended to be overwritten by the
   // subclasses for processing the result of each request, if needed.

   // Default behavior: do nothing
}


//_____________________________________________________________________________
Bool_t THttpRequest::Submit()
{
   // Send the request to the HTTP server. If successful, after returning this
   // request response is ready to be consumed.

   // Don't send again if this request was successfully sent already
   if (fSubmitted)
      return kTRUE;

   // Prepare the request for submission
   if (!PreSubmit()) {
      Error("Submit", "HTTP request not ready for submission");
      return kFALSE;
   }

   // Create a new raw Neon request
   fRawRequest = (ne_request*)fSession->MakeRawRequest(fVerb, fPath, fQuery);
   if (fRawRequest == 0) {
      Error("Submit", "could not create raw request");
      return kFALSE;
   }

   // Add the HTTP headers to the newly created raw request
   for (int i=0; i < fReqHeaderSet.GetNumHeaders(); i++) {
      const THttpHeader* h = fReqHeaderSet.GetHeaderAt(i);
      ne_add_request_header(fRawRequest, h->GetKey(), h->GetValue());
   }

   if (TCloudExtension::fgDebugLevel > 0) {
      Info("Submit", "sending HTTP request to server %s://%s",
         fSession->GetScheme().Data(), fSession->GetServerHostAndPort().Data());
      Info("Submit", "%s %s", fVerb.Data(), GetFullPath().Data());
      DumpRequestHeaders();
   }

   // Add request body (if any)
   if (fBody != 0) {
      ne_set_request_body_buffer(fRawRequest, fBody, fBodyLength);
   }

   // Actually send the request via Neon
   int ret;
   do {
      ret = ne_begin_request(fRawRequest);
   } while (ret == NE_RETRY);

   // Retrieve the response headers if the request was successfully submitted
   if (ret == NE_OK) {
      RetreiveResponseHeaders();
      PostSubmit();
   }

   fSubmitted = (ret == NE_OK);
   if (!fSubmitted && (TCloudExtension::fgDebugLevel > 0)) {
      Info("Submit", "error submitting request [%s]", GetError());
   }
   return fSubmitted;
}


//_____________________________________________________________________________
void THttpRequest::RetreiveResponseHeaders()
{
   // Copy the response headers from Neon to our header set.

   fRespHeaderSet.Clear();
   const char* header;
   const char* value;
   void* cursor = ne_response_header_iterate(fRawRequest, 0, &header, &value);
   while (cursor != 0) {
      fRespHeaderSet.Add(header, value);
      cursor = ne_response_header_iterate(fRawRequest, cursor, &header, &value);
   }

}

//_____________________________________________________________________________
Int_t THttpRequest::GetResponseStatusCode() const
{
   // Return the response status code of this request. If not yet submitted,
   // return 0.

   if (!fSubmitted)
      return 0;

   const ne_status* status = ne_get_status(fRawRequest);
   return status->code;
}


//_____________________________________________________________________________
Int_t THttpRequest::GetResponseStatusClass() const
{
   // Return the response status class of this request. If not yet submitted,
   // return 0. The status class is 200 for all responses with status code
   // in the interval [200, 200).

   if (!fSubmitted)
      return 0;

   const ne_status* status = ne_get_status(fRawRequest);
   return status->klass;
}


//_____________________________________________________________________________
const TString& THttpRequest::GetResponseHeader(const TString& header) const
{
   // Returns the value associated to a HTTP header, if present in the response.
   // The return value may be "" of there is no such header in the response.

   static TString empty;
   if (!fSubmitted)
      return empty;

   return fRespHeaderSet.GetValueForHeader(header);
}


//_____________________________________________________________________________
const char* THttpRequest::GetError() const
{
   // Return an error message associated with this request.

   ne_session* session = ne_get_session(fRawRequest);
   return ne_get_error(session);
}


//_____________________________________________________________________________
TString THttpRequest::GetFullPath() const
{
   // Return the full path of this request, including the query part, if any.

   if (fQuery.Length() == 0)
      return fPath;

   return TString::Format("%s?%s", fPath.Data(), fQuery.Data());
}


//_____________________________________________________________________________
Long64_t THttpRequest::GetResponseBodyLength() const

{
   // Returns the number of bytes in the response body according to the
   // response's HTTP "Content-Length" header. Returns -1 if this header
   // is not in the response.

   if (!fSubmitted)
      return 0;

   TString contentLength(fRespHeaderSet.GetValueForHeader("Content-Length"));
   return (contentLength.Length() == 0) ? -1 : contentLength.Atoll();
}


//_____________________________________________________________________________
Long_t THttpRequest::GetResponseBody(char* buffer, Long_t length)
{
   // Copies up to length bytes from the response body to the buffer.
   // Returns the number of bytes copied which may be less than the requested
   // number if the body length is shorter. Use GetResponseBodyLength() to
   // retrieve the expected number of bytes in the response body.

   char* destination = buffer;
   ssize_t remaining = length;
   while (remaining > 0) {
      ssize_t numRead = ne_read_response_block(fRawRequest, destination, remaining);
      if (numRead <= 0)
         break;
      remaining -= numRead;
      destination += numRead;
   }
   return length - remaining;
}


//_____________________________________________________________________________
void THttpRequest::DumpResponseHeaders() const
{
   // Prints the contents of the response associated to this request
   // (for debugging purposes)

   // Dump status codes
   int statusCode = GetResponseStatusCode();
   int statusClass = GetResponseStatusClass();
   Info("DumpResponseHeaders", "Reponse status code: %d", statusCode);
   Info("DumpResponseHeaders", "Reponse status class: %d", statusClass);

   // Dump headers
   for (int i=0; i < fRespHeaderSet.GetNumHeaders(); i++) {
      const THttpHeader* h = fRespHeaderSet.GetHeaderAt(i);
      Info("DumpResponseHeaders", "%s: %s", h->GetCanonicalKey().Data(), h->GetValue().Data());
   }
}


//_____________________________________________________________________________
void THttpRequest::DumpRequestHeaders() const
{
   // Prints the contents of this request (for debugging purposes)

   Info("DumpRequestHeaders", "Host: %s", fSession->GetServerHostAndPort().Data());

   for (int i=0; i < fReqHeaderSet.GetNumHeaders(); i++) {
      const THttpHeader* h = fReqHeaderSet.GetHeaderAt(i);
      Info("DumpRequestHeaders", "%s: %s", h->GetCanonicalKey().Data(), h->GetValue().Data());
   }
}


//_____________________________________________________________________________
THttpRequest& THttpRequest::SetVerb(const TString& verb)
{
   // Set the HTTP verb (i.e. GET, PUT, HEAD, etc. ). Note that we don't validate
   // the provided verb. If it is not a valid one, the request will fail when
   // it will be submitted.

   fVerb = verb;
   fVerb.ToUpper();
   return *this;
}


//_____________________________________________________________________________
THttpRequest& THttpRequest::SetPath(const TString& path)
{
   // Set the path for this request. We always store an absolute path, i.e.
   // one starting with "/".

   fPath = path.BeginsWith("/") ? path : TString::Format("/%s", path.Data());
   return *this;
}


//_____________________________________________________________________________
THttpRequest& THttpRequest::SetQuery(const TString& query)
{
   // Set the query part of this request. The query will be used when building
   // the full path. It will be put after '?'.

   fQuery = query;
   return *this;
}

