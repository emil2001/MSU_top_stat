#ifndef getTableCombH
#define getTableCombH

class MyParameter {
  public:
    // collect here all information associated with parameter
    MyParameter(TTree *tree, string n, double bfrac, string postfix="");
    
    void ReadEntrie(double weight);
    double GetMean();
    double GetRMS();
    double GetCovariance(MyParameter * other);
    double GetCorrelation(MyParameter * other);

    int burn_events;
    double val, mean, rms, burn_frac;
    string name;
    TH1D * hist;
    vector<double> vals;
    vector<double> weights;
};

void ReplaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace);

#endif