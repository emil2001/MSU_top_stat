///////////////////////////////////////////////////////////////////////////////
//  Author      :     P.S. Mandrik, IHEP
//  Date        :     01/03/20
//  Last Update :     11/05/20
//  Version     :     1.1
///////////////////////////////////////////////////////////////////////////// 

#ifndef PMLIB_ROOT_HIST_DRAWER_HH
#define PMLIB_ROOT_HIST_DRAWER_HH 1

namespace pm {

  class CanvasStyle {
    public:
      CanvasStyle(){
        style_index = 0;
      }

      void SetStyle(){
        if(style_index == 0){
          int font = 132;
          gStyle->SetFrameBorderMode(0);
          gStyle->SetCanvasBorderMode(0);
          gStyle->SetPadBorderMode(0);

          gStyle->SetFrameFillColor(0);
          gStyle->SetFrameFillStyle(0);

          gStyle->SetPadColor(0);
          gStyle->SetCanvasColor(0);
          gStyle->SetTitleColor(1);
          gStyle->SetStatColor(0);

          gStyle->SetLegendBorderSize(0);
          gStyle->SetLegendFillColor(0);
          gStyle->SetLegendFont(font);
          
          gStyle->SetOptStat(0000000);
          gStyle->SetTextFont(font);
          gStyle->SetTextSize(0.05);
          gStyle->SetLabelFont(font,"XYZ");
          gStyle->SetTitleFont(font,"XYZ");
          gStyle->SetLabelSize(0.05,"XYZ"); //0.035
          gStyle->SetTitleSize(0.05,"XYZ");
          
          gStyle->SetTitleOffset(1.25,"X");
          gStyle->SetTitleOffset(2.05,"Y");
          gStyle->SetLabelOffset(0.02,"XY");
          
          // use bold lines and markers
          gStyle->SetMarkerStyle(8);
          gStyle->SetHistLineWidth(3);
          gStyle->SetLineWidth(1);

          gStyle->SetNdivisions(510,"xy");

          // do not display any of the standard histogram decorations
          gStyle->SetOptTitle(0);
          gStyle->SetOptStat(0); //("m")
          gStyle->SetOptFit(0);
          
          //gStyle->SetPalette(1,0)
          gStyle->cd();
          gROOT->ForceStyle();
        }
      }

      void SetStyleHist(TH1 * hist){
        if(style_index == 0){
          cout << "111" << endl;
          hist->GetYaxis()->CenterTitle();
          hist->GetYaxis()->SetNdivisions(510);

          hist->GetYaxis()->SetLabelFont(132);
          hist->GetYaxis()->SetLabelOffset(0.02);
          hist->GetYaxis()->SetLabelSize(0.04);
          hist->GetYaxis()->SetTitleFont(132);
          hist->GetYaxis()->SetTitleOffset(1.5);
          hist->GetYaxis()->SetTitleSize(0.045);

          hist->GetXaxis()->SetLabelFont(132);
          hist->GetXaxis()->SetLabelOffset(0.02);
          hist->GetXaxis()->SetLabelSize(0.06);
          hist->GetXaxis()->SetTitleFont(132);
          hist->GetXaxis()->SetTitleOffset(1.5);
          hist->GetXaxis()->SetTitleSize(0.045);
          hist->SetStats(false);

          gStyle->SetOptTitle(0);
          gStyle->SetOptStat(0); //("m")
          gStyle->SetOptFit(0);
        }
      }

      void SetStyleCanvas(TCanvas * canvas){
        if(style_index == 0){
          canvas->SetRightMargin(0.10);
          canvas->SetLeftMargin(0.20);
          canvas->SetBottomMargin(0.18);
          canvas->SetTopMargin(0.12);
        }
      }

      int style_index;
  };

  void set_minmax_from_hist(double & min, double & max, TH1* hist){
    if( min > hist->GetMinimum() ) min = hist->GetMinimum();
    if( max < hist->GetMaximum() ) max = hist->GetMaximum();
  }

  
  // mode == "" - just central and other histograms
  // mode == "diff" - draw differents between central and other histograms
  // mode == "diff percent" - draw differents between central and other histograms in percents
  void draw_hists_difference(TH1* central, const std::vector<TH1*> & others, std::string mode="", float range_factor=1.2){
    TH1* cen = (TH1D*)central->Clone();
    cen->SetStats(0);
    if(mode == "diff percent" or mode == "diff"){
      cen->Add(central, -1);
    }
    if(mode == "")             cen->GetYaxis()->SetTitle("N");
    if(mode == "diff percent") cen->GetYaxis()->SetTitle("(N_{sys} - N)/N");
    if(mode == "diff")         cen->GetYaxis()->SetTitle("N_{sys} - N");
    cen->Draw("hist");
    double min = cen->GetMinimum();
    double max = cen->GetMaximum();
    if(mode == "percent"){
      min = 0;
      max = 0;
    }

    std::vector<TH1*> all_hre_hists;
    for(auto hist : others){
      TH1 * hre = (TH1*)hist->Clone();
      if(mode == "diff percent" or mode == "diff"){
        hre->Add( central, -1. );
      }
      if(mode == "diff percent"){
        // cout << hist->GetTitle() << endl;
        for(int j = 1; j <= central->GetNbinsX(); j++ ){
           float central_bin_value = central->GetBinContent(j);
          if( TMath::Abs(central_bin_value) > 0.0000001){
            hre->SetBinContent(j, 100. * hre->GetBinContent(j) / central_bin_value );
            // cout << 100. * hre->GetBinContent(j) / central_bin_value << " " << hre->GetBinContent(j) << " " << central_bin_value << endl;
          }
          else if( TMath::Abs( hre->GetBinContent(j) ) > 0.0000001 ) hre->SetBinContent(j, 100. * hre->GetBinContent(j) / TMath::Abs( hre->GetBinContent(j) ) );
          else hre->SetBinContent(j, 0 );
        }
      }
      hre->Draw("hist same");
      set_minmax_from_hist(min, max, hre);
      all_hre_hists.push_back( hre );
    }

    if(max < 0) max /= range_factor;
    else        max *= range_factor;
    if(min > 0) min /= range_factor;
    else        min *= range_factor;
    double size_y = TMath::Abs( max - min );

    cen->GetYaxis()->SetRangeUser(min, max);
    cen->Draw("hist same");

    // draw hist labels at the canvas
    vector< int > reserved_bins;
    vector< float > reserved_positions;
    for(int i = 0; i < all_hre_hists.size(); i++){
      auto hre = all_hre_hists.at(i);

      vector< pair<float, int> > bins_values;
      for(int j = 1; j <= hre->GetNbinsX(); j++ )
        bins_values.push_back( make_pair(TMath::Abs( hre->GetBinContent(j) ), j ) );
      sort(bins_values.begin(), bins_values.end() );

      int bin = bins_values.at(0).second;
      int bin_to_search = bin;
      for(int j = bins_values.size()-1; j >= 0; j--){
        bin_to_search = bins_values.at( j ).second;
        bool change_position = false;
        for(int j = 0; j < reserved_bins.size(); j++){
          if( reserved_bins[j] != bin_to_search ) continue;
          float distance = TMath::Abs( reserved_positions[j] - 1.1*hre->GetBinContent( bin_to_search ) ) / size_y;
          if( distance > 0.05) continue;
          change_position = true;
          break;
        }
        if( not change_position ) break;
      }
      bin = bin_to_search;

      // int bin = 1 + i * TMath::Max( 1, int((0.75 * hre->GetNbinsX()) / all_hre_hists.size()) ) ;
      // bin = bin % hre->GetNbinsX();
      
      string title = string( hre->GetTitle() );

      double posx = hre->GetBinCenter( bin );
      double posy = 1.1 * hre->GetBinContent( bin );

      reserved_bins.push_back( bin );
      reserved_positions.push_back( posy );

      auto *tt = new TText(posx, posy, title.c_str() );
      tt->SetTextSize(0.025);
      tt->SetTextColor( hre->GetLineColor() );
      if( bin > 0.75 * hre->GetNbinsX() ) tt->SetTextAlign(31);
      tt->Draw();
    }
  }


  // draw shade graph with central value and contours
  struct BrasilPoint{
    float xs, pl2, pl1, pc, pu1, pu2;
    BrasilPoint(int index, vector<float> & xsv, vector<float> & pl2v, vector<float> & pl1v, vector<float> & pcv, vector<float> & pu1v, vector<float> & pu2v){
      xs = xsv.at(index);
      pl2 = pl2v.at(index);
      pl1 = pl1v.at(index);
      pc  = pcv.at(index);
      pu1 = pu1v.at(index);
      pu2 = pu2v.at(index);
    }

    bool operator < (const BrasilPoint & p) const{
        return (xs < p.xs);
    }
  };

  TGraph * draw_brasil(vector<float> & xs, vector<float> & pl2, vector<float> & pl1, vector<float> & pc, vector<float> & pu1, vector<float> & pu2){
    int n_size = pc.size();
    TGraph * gc  = new TGraph( n_size );
    TGraph * gs1 = new TGraph( 2*n_size );
    TGraph * gs2 = new TGraph( 2*n_size );

    vector<BrasilPoint> bpoints;
    for(int i = 0; i < n_size; i++){
      bpoints.push_back(BrasilPoint(i, xs, pl2, pl1, pc, pu1, pu2));
    }
    std::sort( bpoints.begin(), bpoints.end() );    
    
    for(int i = 0; i < n_size; i++){
      gc->SetPoint(  i,            bpoints.at(i).xs, bpoints.at(i).pc );
      
      gs1->SetPoint( i,            bpoints.at(i).xs, bpoints.at(i).pl1 );
      gs1->SetPoint( 2*n_size-1-i, bpoints.at(i).xs, bpoints.at(i).pu1 );
      
      gs2->SetPoint( i,            bpoints.at(i).xs, bpoints.at(i).pl2 );
      gs2->SetPoint( 2*n_size-1-i, bpoints.at(i).xs, bpoints.at(i).pu2 );
    }
    
    gs2->SetFillColor(kYellow);
    gs2->Draw("AF");

    gs1->SetFillColor(kGreen+1);
    gs1->Draw("F");

    gc->SetLineStyle(2);
    gc->SetLineWidth(2);
    gc->Draw("L");
    
    return gs2;
  } 
  
  
  
  // draw brasil discrete limits
  class BrasilDrawer{
    public:
    vector<string> names;
    vector<float> s2_ds, s1_ds, cs, s1_us, s2_us, obss;
    CanvasStyle * style;
    string label_y, label_x, right_text_src, left_text_src;
      
    BrasilDrawer(){
      style = nullptr;
    }

    void AddPoint(string name, float s2_d, float s1_d, float c, float s1_u, float s2_u, float obs){
      names.push_back( name );
      s2_ds.push_back( s2_d );
      s1_ds.push_back( s1_d );
      cs.push_back( c );
      s1_us.push_back( s1_u );
      s2_us.push_back( s2_u ); 
      obss.push_back( obs );
    }
    
    TH1D* GetHist(string name, const vector<float> & values){
      TH1D* h = new TH1D(name.c_str(), name.c_str(), values.size(), 0, values.size());
      for(int i = 0; i < values.size(); i++){
        h->Fill( names.at(i).c_str(), values.at( i ) );
      }
      return h;
    }
    
    TH1D* GetHistErrors(string name, const vector<float> & v1, const vector<float> & v2){
      TH1D* h = new TH1D(name.c_str(), name.c_str(), v1.size(), 0, v2.size());
      for(int i = 0; i < v1.size(); i++){
        float c = (v1.at(i) + v2.at(i) ) / 2.;
        h->Fill( names.at(i).c_str(), c );
        h->SetBinError( i+1, TMath::Abs(c - v1.at(i)) );
      }
      return h;
    }
    
    TCanvas * Draw( bool draw_observed ){
      TH1D* h_c = GetHist("Expected 95% C.L.", cs);
      TH1D* h_s1 = GetHistErrors("#pm#sigma", s1_us, s1_ds);
      TH1D* h_s2 = GetHistErrors("#pm2#sigma", s2_us, s2_ds);

      double maxh_v = -1.0;
      double minh_v = -1.0;
      vector<TH1D*> hists = {h_c, h_s1, h_s2};
      for(TH1D * hist : hists){
        maxh_v = TMath::Max(maxh_v, hist->GetBinContent(hist->GetMaximumBin()));
        minh_v = TMath::Min(minh_v, hist->GetBinContent(hist->GetMinimumBin()));
      }

      for(TH1D * hist : hists){
        hist->SetMaximum( 3.5 * maxh_v );
        hist->SetMinimum( 1.5 * minh_v );
      }

      TCanvas * canv = new TCanvas("Brasil", "Brasil", 640, 480);
      if(style){
        style->SetStyle();
        style->SetStyleCanvas(  canv );
        for(TH1D * hist : hists){
          style->SetStyleHist(hist);
          hist->GetYaxis()->SetTitle( label_y.c_str() );
          hist->GetXaxis()->SetTitle( label_x.c_str() );
        }
      }
      
      h_s2->Draw("E2");
      h_s1->Draw("E2 same");
      h_c->Draw("hist same");
      h_c->SetLineWidth(2);

      h_s1->SetFillColor(kGreen);
      h_s2->SetFillColor(kYellow);
      h_c->SetLineStyle(2);

      TLegend * legend = new TLegend(0.55,0.65,0.90,0.88);
      legend->SetFillColor(0);
      legend->SetFillStyle(3001);
      legend->SetLineColor(0);
      legend->SetTextFont(132) ;
      legend->AddEntry(h_c, "Expected 95% C.L.", "l");
      legend->AddEntry(h_s1, "#pm#sigma", "f");
      legend->AddEntry(h_s2, "#pm2#sigma", "f");
      legend->Draw();

      TLatex * rtext = new TLatex( 0.23, 0.82, right_text_src.c_str());
      rtext->SetNDC(kTRUE) ;
      rtext->SetTextSize( 0.044 ) ;
      rtext->SetTextFont(132) ;
      rtext->Draw() ;

      TLatex * ltext = new TLatex( 0.50, 0.92, left_text_src.c_str());
      ltext->SetNDC(kTRUE) ;
      ltext->SetTextSize( 0.044 ) ;
      ltext->SetTextFont(132) ;
      ltext->Draw() ;

      canv->RedrawAxis();
      canv->GetFrame()->SetBorderSize( 12 );
      return canv;
    }
  };
  
};

void pmlib_root_hist_drawer(){
  TH1F *h = new TH1F("h","",10,0,10);

  h->SetLineColor(kRed);

  for(int i=0; i<9; i++){
    for(int j = 0; j < 100; j++) h->Fill(i,10 * i);
  }

  /*try this first, you
    see it will fill the 
    whole histogram with
    blue*/
  h->Draw("E2");
  //h->Draw("hist same");
  

  /*if i instead try the opposite
    I get back to where I was 
    where the whole histogram is filled*/
  //h->DrawCopy("hist"); 
  h->SetFillColor(kBlue);
  //h->SetFillStyle(3018);
  //h->Draw("e2same");
 

  /*then try this instead,
    you see only the error band is 
    blue, and this is what I want
    but in addition
    I want the line of the histogram 
    showing, but this just 
    gives me the two cases above*/
  //h->Draw("e2");

  }

#endif
