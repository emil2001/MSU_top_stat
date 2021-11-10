///////////////////////////////////////////////////////////////////////////////
//  Author      :     P.S. Mandrik, IHEP
//  Date        :     11/05/20
//  Last Update :     11/05/20
//  Version     :     1.0
///////////////////////////////////////////////////////////////////////////// 

#ifndef PMLIB_TREE_TO_HIST_HH
#define PMLIB_TREE_TO_HIST_HH 1

namespace pm {
  
  double get_ttree_integral(std::string input_file_name, std::string tree_name, std::string weight_rule){
    double answer = 0;
    TFile * file = TFile::Open( input_file_name.c_str() );
    
    if(!file or file->IsZombie()){
      cerr << __FUNCTION__ << ": can't open file name \"" << input_file_name << "\", skip ... " << endl;
      file->Close();
      return -1;
    }

    TTreeReader * reader = new TTreeReader(tree_name.c_str(), file);
    if(!reader->GetTree()){
      cerr << __FUNCTION__ << ": can't get ttree \"" << tree_name << "\" in file \"" << file->GetName() << "\", skip ... " << endl;
      file->Close();
      return -1;
    }
    
    TTreeFormula weight_f(weight_rule.c_str(), weight_rule.c_str(), reader->GetTree());
    if( weight_f.GetNdim()==0 ){
      reader->GetTree()->Print();
      cerr << __FUNCTION__ << ": can't create weight rule \"" << tree_name << "\" in file \"" << file->GetName() << "\", skip ... " << endl;
      return -1;
    }
    
    while(reader->Next()){
      answer += weight_f.EvalInstance();
    }
    
    file->Close();
    return answer;
  }
  
  double get_ttree_integral(string prefix, std::vector<std::string> input_file_names, std::string tree_name, std::string weight_rule){
    double answer = 0;
    for(auto name : input_file_names){
      answer += get_ttree_integral(prefix + name, tree_name, weight_rule);
    }
    return answer;
  }
  
};

#endif

