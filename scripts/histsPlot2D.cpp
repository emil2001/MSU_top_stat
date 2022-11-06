#include "getQuantiles.cpp"
#include "mroot/mRootStackDrawer2D.cpp"

int histsPlot2D(string mode, TString inputFileName, float scaleFactor=0.){
  TFile *file = TFile::Open( inputFileName );

  mRoot::StackDrawer drawer;
  drawer.AddHistsFromFile( file, ".+", "(.+Down$)|(.+Up$)|(.+_alt)|(.+_weight_.+)|(.+_scale_.+)" );
  
  drawer.legend_width = 0.25;

  sort(drawer.stack_hists.begin(), drawer.stack_hists.end(), [](const TH2D* a, const TH2D* b){ return strcoll(a->GetName(),b->GetName()) < 0; } );

  TCanvas * canv = drawer.GetCanvas( mode );
  canv->Print( (mode + ".png").c_str() );
  return 0;
}


