/*****************************************************************************
 * File:        createTree.cxx                                               *
 * Description: ROOT macro to create a ROOT file containing 3 tree branches. *
 *              In the context of this project it is used to create a ROOT   *
 *              file for testing remote access using the ROOT cloud storage  *
 *              extension.                                                   *
 *              Adapted from ROOT tutorial: $ROOTSYS/tutorials/tree/htest.C  *                   *
 *****************************************************************************/


// If called without arguments, this function will create a local file named
// "tree.root" of length about 33 MBytes. You can scan its contents
// using the macro "scanTree.cxx".
//
// For invoking this function, execute ROOT in batch mode with the command:
//
//       $ root -l -b -q createTree.cxx
//
// or interactively in the ROOT interpreter:
//
//       $ root -l
//       root [0] .L createTree.cxx
//       root [0] createTree()
//
// In order to test remote access using cloud storage protocols within ROOT,
// upload the "tree.root" file to your own cloud storage space and
// use the macro "scanTree.cxx" passing as argument the location of
// the ROOT file in the cloud, i.e.
//     s3://s3.amazonaws.com/myBucket/path/to/tree.root


void createTree(const char* fileName="tree.root", int numIter=25000) {
   // create a Tree with a few branches of type histogram
   // 25000 entries are filled in the Tree
   // For each entry, the copy of 3 histograms is written
   // The data base will contain 75000 histograms.
   gBenchmark->Start("hsimple");
   TFile outFile(fileName, "recreate");
   if (!outFile.IsOpen()) return;

   Info("createTree", "populating tree in file '%s'", fileName);
   TTree* tree = new TTree("T", "test");
   TH1F* hpx = new TH1F("hpx", "This is the px distribution", 100, -4, 4);
   TH2F* hpxpy = new TH2F("hpxpy", "py vs px", 40, -4, 4, 40, -4, 4);
   TProfile* hprof = new TProfile("hprof", "Profile of pz versus px", 100, -4, 4, 0, 20);
   tree->Branch("hpx", "TH1F", &hpx, 32000, 0);
   tree->Branch("hpxpy", "TH2F", &hpxpy, 32000, 0);
   tree->Branch("hprof", "TProfile", &hprof, 32000, 0);

   Float_t px, py, pz;
   for (Int_t i=0; i < numIter; i++) {
      // if (i%1000 == 0) Info("createTree", "at entry: %d",i);
      gRandom->Rannor(px, py);
      pz = px*px + py*py;
      hpx->Fill(px);
      hpxpy->Fill(px, py);
      hprof->Fill(px, pz);
      tree->Fill();
   }
   tree->Print();
   outFile.Write();
   outFile.Close();
   gBenchmark->Show("hsimple");
}
