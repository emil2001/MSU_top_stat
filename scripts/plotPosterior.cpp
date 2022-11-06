#include "mroot/mRoot.cpp"
#include "mroot/msg.hh"
#include "MyParameter.cpp"
#include "getQuantiles.cpp"



class MyParameterPost: public MyParameter{
    public:
    MyParameterPost(TTree *tree, string n, double bfrac, string postfix=""): MyParameter(tree, n, bfrac, "") {};
    void ResetTree(TTree *tree, string n){
        tree->SetBranchAddress(n.c_str(), &val);
        burn_events += burn_frac * tree->GetEntriesFast();
    }
};


struct cov_item  //struct responsible for collecting computed values
{
    int x;
    double variance;
    double correlation;
    cov_item(int ax = 0, double avariance = 0, double acorrelation = 0) 
      : 
      x(ax), 
      variance(avariance), 
      correlation(acorrelation) 
      {}
};

void SetLabels(TH2D* hist, const vector <MyParameterPost*> &parameters)
{
    for(int i=1; i <= hist->GetNbinsX(); i++) hist->GetXaxis()->SetBinLabel(i, parameters.at(i-1)->name.c_str() );
    for(int i=1; i <= hist->GetNbinsX(); i++) hist->GetYaxis()->SetBinLabel(i, parameters.at(i-1)->name.c_str() );
}


// Creates, fills, and saves covariance and correlation histograms
void fillCovHisto(const vector <MyParameterPost*> &parameters, string postfix) { 
    // Create a pool of threads
    ROOT::TProcessExecutor pool(32); 

    TH2D* hist_cov = new TH2D("Covariance", "Covariance", parameters.size(), 0, parameters.size(), parameters.size(), 0, parameters.size());
    TH2D* hist_cor = new TH2D("Correlation", "Correlation", parameters.size(), 0, parameters.size(), parameters.size(), 0, parameters.size());

    // Lambda function for each thread
    auto fillCovHisto = [&parameters](int x) {
        vector<cov_item> variance_v;
        for(int y = 0; y < parameters.size(); y++){
            double variance = parameters.at(y)->GetCovariance( parameters.at(x) );
            double correlation = parameters.at(y)->GetCorrelation( parameters.at(x) );
            variance_v.push_back(cov_item(x, variance, correlation));
            if(x == y) mRoot::msg( x, y, variance );
        }
        return variance_v;
    };
    // vector of args for function
    vector<int> v(parameters.size()) ; 
    iota (begin(v), end(v), 0); 

    // Fill the pool with work
    auto variance_vv = pool.Map(fillCovHisto, v);

    // Fill hists with computed values
    for(int x = 0; x < parameters.size(); x++){
        for(int y = 0; y < parameters.size(); y++){
            auto v = variance_vv[x][y];
            hist_cov->Fill(v.x, y, v.variance);
            hist_cor->Fill(v.x, y, v.correlation );
        }                
    }

    SetLabels(hist_cov, parameters);
    SetLabels(hist_cor, parameters);

    // Plotter block
    auto canv = mRoot::plotCorrelationHist( hist_cov, false, 2 );
    string cov_hist_out_name = "Cov" + postfix + ".png";
    canv->Print( cov_hist_out_name.c_str() );

    canv = mRoot::plotCorrelationHist( hist_cor, 2 );
    string cor_hist_out_name = "Cor" + postfix + ".png";
    canv->Print(cor_hist_out_name.c_str());
};

void treeReader(vector <MyParameterPost*> &parameters, TFile &file, double def_bfrac, int nchains) {
    int chain_index = 0;
    int par_index = 0;
    while(true){

        chain_index++;
        string cname = "chain_" + to_string( chain_index );
        TTree *tree = (TTree*)(file.Get( cname.c_str() ));
        if(not tree || chain_index == nchains + 1) {
            cout << "break at chain no." << chain_index << endl;
            break;
        }
        
        TObjArray * mycopy = tree->GetListOfBranches();
        TIter iObj(mycopy);
        par_index = 0;
        while (TObject* obj = iObj()) {
            
            if(obj->GetName() == string("weight")) continue;
            if(obj->GetName() == string("nll_MarkovChain_local_")) continue;
            if(chain_index == 1)
            {
                MyParameterPost * parameter = new MyParameterPost(tree, obj->GetName(), def_bfrac);
                parameters.push_back( parameter );
            }
            else
            {
                parameters.at(par_index)->ResetTree(tree, obj->GetName());
            }
            par_index++;
        }

        Int_t weight;
        
        tree->SetBranchAddress("weight",  &weight);
        for(int l = 0; l < (int)tree->GetEntries(); ++l){
            
            tree->GetEntry(l);
            for(auto param : parameters) {
                param->ReadEntrie( weight );
            }
        }
        cout << "Read chain " << chain_index << endl;
        
    }
}


int plotPosterior (string filename, string postfix, double def_bfrac, int nchains) {
    // Required for proper multithreading
    ROOT::EnableThreadSafety();
    // Timer for debug
    TStopwatch t; 
    t.Start();

    vector <MyParameterPost*> parameters;
    // Read chains from file 
    TFile file( filename.c_str() );
    treeReader(parameters, file, def_bfrac, nchains);
    cout << "Outside hist size "<< parameters[0]->hist->Integral() << endl;

    t.Stop();
    cout << "Time for reading TTree" << endl;
    t.Print();
    t.Reset();

    ofstream fout;
    fout.open("quantiles.txt");        
    double alpha = 0.3173;
    double l,c,u;
    string chains_path = ("chains_" + postfix);
    ReplaceStringInPlace(chains_path, string("_"), string("X"));
    gSystem->mkdir( chains_path.c_str() );
    for (auto param: parameters)
    {
        TH1D * hist = param->hist;
        l = get_qv(hist, alpha*0.5 );
        c = get_qv(hist, 0.5) ;
        u = get_qv(hist,  1. - alpha*0.5 );
        fout << hist->GetTitle() << " " << get_string(l, 3) << " " << get_string(c, 3) << " " << get_string(u, 3) << endl;  
    }

    t.Start();

    fillCovHisto(parameters, postfix);

    t.Stop();
    cout << "Time for computing covariation/correlation" << endl;           
    t.Print();

    fout.close();  
    return true;
}