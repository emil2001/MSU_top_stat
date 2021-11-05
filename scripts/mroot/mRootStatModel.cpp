
#ifndef mRootStatModel_hh
#define mRootStatModel_hh 1

// MROOT

namespace mRoot{

  //
  //  Statistic model :
  //    Chanals - nominal hist + {normalisation parameters} + {interpolation parameters + {up, down hists} }
  //

  class Chanal{
    public:
    TH1D* nom_hist;
    vector<pair<TH1D*, TH1D*>> interp_hists;

    string name;
    vector<string> norm_pars;
    vector<string> interp_pars;
  };

  class StatModel{
    public:
    TH1D * GetHist(string name){
      auto chanal = find_if(chanals.begin(), chanals.end(), [name] (const Chanal & ch) { return ch.name == name; } );
      if(chanal == chanals.end()) return nullptr;
      TH1D * hist = (TH1D *)chanal->nom_hist->Clone();
      cout << name << endl;
      // find value in each bin
      //cout << "find value in each bin " << name << endl;
      for(int bin = 1; bin <= hist->GetXaxis()->GetNbins(); bin++){  
      
        double nom_val = hist->GetBinContent(bin);
        double bin_val = nom_val;

        
        // interpolation
        if( abs(nom_val) > 0.00000001 ){// 0.00000001
         // cout << "interpolation " << chanal->interp_pars.size() << endl;
          for(int i = 0; i < chanal->interp_pars.size(); i++){
            TH1D * hist_up   = chanal->interp_hists.at(i).first;
            TH1D * hist_down = chanal->interp_hists.at(i).second;
    
            double up_val   = hist_up->GetBinContent(bin);
            double down_val = hist_down->GetBinContent(bin);
            double par_val = GetParameter(chanal->interp_pars.at(i));
            if(par_val > 0) bin_val = bin_val * pow( up_val   / nom_val, TMath::Abs(par_val));
            else            bin_val = bin_val * pow( down_val / nom_val, TMath::Abs(par_val));
            if (name == "Wc") {
                cout << chanal->interp_pars.at(i) << endl;
        	cout << "up_val " << up_val << endl << "down_val " << down_val << endl;
        	cout << "bin_val " << bin_val << endl;
        	
            }
            if (bin_val != bin_val) bin_val = 0.;
          }
        }

        double bin_val_prenorm = bin_val;
        // normalisation
        //cout << "normalisation" << endl;
        for(int i = 0; i < chanal->norm_pars.size(); i++){
          bin_val *= GetParameter(chanal->norm_pars.at(i));
        }
      
        cout << bin << " " << nom_val << " -> " << bin_val_prenorm << " -> " << bin_val << endl;
        hist->SetBinContent(bin, bin_val);
      }

      return hist;
    }

    void SetParameter(string name, double value){
      auto it = parameters.find(name);
      if(it == parameters.end()) return;
      it->second = value;
    }

    double GetParameter(string name){
      auto it = parameters.find(name);
      if(it == parameters.end()){ 
        cout << "mRoot::StatModel.GetParameter( " << name << " ) cant find parameter" << endl;
        return -999;
      }
      return it->second;
    }

    // here we will keep parameters values
    map <string, double> parameters;

    // here we will kepp parameters and asociated hists 
    vector<Chanal> chanals;
  };

};

#endif















