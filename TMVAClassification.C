/**********************************************************************************
 * This macro performs training and testing of a BDT classifier to discriminate   *
 * physics signal from a soft-QCD/MinimumBias background.                         *
 *                                                                                *
 * Input data: Signal = Drell-Yan Z->ee events, Background = MinimumBias events   *
 *                                                                                *
 * To run this macro, type the following command in a tereminal window            *
 *                                                                                *
 *    root -l TMVAClassification.C                                                *
 *                                                                                *
 * The output file "TMVA.root" can be analysed with the use of dedicated          *
 * macros (simply say: root -l <macro.C>), which can be conveniently              *
 * invoked through a GUI that will appear at the end of the run of this macro.    *
 **********************************************************************************/

#include "TMVAGui.C"

void TMVAClassification( ) 
{
   // this loads the library
   TMVA::Tools::Instance();
   std::cout << std::endl;
   std::cout << "==> Start TMVAClassification" << std::endl;

   // Create a new root output file.
   TString outfileName( "TMVA.root" );
   TFile* outputFile = TFile::Open( outfileName, "RECREATE" );

   // Create the factory object.
   TMVA::Factory *factory = new TMVA::Factory( "TMVAClassification", outputFile, 
                                               "!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D" );

   // ---------- input variables 
   factory->AddVariable("iso12x12", 'F');
   factory->AddVariable("iso4x4", 'F');
   factory->AddVariable("isoLshaped", 'F');
   factory->AddVariable("PUM0", 'F');

   // ---------- spectators ----------------
   factory->AddSpectator("pt", "F");
   factory->AddSpectator("eta", "F");
   factory->AddSpectator("phi", "F");
   factory->AddSpectator("e", "F");

   // read training and test data
   TString fSig = "DrellYan_Zee.root";
   TString fBkg = "MinBias.root";

   TFile *fileSig = TFile::Open( fSig );
   TFile *fileBkg = TFile::Open( fBkg );
   TTree *signal     = (TTree*)fileSig->Get("tree");
   TTree *background = (TTree*)fileBkg->Get("tree");
   

   // global event weights per tree (see below for setting event-wise weights)
   Double_t signalWeight     = 1.0;
   Double_t backgroundWeight = 1.0;

   // ====== register trees ====================================================
   // you can add an arbitrary number of signal or background trees
   factory->AddSignalTree    ( signal,     signalWeight     );
   factory->AddBackgroundTree( background, backgroundWeight );
   TCut mycuts = "pt > 10"; 
   TCut mycutb = "pt > 10"; 


   // tell the factory to use all remaining events in the trees after training for testing:
   factory->PrepareTrainingAndTestTree( mycuts, mycutb,
                                        "nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=NumEvents:!V" );

   // If no numbers of events are given, half of the events in the tree are used for training, and 
   // the other half for testing:

   // ---- Book MVA methods
   // Boosted Decision Trees
   factory->BookMethod( TMVA::Types::kBDT, "BDT", 
                        "!H:!V:NTrees=400:nEventsMin=400:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=20:PruneMethod=NoPruning" );
   
   // ---- Train MVAs using the set of training events
   factory->TrainAllMethods();
   
   // ---- Evaluate all MVAs using the set of test events
   factory->TestAllMethods();

   // ----- Evaluate and compare performance of all configured MVAs
   factory->EvaluateAllMethods();    

   // Save the output
   outputFile->Close();

   std::cout << "==> Wrote root file: " << outputFile->GetName() << std::endl;
   std::cout << "==> TMVAClassification is done!" << std::endl;      

   delete factory;

   // Launch the GUI for the root macros
   if (!gROOT->IsBatch()) TMVAGui( outfileName );
}
