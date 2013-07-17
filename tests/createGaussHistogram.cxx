/*****************************************************************************
 * File:        createGaussHistogram.cxx                                     *
 * Description: ROOT macro to create and store a Gauss histogram into a file *
 *              In the context of this project it is used to create a small  *
 *              file for testing remote access using the ROOT cloud storage  *
 *              extension.                                                   *
 *              Adapted from some examples included in ROOT distributon      *
 *****************************************************************************/

// If called without arguments, this function will create a local file named
// "gaussHistogram.root" of length about 4 KBytes. You can draw its contents
// using the macro "drawGaussHistogram.cxx".
//
// For invoking this function, execute ROOT in batch mode with the command:
//
//       $ root -l -b -q createGaussHistogram.cxx
//
// In order to test remote access using cloud storage protocols within ROOT,
// upload the "gaussHistogram.root" file to your own cloud storage space and
// use the macro "drawGaussHistogram.cxx" passing as argument the location of
// the ROOT file in the cloud, i.e.
//     s3://s3.amazonaws.com/myBucket/path/to/gaussHistogram.root

void createGaussHistogram(const char* fileName="gaussHistogram.root")
{
   // Create an instance of 1D histogram with a gaussian distribution
   TH1F histo("h1gauss", "Gaussian Distribution;X;Number of entries", 100, -5, 5);

   // Fill the histogram randomly
   histo.FillRandom("gaus", 20000);

   // Store the result in a file
   TFile outFile(fileName, "RECREATE");

   // Write the histogram in the file
   histo.Smooth(1000);
   histo.SetFillColor(kTeal);
   histo.Write();

   // Close the file
   outFile.Close();
}
