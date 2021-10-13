
#ifndef mRootAnalyser_hh
#define mRootAnalyser_hh 1

#include "msg.hh"

namespace mRoot {
 
  template<class TreeType> class EventReader {
    public:
      ~EventReader(){
        for(auto it = vars.begin(); it != vars.end(); it++) delete it->second;
      }

      void Init(TFile * file, string tree_name){
        // reset
        for(auto it = vars.begin(); it != vars.end(); it++){
          delete it->second;
        }
        vars.clear();

        // init new tree
        if( file == nullptr ){
          msg_err( "mRoot::EventReader.Init() : TFile * file =", file );
          return;
        }

        tree = (TTree*) file->Get( tree_name.c_str() );
        if(tree == nullptr){
          msg_err( "mRoot::EventReader.Init() : cant find tree ", "\"" + tree_name + "\"", "in file", file->GetName() );
          file->ls();
          file->Print();
          return;
        }

        TObjArray *brs = tree->GetListOfBranches();
        for(int i = 0; i < brs->GetEntries(); ++i){ 
          AddVar( brs->At(i)->GetName() ); 
        }

        for(auto it = vars.begin(); it != vars.end(); it++){
          tree->SetBranchAddress( it->first.c_str(), it->second);
        }
      }

      void AddVar(string name){
        vars[ name ] = new TreeType;
      }
      void AddVars(vector<string> names){
        for(auto name : names) AddVar( name );
      }

      TreeType * GetVar(string name){
        return vars.find( name )->second;
      }

      map<string, TreeType*> vars;
      TTree * tree;
  };




  class FileHists{
    public:
    map<string, TH1D*> hists_1d;
    map<string, TH2D*> hists_2d;

    TH1D * get_th1d(string key){
      auto it = hists_1d.find( key );
      if(it != hists_1d.end()) return it->second;
      return nullptr;
    }

    TH2D * get_th2d(string key){
      auto it = hists_2d.find( key );
      if(it != hists_2d.end()) return it->second;
      return nullptr;
    }

    void Save(){
      TTree * metadata = new TTree("FileHists_metadata", "FileHists_metadata");
      string key, hist_name;
      metadata->SetBranchAddress( "key", &key);
      metadata->SetBranchAddress( "hist_name", &hist_name);
      for(auto it = hists_1d.begin(); it != hists_1d.end(); it++){
        TH1D* hist = it->second;
        key = it->first;
        hist_name = hist->GetName();
        metadata->Fill();
        hist->Write();
      }

      for(auto it = hists_2d.begin(); it != hists_2d.end(); it++){
        TH2D* hist = it->second;
        key = it->first;
        hist_name = hist->GetName();
        metadata->Fill();
        hist->Write();
      }

      metadata->Write();
    }

    void Load(TFile * file){
      TTree * metadata = (TTree*) file->Get( "FileHists_metadata" );
      string key, hist_name;
      metadata->SetBranchAddress( "key", &key);
      metadata->SetBranchAddress( "hist_name", &hist_name);

      vector<TH1D*> * h1ds = get_all_with_type<TH1D>(file, "TH1D");
      vector<TH2D*> * h2ds = get_all_with_type<TH2D>(file, "TH2D");

      int nevents = metadata->GetEntries();
      for(int i = 0; i < nevents; i++){
        metadata->GetEntry(i);
        for(TH1D* h1d : *h1ds){
          if(hist_name != string( h1d->GetName() ) ) continue;
          hists_1d[key] = h1d;
          break;
        }
        for(TH2D* h2d : *h2ds){
          if(hist_name != string( h2d->GetName() ) ) continue;
          hists_2d[key] = h2d;
          break;
        }
      }
    }
  };


  












  class HistsDatabase{
    public:
      map<string, TH1D*> hists_map;

      

      vector<vector<string>> super_keys;
      vector<TH1D*> entries;
/*
      TH1D * GetHist(vector<string> key){
        int index = 0;
        for(index = 0; index < super_keys.size(); index++)
          if(super_keys[index] == key) break;
        if(index < entries.size()) return entries.at(index);
        return nullptr;
      }

      vector<string> GetUnicKeys(int level){
        vector<string> answer;
        for(auto keys : super_keys){
          if(level >= keys.size() ) continue;
          string key = keys.at(level);
          if( not count(answer.begin(), answer.end(), key) ) answer.push_back( key );
        }
        return answer;
      }

      void AddHist(string superkey, TH1D * hist){
        entries.push_back(hist);
        super_keys.push_back( split_string(superkey, "@") );
      }

      void AddHistsFromFile(TFile * file){
        auto keys = file->GetListOfKeys();
        for(int i = 0; i < keys->GetSize(); i++){
          auto obj =  ((TKey*) keys->At(i))->ReadObj();
          if( string(obj->ClassName()) == string("TH1D")) AddHist( (TH1D*)obj );
        }
      }

      vector<TH1D*> GetHists(string pattern){
        vector<TH1D*> answer;
        regex re( pattern );
        for( auto hist : entries ) 
          if( regex_match(hist->GetName(), re) )
            answer.push_back(hist);
        return answer;
      }
*/
  };


};

#endif
