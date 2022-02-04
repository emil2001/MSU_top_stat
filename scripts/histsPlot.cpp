

#include "getQuantiles.cpp"
#include "mroot/mRootStackDrawer.cpp"

int histsPlot(string mode, TString inputFileName, float scaleFactor=0.){
  TFile *file = TFile::Open( inputFileName );

  mRoot::StackDrawer drawer;
  drawer.AddHistsFromFile( file, ".+", "(.+Down$)|(.+Up$)|(.+_alt)|(.+_weight_.+)|(.+_scale_.+)" );
  drawer.legend_width = 0.25;

  sort(drawer.stack_hists.begin(), drawer.stack_hists.end(), [](const TH1D* a, const TH1D* b){ return strcoll(a->GetName(),b->GetName()) < 0; } );
  for(auto hist : drawer.stack_hists){
    cout << hist->GetName() << endl;
    //cout << drawer.stack_hists.size() << endl;
    }
  if(mode == "QCD_before"){}
  else if(mode == "QCD_after"){
    double qcd_f = scaleFactor;
    for(auto hist : drawer.stack_hists)
      if( string(hist->GetName()) == "QCD" ) hist->Scale( qcd_f );
  }
  if (mode == "SM2D_before"){
  TCanvas * canv = drawer.GetCanvas( mode );
  mode = "SM_before";
  canv->Print( (mode + ".png").c_str() );
  }
  else if(mode == "main_before"){}
  TCanvas * canv = drawer.GetCanvas( mode );
  canv->Print( (mode + ".png").c_str() );

  return 0;
}


