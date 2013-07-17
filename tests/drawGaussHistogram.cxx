/*****************************************************************************
 * File:        drawGaussHistogram.cxx                                       *
 * Description: ROOT macro to read and display a Gauss histogram created by  *
 *              the macro createGaussHistogram.cxx.                          *
 *              In the context of this project it is used to demonstrate     *
 *              remote access using the ROOT cloud storage extension.        *
 *              Adapted from some examples included in ROOT distributon      *
 *****************************************************************************/

// If called without arguments, this function will read and display a local
// file named "gaussHistogram.root" which contains a histogram created by the
// macro in "createGaussHistogram.cxx".
//
// In order to test remote access using cloud storage protocols within ROOT,
// upload the "gaussHistogram.root" file to your own cloud storage space. Then
// set the environmental variables S3_ACCESS_KEY and S3_SECRET_KEY with the
// contents of your access key and secret keys, respectively, by using the
// shell commands:
//
//    export S3_ACCESS_KEY="my access key"
//    export S3_SECRET_KEY="my very secret key"
//
// and then read the file from within a ROOT interactive session by passing
// its URL as an argument to this function. For instance:
//
// $ root -l
// root [0] .L drawGaussHistogram.cxx
// root [1] drawGaussHistogram("s3://s3.amazonaws.com/myBucket/path/to/gaussHistogram.root")
// Info in <drawGaussHistogram>: opening file 's3://s3.amazonaws.com/myBucket/path/to/gaussHistogram.root'
// Info in <TCanvas::MakeDefCanvas>:  created default TCanvas with name c1
//
// You should see in your screen a Gauss histogram.

void drawGaussHistogram(const char* fileName="gaussHistogram.root")
{
   // Open the ROOT file which contains the histogram
   Info("drawGaussHistogram", "opening file '%s'", fileName);
   TFile* inputFile = TFile::Open(fileName);
   if (inputFile == 0) {
      Error("drawGaussHistogram", "could not open input file '%s'", fileName);
      return;
   }

   // Load the histogram
   TH1F* histogram = (TH1F*)inputFile->GetObjectChecked("h1gauss", "TH1F");

   // Draw it
   histogram->Draw();
}
