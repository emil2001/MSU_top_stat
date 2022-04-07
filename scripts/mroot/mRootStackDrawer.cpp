
#ifndef mRootStackDrawer_
#define mRootStackDrawer_ 1

#include <TFile.h>
#include <TTree.h>
#include <TH1.h>
#include <TROOT.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TLegend.h>
#include <TRegexp.h>

#include <vector>
#include <string>
#include <regex>

#include "mRootNiceColors.cpp"

namespace mRoot {
  class StackDrawer{
    public : 
      StackDrawer(){
        draw_residial = true;
        legend_width    = 0.25;
        data_hist = nullptr;
      } 

      void AddHistsFromFile(TFile * file, TString incl_wildcard=".*", TString excl_wildcard="QWERTZXZCAD"){
        data_hist = nullptr;
        data_hist = (TH1D*)file->Get("data");
        TIter next(file->GetListOfKeys());
        TKey *key;

        std::regex re_incl(incl_wildcard);
        std::regex re_excl(excl_wildcard);

        while ((key = (TKey*)next())) {
          string name = key->GetName();
          if( not std::regex_match(name, re_incl) ) continue;
          if( std::regex_match(name, re_excl) ) continue;

          TClass *cl = gROOT->GetClass(key->GetClassName());
          if (!cl->InheritsFrom("TH1")) continue;
          TH1D *h = (TH1D*)key->ReadObj();
          if( string(h->GetName()) == string("data")) continue;

          cout << "StackDrawer add " << name << endl;
         // cout << stack_hists.size() << endl;
          stack_hists.push_back(h);
        }
      }

      TCanvas * GetCanvas(const char * name, int width = 640 * 2, int height = 480 * 2){
        TCanvas * canv = new TCanvas(name, "", width, height);

        int textFont = 63;
        double axisFontSize = 20;

        if(not data_hist) draw_residial = false;
        double residial_height = draw_residial ? 0.33 : 0.0;

        std::vector <int> colors = mRoot::getNiceColors(200);
        int color = 0;
        int signal_color = 2;
        hs = new THStack("stack", name);
        TLegend * leg;
        if(draw_residial) leg = new TLegend(0.00,0.00,0.90,0.90);
        else              leg = new TLegend(0.00,0.10,0.90,0.90);
        leg->SetTextSize(22);
        leg->SetTextFont(textFont);
        if(data_hist) leg->AddEntry(data_hist, data_hist->GetTitle(), "lp");
        for(auto hist : signal_hists){
          hist->SetLineColor( signal_color++ );
          hist->SetLineWidth( 5 );
          hist->SetLineStyle( 7 );
          leg->AddEntry(hist, hist->GetTitle(), "l");
        }
        cout << stack_hists.size() << endl;
        int i = 0;
        for(auto hist : stack_hists){          
          hs->Add(hist);          
          hist->SetFillColor( colors.at(color++) );
          hist->SetLineColor( 1 );
          hist->SetLineWidth( 2 );          
          leg->AddEntry(hist, hist->GetTitle(), "f");
          cout << hist->GetTitle() << endl;
        }
        cout << "END" << endl;
        TPad *pad_main = new TPad("p1","p1", 0.00, residial_height, 1.-legend_width-0.01, 1.0);
        pad_main->Draw();
        pad_main->cd();
        hs->Draw("hist");
        for(auto hist : signal_hists) hist->Draw("same hist");
        if(data_hist) {
          data_hist->SetMarkerStyle(20);
          data_hist->Draw("p0e1 same");
        }

        hs->GetYaxis()->SetLabelFont(textFont);
        hs->GetYaxis()->SetLabelSize(axisFontSize);
        hs->GetXaxis()->SetLabelFont(textFont);
        hs->GetXaxis()->SetLabelSize(axisFontSize);

        canv->cd();
        TPad *pad_leg = new TPad("p2","p2", 1.-legend_width, residial_height, 1., 1.0);
        pad_leg->Draw();
        pad_leg->cd();
        leg->Draw();
        pad_leg->SetLeftMargin(0.00001);
        pad_main->SetRightMargin(0.00001);

        if(draw_residial and data_hist){
          canv->cd();
          TPad *pad_res = new TPad("p3","p3", 0.0, 0.0, 1.-legend_width-0.01, residial_height);
          pad_res->Draw();
          pad_res->cd();
          TH1D * rhist = new TH1D(*data_hist);
          rhist->Sumw2();

          TH1D total_stack_hist(*(stack_hists.at(0)));
          total_stack_hist.Sumw2();
          for(int i = 1; i < stack_hists.size(); i++) total_stack_hist.Add(stack_hists.at(i));

          rhist->Add(&total_stack_hist, -1.);
	  rhist->Divide(data_hist);
          rhist->Draw("p0e1");
          rhist->SetStats(0);
          rhist->SetTitle("");

          pad_main->SetBottomMargin(0.00001);
          pad_res->SetTopMargin(0.00001);
          pad_res->SetBottomMargin(0.1);
          pad_res->SetRightMargin(0.00001);
          hs->GetXaxis()->SetLabelOffset(999.);

          rhist->GetXaxis()->SetLabelFont(textFont);
          rhist->GetXaxis()->SetLabelSize(axisFontSize);
          rhist->GetYaxis()->SetLabelFont(textFont);
          rhist->GetYaxis()->SetLabelSize(axisFontSize);

          auto l = new TLine( 0., 0., 1., 0. );
          l->SetLineColor( 2 );
          l->SetLineWidth( 2 );
          l->Draw();

          canv->cd();
          pad_main->Draw();
        }

        return canv;
      }

      TCanvas * GetCanvas(string name, int width = 640 * 2, int height = 480 * 2){
        return GetCanvas(name.c_str(), width, height);
      }

      vector<TH1D*> stack_hists;
      vector<TH1D*> signal_hists;
      TH1D* data_hist;
      THStack *hs;
      bool draw_residial;
      double legend_width;
  };
};

#endif






