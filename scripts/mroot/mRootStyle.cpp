
#ifndef mRootStyle_hh
#define mRootStyle_hh 1

#include <TROOT.h>
#include <TStyle.h>
#include <TLatex.h>
#include <TText.h>
#include "mRootStackDrawer.cpp"
#include "mRootNiceColors.cpp"

namespace mRoot {

  template<class TMP_hist_type = TH1D>
  class HistDrawer {
    public:
      HistDrawer(){
        font = 132;
        logY = false;
        label_x = "X";
        label_y = "Events";
        xmin = 0; xmax = 0; 
        ymin = 0; ymax = 0;
      }

      void Print(){
        cout << "mRoot::HistDrawer.Print() " << endl;
        cout << "signals = " << signals.size() << endl;
        for(auto hist : signals) hist->Print();
        cout << " backgroudns = " << backgrounds.size() << endl;
        for(auto hist : backgrounds) hist->Print();
      }

      void SetStyleHist(TH1 * hist){
        hist->GetYaxis()->SetTitle( label_y.c_str() );
        hist->GetXaxis()->SetTitle( label_x.c_str() );

        hist->GetYaxis()->CenterTitle();
        hist->GetYaxis()->SetNdivisions(510);

        hist->GetYaxis()->SetLabelFont(132);
        hist->GetYaxis()->SetLabelOffset(0.02);
        hist->GetYaxis()->SetLabelSize(0.05);
        hist->GetYaxis()->SetTitleFont(132);
        hist->GetYaxis()->SetTitleOffset(1.5);
        hist->GetYaxis()->SetTitleSize(0.045);

        hist->GetXaxis()->SetLabelFont(132);
        hist->GetXaxis()->SetLabelOffset(0.02);
        hist->GetXaxis()->SetLabelSize(0.05);
        hist->GetXaxis()->SetTitleFont(132);
        hist->GetXaxis()->SetTitleOffset(1.5);
        hist->GetXaxis()->SetTitleSize(0.045);
        hist->SetStats(false);
      }

      void SetStyle(){
        std::vector <int> colors = mRoot::getNiceColors();
        int color = 0;
        int signal_color = 2;

        for(auto hist : signals){
          // colors
          hist->SetLineColor( signal_color++ );
          hist->SetLineWidth( 5 );
          hist->SetLineStyle( 7 );
          // style
          SetStyleHist(hist);

          if(xmin != xmax) hist->GetXaxis()->SetRangeUser(xmin, xmax);
          else             hist->GetXaxis()->UnZoom();
        }

        for(auto hist : backgrounds){
          // colors
          hist->SetLineColor( colors.at(color++) );
          hist->SetLineWidth( 3 );
          SetStyleHist(hist);

          if(xmin != xmax) hist->GetXaxis()->SetRangeUser(xmin, xmax);
          else             hist->GetXaxis()->UnZoom();
        }
      }

      void SetMinMax(double minh_v, double maxh_v){
        for(auto hist : signals     ){ 
          hist->SetMaximum( maxh_v );
          hist->SetMinimum( minh_v );
        }
        for(auto hist : backgrounds ) {
          hist->SetMaximum( maxh_v );
          hist->SetMinimum( minh_v );
        }
      }

      void SetMaximum(){
        double maxh_v = -1.0;
        double minh_v = -1.0;
        for(auto hist : signals     ) maxh_v = TMath::Max(maxh_v, hist->GetBinContent(hist->GetMaximumBin()));
        for(auto hist : backgrounds ) maxh_v = TMath::Max(maxh_v, hist->GetBinContent(hist->GetMaximumBin()));
        if( logY ) {
          // minh_v = 0.000001*maxh_v;
          // maxh_v = 100000*maxh_v;
          minh_v = 0.0000001*maxh_v;
          maxh_v = 10000*maxh_v;
        }
        else {
          minh_v = 0.;
          maxh_v = 1.5*maxh_v;
        }
        SetMinMax(minh_v, maxh_v);
      }

      void SetMaximumStack(THStack * hs){
        double maxh_v =  0.0;
        double minh_v = -1.0;
        for(auto hist : backgrounds ) maxh_v += hist->GetBinContent(hist->GetMaximumBin());
        for(auto hist : signals     ) maxh_v = TMath::Max(maxh_v, hist->GetBinContent(hist->GetMaximumBin()));
        if( logY ) {
          minh_v = 0.000001*maxh_v;
          maxh_v = 100000*maxh_v;
        }
        else {
          minh_v = 0.;
          maxh_v = 1.5*maxh_v;
        }
        SetMinMax(minh_v, maxh_v);
        hs->SetMinimum( minh_v );
        hs->SetMaximum( maxh_v );
      }

      void DrawHists(){
        SetMaximum();
             if(backgrounds.size())  backgrounds.at(0)->Draw("hist");
        else if( signals.size()   )      signals.at(0)->Draw("hist");
        for(auto hist : backgrounds) hist->Draw("same hist");
        for(auto hist : signals) hist->Draw("same hist");
      }

      void DrawHistsStack(){
        THStack * hs = new THStack("stack", "stack");
        for(auto hist : backgrounds){
          hist->SetFillColor( hist->GetLineColor() );
          hs->Add(hist);
        }
        SetMaximumStack( hs );
        hs->Draw("hist");
        for(auto hist : signals) hist->Draw("same hist");

        if( not hs->GetXaxis() or not hs->GetYaxis() or not hs->GetHistogram() ) return;
        SetStyleHist( hs->GetHistogram() );
        if( signals.size() ){
          hs->GetXaxis()->SetTitle( signals.at(0)->GetXaxis()->GetTitle() );
          hs->GetYaxis()->SetTitle( signals.at(0)->GetYaxis()->GetTitle() );
        }
        else if( backgrounds.size() ){
          hs->GetXaxis()->SetTitle( backgrounds.at(0)->GetXaxis()->GetTitle() );
          hs->GetYaxis()->SetTitle( backgrounds.at(0)->GetYaxis()->GetTitle() );
        }
      }

      void DrawHistsTMVA(){
        THStack * hs = new THStack();
        double sum_integral = 0;
        for(auto hist : backgrounds){
          sum_integral += hist->Integral();
          hist->SetFillColor( hist->GetLineColor() );
          hist->Print();
          hs->Add(hist);
        }
        //for(auto hist : backgrounds)
        //  hist->Scale(1./sum_integral);
        for(auto hist : signals)
          hist->Scale( 0.25*sum_integral / hist->Integral() );

        SetMaximumStack( hs );
        hs->Draw("hist");

        for(auto hist : signals)
          hist->Draw("same hist");

        if( not hs->GetXaxis() or not hs->GetYaxis() or not hs->GetHistogram() ) return;
        SetStyleHist( hs->GetHistogram() );
        if( signals.size() ){
          hs->GetXaxis()->SetTitle( signals.at(0)->GetXaxis()->GetTitle() );
          hs->GetYaxis()->SetTitle( signals.at(0)->GetYaxis()->GetTitle() );
        }
        else if( backgrounds.size() ){
          hs->GetXaxis()->SetTitle( backgrounds.at(0)->GetXaxis()->GetTitle() );
          hs->GetYaxis()->SetTitle( backgrounds.at(0)->GetYaxis()->GetTitle() );
        }
      }

      void DrawHists2D(){
        for(auto hist : backgrounds){
          hist->SetMarkerColor( 1 );
          hist->SetLineColor( 1 );
        }
        for(auto hist : signals)
          hist->SetMarkerColor( hist->GetLineColor() );

             if(backgrounds.size()) backgrounds.at(0)->Draw("SCAT");
        else if( signals.size())    signals.at(0)->Draw("SCAT");

        for(auto hist : backgrounds) hist->Draw("same SCAT");
        for(auto hist : signals) hist->Draw("same SCAT");
      }

      TLegend * GetLegend(float x1=0.55, float y1=0.65, float x2=0.90, float y2=0.88){
        TLegend * legend = new TLegend(x1,y1,x2,y2);
        legend->SetFillColor(0);
        legend->SetFillStyle(3001);
        legend->SetLineColor(0);
        legend->SetTextFont(font) ;
        for(auto hist : signals)
          legend->AddEntry(hist, hist->GetTitle(), "l");
        for(auto hist : backgrounds)
          legend->AddEntry(hist, hist->GetTitle(), "f");
        return legend;
      }

      TLegend * GetLegend2D(float x1=0.55, float y1=0.75, float x2=0.90, float y2=0.88){
        TLegend * legend = new TLegend(x1,y1,x2,y2);
        legend->SetLineColor(1);
        legend->SetLineWidth(3);
        legend->SetTextFont(font) ;
        gStyle->SetLegendBorderSize(3);
        for(auto hist : signals)
          legend->AddEntry(hist, hist->GetTitle(), "lp");
        if(backgrounds.size())
          legend->AddEntry(backgrounds.at(0), "background", "lp");
        return legend;
      }

      TLegend * GetLegendCutStyle(string cut_text, float x1=0.55, float y1=0.65, float x2=0.90, float y2=0.88){
        TLegend * legend = new TLegend(x1,y1,x2,y2);
        legend->SetFillColor(0);
        legend->SetFillStyle(3001);
        legend->SetLineColor(0);
        legend->SetTextFont(font) ;
        legend->AddEntry(signals.at(0), signals.at(0)->GetTitle(), "l");
        legend->AddEntry(backgrounds.at(0), backgrounds.at(0)->GetTitle(), "l");
        legend->AddEntry((TObject*)0, cut_text.c_str(), "");
        legend->AddEntry(signals.at(1), signals.at(1)->GetTitle(), "l");
        legend->AddEntry(backgrounds.at(1), backgrounds.at(1)->GetTitle(), "l");
        return legend;
      }

      TLatex * GetText(string text_src, float x = 0.23, float y = 0.82, float text_size = 0.044){
        TLatex * text = new TLatex(x, y, text_src.c_str());
        text->SetNDC(kTRUE) ;
        text->SetTextSize(text_size) ;
        text->SetTextFont(font) ;
        return text;
      }

      TLatex * GetLeftText(string text_src){  return GetText(text_src, 0.16, 0.82, 0.034); }
      TLatex * GetRightText(string text_src){ return GetText(text_src, 0.45, 0.92, 0.040); }

      TCanvas * GetCanvas(string name, int size_x = 600, int size_y = 600){
        TCanvas * canvas = new TCanvas(name.c_str(), name.c_str(), size_x, size_y);
        canvas->SetTicks(1,1);
        canvas->SetLeftMargin(0.14); 
        canvas->SetRightMargin(0.08); 
        gPad->SetBottomMargin(0.20); 
        gStyle->SetOptStat(0000000);
        gStyle->SetTextFont(font);
        gStyle->SetOptTitle(0);
        if(logY) canvas->SetLogy();
        return canvas;
      }

      TCanvas * GetCanvas2D(string name, int size_x = 800, int size_y = 600){
        TCanvas * canvas = new TCanvas(name.c_str(), name.c_str(), size_x, size_y);
        canvas->SetRightMargin(0.18);
        canvas->SetLeftMargin(0.22);
        canvas->SetBottomMargin(0.18);
        canvas->SetTopMargin(0.12);
        return canvas;
      }


      void Add(TMP_hist_type* hist, bool signal=true){
        if(signal) signals.push_back( hist );
        else       backgrounds.push_back( hist );
      }

      void AddCummulative(TMP_hist_type* hist, bool signal=true, int index=0){
        if(signal){ 
          if(index >= signals.size()) signals.push_back( hist ); 
          else                        signals.at(index)->Add( hist );
        }
        else{ 
          if(index >= backgrounds.size()) backgrounds.push_back( hist ); 
          else                            backgrounds.at(index)->Add( hist );
        }
      }

      vector<TMP_hist_type*> signals;
      vector<TMP_hist_type*> backgrounds;
      int font;
      bool logY;
      string label_x, label_y;
      double xmin, xmax, ymin, ymax;
  };

  void set_style_FCC(){
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
    gStyle->SetTitleOffset(1.95,"Y");
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

  template<class T>
  TCanvas * draw_hists_FCC( HistDrawer<T> * drawer, string path, string name, string label, string extra_title = "", string mode = "default"){
    set_style_FCC();
    string leftText  = "#it{#sqrt{s} = 100 TeV, L = 30 ab^{-1}}";
    string rightText = "RECO: Delphes-3.4.2";
           rightText = "#bf{FCC-hh Simulation (Delphes)}";
  
    cout << name << endl;
    drawer->Print();

    TCanvas * canvas;
    if(mode == "default" or mode == "default nolog"){
      drawer->logY = true;
      if(mode == "default nolog") drawer->logY = false;
      drawer->label_x = label;
      drawer->SetStyle();
      canvas = drawer->GetCanvas( name );
      drawer->DrawHists();
      auto legend = drawer->GetLegend();
      auto ltext  = drawer->GetLeftText( leftText );
      auto rtext  = drawer->GetRightText( rightText );
      auto etext  = drawer->GetText(extra_title, 0.23, 0.92);
      legend->Draw();
      ltext->Draw() ;
      rtext->Draw() ;
      etext->Draw() ;
    } 
    else if(mode == "tmva"){
      drawer->logY = false;
      drawer->label_x = label;
      drawer->SetStyle();
      canvas = drawer->GetCanvas( name );
      drawer->DrawHistsTMVA();
      auto legend = drawer->GetLegend();
      auto ltext  = drawer->GetLeftText( leftText );
      auto rtext  = drawer->GetRightText( rightText );
      auto etext  = drawer->GetText(extra_title, 0.23, 0.92);
      legend->Draw();
      ltext->Draw() ;
      rtext->Draw() ;
      etext->Draw() ;
    } else if (mode == "correlation"){
      drawer->logY = false;
      drawer->SetStyle();
      canvas = drawer->GetCanvas2D( name );
      drawer->DrawHists2D();
      auto legend = drawer->GetLegend2D();
      auto ltext  = drawer->GetLeftText( extra_title );
      auto rtext  = drawer->GetRightText( rightText );
      auto etext  = drawer->GetText("", 0.23, 0.92);
      legend->Draw();
      ltext->Draw() ;
      rtext->Draw() ;
      etext->Draw() ;
    } else if (mode == "cut"){
      drawer->logY = true;
      drawer->label_x = label;
      drawer->SetStyle();
      canvas = drawer->GetCanvas( name );
      drawer->DrawHists();
      auto legend = drawer->GetLegendCutStyle( extra_title );
      auto ltext  = drawer->GetLeftText( leftText );
      auto rtext  = drawer->GetRightText( rightText );
      // auto etext  = drawer->GetText(extra_title, 0.23, 0.92);
      legend->Draw();
      ltext->Draw() ;
      rtext->Draw() ;
      //etext->Draw() ;
    } else {
      msg_err("draw_hists_FCC(): wrong draw mode = ", mode);
    }
    canvas->SetTicks(1,1);

    canvas->RedrawAxis();
    canvas->GetFrame()->SetBorderSize( 12 );
    canvas->Modified();
    canvas->Update();
    canvas->Print( (path + "/" + name).c_str() );
    canvas->Write();

    return canvas;
  }






  class BrasilDrawer{
    public:
    TCanvas * draw_brasil_plot(vector<double> * x_axis, vector<double> * sigma_2_down, vector<double> * sigma_1_down, vector<double> * central, vector<double> * sigma_1_up, vector<double> * sigma_2_up){
      HistDrawer<TH1D> * drawer = new HistDrawer<TH1D>();
      drawer->logY = true;
      TCanvas * canvas = drawer->GetCanvas("limits");
      canvas->SetLeftMargin(0.22); 

      TGraph * gr_central = new TGraph( central->size() );
      FillGraph gr_2sigma, gr_1sigma;

      for(int i = 0; i < central->size(); i++){
        gr_central->SetPoint( i, x_axis->at(i), central->at(i) );
        gr_2sigma.AddPoint( x_axis->at(i), sigma_2_down->at(i), sigma_2_up->at(i) );
        gr_1sigma.AddPoint( x_axis->at(i), sigma_1_down->at(i), sigma_1_up->at(i) );
      }

      TGraph * gr_1_sigma = gr_1sigma.GetGraph();
      TGraph * gr_2_sigma = gr_2sigma.GetGraph();

      set_style_FCC();
        gr_1_sigma->GetYaxis()->CenterTitle();
        gr_1_sigma->GetYaxis()->SetNdivisions(510);

        gr_1_sigma->GetYaxis()->SetLabelFont(132);
        gr_1_sigma->GetYaxis()->SetLabelOffset(0.02);
        gr_1_sigma->GetYaxis()->SetLabelSize(0.04);
        gr_1_sigma->GetYaxis()->SetTitleFont(132);
        gr_1_sigma->GetYaxis()->SetTitleOffset(1.5);
        gr_1_sigma->GetYaxis()->SetTitleSize(0.045);

        gr_1_sigma->GetXaxis()->SetLabelFont(132);
        gr_1_sigma->GetXaxis()->SetLabelOffset(0.02);
        gr_1_sigma->GetXaxis()->SetLabelSize(0.04);
        gr_1_sigma->GetXaxis()->SetTitleFont(132);
        gr_1_sigma->GetXaxis()->SetTitleOffset(1.5);
        gr_1_sigma->GetXaxis()->SetTitleSize(0.045);

      gr_2_sigma->SetFillColor(kYellow);
      gr_2_sigma->GetXaxis()->SetRangeUser(xmin, xmax);
      gr_2_sigma->GetXaxis()->SetTitle(label_x.c_str());
      gr_2_sigma->GetYaxis()->SetTitle(label_y.c_str());
      gr_2_sigma->Draw("AF");

      gr_1_sigma->SetFillColor(kGreen);
      gr_1_sigma->Draw("F");

      gr_central->SetLineStyle(2);
      gr_central->SetLineWidth(2);
      gr_central->Draw("L");


      TLegend * leg  = new TLegend(0.52,0.65,0.90,0.85);
      leg->AddEntry(gr_central, "95% CL Expected Limit", "l");
      leg->AddEntry(gr_1_sigma, "#pm 1 std. deviation", "f");
      leg->AddEntry(gr_2_sigma, "#pm 2 std. deviation", "f");
      leg->Draw();

      string rightText = "#bf{FCC-hh Simulation (Delphes)}";
      auto rtext  = drawer->GetRightText( rightText );
      rtext->Draw() ;

      canvas->RedrawAxis();
      canvas->GetFrame()->SetBorderSize( 12 );
      return canvas;
    }

    int font;
    bool logY, logX;
    string label_x, label_y;
    double xmin, xmax, ymin, ymax;
  };





};

#endif









