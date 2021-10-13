
#include "/afs/cern.ch/user/p/pmandrik/public/global_cfg/mRoot.cpp"
#include "/afs/cern.ch/user/p/pmandrik/public/global_cfg/msg.hh"

#include "tdrstyle.C"

struct point{
  double x, y, r;
  double weight;
};

void plot2D(string path, string mode = "branch"){
  setTDRStyle();

  // string mode = "branch"; // "kappas" "branch"

  TH2D * hist = new TH2D("2d", "", 100, 0, 6, 100, 0, 4);
  if(mode != "kappas")
    hist = new TH2D("2d", "", 100, 0, 36, 100, 0, 8);

  TFile * file = TFile::Open( path.c_str() );

  TTree *te = (TTree*)file->Get("chain_1");
  Double_t   kce, kue;
  Int_t      weighte;
  te->SetBranchAddress("fcnc_tcg_norm",  &kce);
  te->SetBranchAddress("fcnc_tug_norm",  &kue);
  te->SetBranchAddress("weight",         &weighte);

  vector<point> points;
  double bfrac = 0.1;
  double sum_x = 0;
  double sum_y = 0;
  double integral = 0;
  for(int l = bfrac * te->GetEntries(); l < (int)te->GetEntries();++l){
    te->GetEntry(l);
    double val_x, val_y;
    val_x = 0.03*TMath::Sqrt(kce) * 1000;
    val_y = 0.03*TMath::Sqrt(kue) * 1000;

    if(mode != "kappas"){
      val_y = 1.198 * val_y * val_y;
      val_x = 1.195 * val_x * val_x;
    }

    sum_x += val_x;
    sum_y += val_y;
    
    point p;
    p.x = val_x;
    p.y = val_y;
    p.weight = weighte;
    points.push_back( p );

    // hist->Fill(val_x, val_y, weighte);
    integral += weighte;
  }
  double koff = sum_x / sum_y;
  for(int i = 0; i < points.size(); i++){
    point * p = & points.at(i);
    p->r = TMath::Sqrt(pow(p->x, 2) + pow(koff * p->y, 2));
  }

  std::sort(points.begin(), points.end(), [] (const point & s1, const point & s2) -> bool{ return s1.r < s2.r; } );

  point last = points.at( 0 );
  double current_integral = 0;
  double max_x = 0;
  double max_y = 0;
  for(auto point : points){
    current_integral += point.weight;
    cout << point.r << " " << current_integral << " " << integral << endl;
    max_x = TMath::Max(max_x, point.x);
    max_y = TMath::Max(max_y, point.y);
    hist->Fill(point.x, point.y);
    if( current_integral >= 0.95 * integral){
      break;
    }
  }
  TEllipse * elips = new TEllipse(0, 0, max_x, max_y, 0, 90);
  elips->SetFillColor(38);
  //elips->SetFillStyle(3002);
  //elips->SetLineStyle(0);
  elips->SetLineWidth(2);
  //elips->SetFillStyle(0);
  cout << "here " << last.x << " " << last.y << endl;

  current_integral = 0;
  max_x = 0;
  max_y = 0;
  for(auto point : points){
    current_integral += point.weight;
    cout << point.r << " " << current_integral << " " << integral << endl;
    max_x = TMath::Max(max_x, point.x);
    max_y = TMath::Max(max_y, point.y);
    hist->Fill(point.x, point.y);
    if( current_integral >= 0.68 * integral){
      break;
    }
  }

  TEllipse * elips_68 = new TEllipse(0, 0, max_x, max_y, 0, 90);
  elips_68->SetFillColor(9);
  elips_68->SetLineWidth(2);
  //elips_68->SetFillStyle(3002);
  //elips_68->SetFillStyle(0);
  elips_68->SetLineStyle(2);

  hist->GetXaxis()->SetTitle("#frac{#kappa^{tcg}}{#Lambda}, 10^{-3} [TeV^{-1}]");
  hist->GetYaxis()->SetTitle("#frac{#kappa^{tug}}{#Lambda}, 10^{-3} [TeV^{-1}]");
  hist->GetXaxis()->SetTitleOffset(1.25);
  hist->GetYaxis()->SetTitleOffset(1.75);
  hist->GetXaxis()->SetTitleSize(.05);
  hist->GetYaxis()->SetTitleSize(.05);

  if(mode != "kappas"){
    hist->GetXaxis()->SetTitle("Br(t #rightarrow cg), 10^{-6}");
    hist->GetYaxis()->SetTitle("Br(t #rightarrow ug), 10^{-6}");
    //hist->GetXaxis()->SetTitle("#it{B}(t #rightarrow cg), 10^{-6}");
    //hist->GetYaxis()->SetTitle("#it{B}(t #rightarrow ug), 10^{-6}");
  }

  cout << "here " << last.x << " " << last.y << endl;
  TCanvas * canv = new TCanvas( "2d", "2d", 600, 600);
  cout << "here " << last.x << " " << last.y << endl;
  hist->Draw("AXIS");
  elips->Draw("only");
  elips_68->Draw("only");

  canv->RedrawAxis();
  canv->SetLeftMargin(0.20);
  canv->SetTopMargin(0.10);
  canv->SetBottomMargin(0.15);

  TLegend * leg  = new TLegend(0.34,0.63,0.875,0.86);
  leg->SetHeader("CMS Phase-2 Simulation");
  // leg->AddEntry((TObject*)0, "3000 fb^{-1}, #sqrt{s} = 14 TeV", "");
  leg->AddEntry(elips,  "95% CL Expected Limit", "lf");
  leg->AddEntry(elips_68, "68% CL Expected Limit", "lf");
  leg->SetBorderSize(0);
  leg->SetFillStyle(0);
  leg->Draw();

  cout << "print" << endl;

  TLatex Tl;
  Tl.SetTextFont(42);
  Tl.SetTextAlign(13);
  Tl.SetTextSize(0.045);

  gStyle->SetLabelSize(0.055, "XYZ");
  Tl.DrawLatexNDC(0.60, 0.96,"3000 fb^{-1} (14 TeV)");

  canv->Print( ("contour2d_" + mode + ".png").c_str() );
  canv->Print( ("contour2d_" + mode + ".pdf").c_str() );
}

