// ---------------------------------------------------------------------------//
// File:  scanTree.cxx
// Date:  June 2013
// Description: ROOT macro to automating testing the ROOT extensions to read
//              files using cloud protocols such as S3 and Swift API.
// ---------------------------------------------------------------------------//
#include <iostream>

// The file names are expected to have one of the following forms:
//
// Local file
//     "tree.root"

// HTTP file
//       "http://localhost:8080/tree.root"
//      "https://localhost:443/tree.root"

// Amazon (S3 and HTTP[s])
//         "s3://s3.amazonaws.com/myBucket/file.root"
//    "s3https://s3.amazonaws.com/myBucket/file.root"
//     "s3http://s3.amazonaws.com/myBucket/file.root"
//       "http://s3.amazonaws.com/myBucket/world-readable-file.root"
//      "https://s3.amazonaws.com/myBucket/world-readable-file.root"

// Google Storage (S3 and HTTP[s])
//         "gs://storage.googleapis.com/myBucket/file.root"
//    "gshttps://storage.googleapis.com/myBucket/file.root"
//     "gshttp://storage.googleapis.com/myBucket/file.root"
//       "http://storage.googleapis.com/myBucket/world-readable-file.root"
//      "https://storage.googleapis.com/myBucket/world-readable-file.root"

// Swift (S3 and native Swift API)
//         "s3://myswift.example.com:8080/myBucket/file.root"
//      "swift://myswift.example.com:8080/myBucket/file.root"

// Rackspace (native Swift API)
//      "swift://identity.api.rackspacecloud.com/myBucket/file.root"


int scanTree(const char* fileName="tree.root", int debug=0)
{
   // Set ROOT's debug level
   if (debug > 0)
      gDebug = debug;

   // Our test file contains a tree named "T". We retrieve it and show histograms
   // for entry 12345.
   // This is adapted from tutorial in $ROOTSYS/tutorials/tree

   // Open the file
   TFile* inputFile = TFile::Open(fileName, "READ");
   if (inputFile == 0) {
      Info("scanTree", "could not open file '%s'\n", fileName);
      return 1;
   }

   // Collect benchmark information
   gBenchmark->Start("scan");

   // Retrieve the tree named "T" and display histograms
   TTree *tree = (TTree*)inputFile->Get("T");
   TString canvasName = TString::Format("canvas2-%s", fileName);
   TCanvas *canvas = new TCanvas(canvasName, fileName, 10, 10, 600, 1000);
   canvas->Divide(1, 3);
   canvas->cd(1);
   tree->Draw("hpx.Draw()", "", "goff", 1, 12345);
   canvas->cd(2);
   tree->Draw("hpxpy.Draw()", "", "goff", 1, 12345);
   canvas->cd(3);
   tree->Draw("hprof.Draw()", "", "goff", 1, 12345);

   // Show some benchmark figues
   showBenchmarkResults("scan", inputFile);
}


void showBenchmarkResults(const char* benchmark, const TFile* f)
{
   gBenchmark->Show(benchmark);
   Float_t realTime = gBenchmark->GetRealTime(benchmark);
   if (realTime > 0.001) {
      Long64_t bytesRead = f->GetBytesRead();
      if (bytesRead > 0) {
         Float_t rate = bytesRead/realTime;
         Info("showBenchmarkResults", "data read rate: %s\n", formatRate(rate).Data());
      }
   }
}


TString formatRate(Float_t rate) {
   const char* units[] = {
      "bytes", "KBytes", "MBytes", "GBytes"
   };
   int numUnits = sizeof(units)/sizeof(units[0]);
   int i = 0;
   for (;i < numUnits; i++) {
      if (rate > 1024.0)
         rate /= 1024.0;
      else
         break;
   }
   if (i == numUnits)
      --i;
   return TString::Format("%.0f %s/sec", rate, units[i]);
}
