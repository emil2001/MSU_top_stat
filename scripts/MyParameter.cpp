class MyParameter {
  public:
    // collect here all information associated with parameter
    MyParameter(TTree *tree, string n, double bfrac, string postfix="") : name(n){
      tree->SetBranchAddress(n.c_str(), &val);
      if( name == "KU" or name == "KC" or name == "cta_norm" or name == "uta_norm")
        hist = new TH1D((n + " bf= "+ to_string(bfrac) + postfix).c_str(), (n + postfix).c_str(), 100000,    0, 1.); // FIXME
      else 
        hist = new TH1D((n + " bf= "+ to_string(bfrac) + postfix).c_str(), (n + postfix).c_str(), 100000, -10, 10.); // FIXME
      mean = -1999, rms = -1999;

      burn_frac   = bfrac;
      burn_events = burn_frac * tree->GetEntriesFast();
    };

    void ReadEntrie(double weight){
      if(burn_events >= 0) {burn_events -= 1; return;}
      hist->Fill( val, weight );
      weights.push_back(weight);
      vals.push_back(val);
    }

    double GetMean(){
      if(mean > -999) return mean;
      mean = 0;
      for(int i = 0; i < vals.size(); i++) mean  += vals.at(i) * weights.at(i);
      mean /= std::accumulate(weights.begin(), weights.end(), 0.);
      return mean;
    }

    double GetRMS(){
      if(rms > -999) return rms;
      rms = 0;
      mean = GetMean();
      for(int i = 0; i < vals.size(); i++) 
        rms  += pow(vals.at(i) - mean, 2) * weights.at(i);
      rms /= std::accumulate(weights.begin(), weights.end(), 0.);
      rms = sqrt(rms);
      return rms;
    }

    double GetCovariance(MyParameter * other){
      double mean_x = GetMean();
      double mean_y = other->GetMean();
      double cov = 0;
      for(int i = 0; i < vals.size(); i++){
        cov += (vals.at(i) - mean_x) * (other->vals.at(i) - mean_y) * weights.at(i);
      }
      cov /= std::accumulate(weights.begin(), weights.end(), 0.);
      return cov;
    }

    double GetCorrelation(MyParameter * other){
      double cor = GetCovariance(other) / (GetRMS() * other->GetRMS()); 
      return cor;
    }
    
    int burn_events;
    double val, mean, rms, burn_frac;
    string name;
    TH1D * hist;
    vector<double> vals;
    vector<double> weights;
};

void ReplaceStringInPlace(std::string& subject, const std::string& search,
                          const std::string& replace) {
    size_t pos = 0;
    while((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
}
template <typename T>
std::string get_string(const T a_value, const int n = 6)
{
    std::ostringstream out;
    out << std::setprecision(n) << a_value;
    return out.str();
}

template <typename T>
std::string get_perc(const T a_value, const int n = 6)
{
    std::ostringstream out;
    out << std::setprecision(n) << a_value * 100;
    return out.str() + "\\%";
}
