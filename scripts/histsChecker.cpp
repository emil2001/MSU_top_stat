
#include <regex>
#include "mroot/msg.hh"
#include "mroot/mRootNiceColors.cpp"

#include "pmlib/pmlib_root_hist_drawer.hh"

using namespace mRoot;

vector <TObject*> * get_all_from_folder(TDirectory * dir, string include_rexp=".+", string exclude_rexp=""){
  vector <TObject*> * objs = new vector <TObject*>;

  regex reg_incl( include_rexp );
  regex reg_excl( exclude_rexp );

  auto keys = dir->GetListOfKeys();

  for(int i = 0; i < keys->GetSize(); i++){
    auto obj =  ((TKey*) keys->At(i))->ReadObj();
    string name = obj->GetName();

    if( !regex_match(name, reg_incl) ) continue;
    if( regex_match(name, reg_excl) ) continue;

    objs->push_back( obj );
  }

  return objs;
}

void check_template(TH1 * hist){
  int nbins = hist->GetXaxis()->GetNbins();
  bool hist_is_ok = true;
  for(int nbins = 1; nbins <= hist->GetXaxis()->GetNbins(); nbins++) {
    // cout << hist->GetBinError(nbins) << " " << endl;
    float value = hist->GetBinContent(nbins);

    if( string(hist->GetName()) == string("Wjets"))
      msg("Wjets", nbins, value);
    if( string(hist->GetName()) == string("Wjets_JESUp"))
      msg("Wjets_JESUp", nbins, value);
    if( string(hist->GetName()) == string("Wjets_JESDown"))
      msg("Wjets_JESUp", nbins, value);

    if(value <= 0){
      //msg("WARNING !!! ", hist->GetName(), "bin", nbins, value);
      hist_is_ok = false;
    }
  }
  // if(hist_is_ok) msg("hist is ok ", hist->GetName());
}

#include "mroot/mRootStackDrawer.cpp"
void histsChecker(TString inputFileName, TString postfix, string diff_mode, int dummy){
  // TString inp_file_name = "hists13Charlie_SM.root";
  // TString inp_file_name = "hist_copy.root";
  // TString inp_file_name = "hists13Charlie_SM_fix.root";
  gErrorIgnoreLevel = kWarning;
  TFile * file = TFile::Open(inputFileName);
    
  mRoot::StackDrawer drawer;
  drawer.AddHistsFromFile( file, "W.+", "(Wjets)|(.+Down$)|(.+Up$)|(.+_alt)|(.+_weight_.+)|(.+_scale_.+)" );
  drawer.legend_width = 0.25;
  drawer.data_hist = (TH1D*)file->Get("Wjets");

  TCanvas * canv = drawer.GetCanvas( "Comparison_wjets" );
  canv->Print( "Comparison_wjets.png" );
  
  // draw all hists
  // for every draw central and shifted hists
  auto hists_central = get_all_from_folder(file, ".+", "(.+Down$)|(.+Up$)|(.+_weight_.+)|(.+_scale_.+)");
  vector <int> indexes = getNiceColors(150);
 // PrintNiceColors(10);  

  for(auto hist : *hists_central){
    TCanvas * canv = new TCanvas(hist->GetName(), hist->GetName(), 640 * 4, 640*3);

    canv->Divide(4, 3);
    
    TH1* h = (TH1*)hist;
    cout << " " << h->GetName() << " ... " << endl;

    check_template(h);

    auto shifts = get_all_from_folder(file, string(hist->GetName()) + "_.+", "("+string(hist->GetName()) + "_alt.*)|(.+_weight_.+)");
    int color_i = 0;
    h->SetLineColor( 1 );
    h->SetLineWidth( 1 );

    TLegend * legend = new TLegend(0.05,0.05,0.95,0.95);
    legend->AddEntry(h,  h->GetName(), "l");
   
    list<TH1D*> herrs;
    string lname = "";
    int index = 0;
    int prev_index = -1;

    vector<TH1*> hists_other;
    for(auto shift : *shifts){
      TH1* hs = (TH1*)shift;
         
      check_template(hs);
      hs->SetLineColor( indexes.at(color_i) );
      //cout << color_i << endl;   
      hs->SetLineWidth( 1 );
      //cout << " " << hs->GetName() << " ... " << hs->Integral() << endl;
      hists_other.push_back( hs );
      lname.size() < string(hs->GetTitle()).size() ? lname = string(hs->GetTitle()) : lname;

      TH1D* hist_err = (TH1D*)hs->Clone();
      hist_err->Add( h, -1. );
      hist_err->Divide( h );
      hist_err->SetMarkerStyle( 30 + color_i );
      hist_err->SetFillStyle( 3000+color_i );
      hist_err->SetFillColor( indexes.at(color_i) );
      color_i += 1;

      index++;

      for(int i = 1; i < hist_err->GetNbinsX()+1; i++){
        hist_err->SetBinContent(i, abs(hist_err->GetBinContent(i)));
      }

      for(auto it = herrs.begin(); it!=herrs.end(); ++it){
        if( (*it)->Integral() < hist_err->Integral() ){
          herrs.insert(it, hist_err);
          goto skipp_add;
        }
      }
      herrs.push_back(hist_err);
      skipp_add: ;
      //cout << " " << hs->GetName() << " ... " << index << endl;  
    }

    // TH1D* central, const std::vector<TH1D*> & others
    
    bool if_odd = (hists_other.size() / 11) % 2;
    int size = hists_other.size() / 11;
    for(int i = 1; i <= 11; i ++){
      if (size == 0) break;
      canv->cd( i );
      vector<TH1*> hists_other_i; 
      //cout << (i-1) * size << endl;
      for(int j = (i-1) * size; j < i * size - 1 ; j+=2 )
      {
        hists_other_i.push_back( hists_other.at(j) );
        hists_other_i.push_back( hists_other.at(j+1) );
        //cout << j << " " << hists_other.at(j)->GetName() << " " << hists_other.at(j+1)->GetName() << endl;
      }
      if (if_odd && i % 2 == 0) {
        hists_other_i.push_back( hists_other.at(i * size - 1) );
        hists_other_i.push_back( hists_other.at((i-1) * size - 1) );
        //cout << "odd    " << hists_other.at(i * size - 1)->GetName() << " " << hists_other.at((i-1) * size - 1)->GetName() << endl;
      }

      pm::draw_hists_difference( h, hists_other_i, diff_mode );
    }

    //THStack * stack = new THStack(TString(h->GetName()) + "_stack", TString(h->GetName()) + "_stack");
    for(auto it = herrs.begin(); it!=herrs.end(); ++it){
      //stack->Add( (*it) );
      
      string title = string((*it)->GetTitle());
      for(int i = (lname.size() - title.size())*1.3; i>=0; i--) title += " ";
      title += " - " + to_string( (*it)->Integral()/(*it)->GetNbinsX() );
      legend->AddEntry(*it,  title.c_str(), "lf");
    }

    canv->cd(12);
    legend->Draw();

    //canv->cd(12);
    //stack->Draw("hist f");

    canv->Print(postfix + TString(hist->GetTitle()) + TString(".pdf"));
  }

}



