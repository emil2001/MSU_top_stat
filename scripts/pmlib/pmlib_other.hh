#ifndef PMLIB_OTHER_HH
#define PMLIB_OTHER_HH 1

namespace pm {

  //=================================== STRING MANIPULATION =============================================================
  static void ltrim(std::string &s) {
    if(not s.size()) return;
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) { return !std::isspace(ch); }));
  }

  static void rtrim(std::string &s) {
    if(not s.size()) return;
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) { return !std::isspace(ch); }).base(), s.end());
  }

  std::string strip(std::string s) {
    if(not s.size()) return s;
    ltrim(s);
    rtrim(s);
    return s;
  }

  void split_string(std::string str, std::vector<std::string> & answer, const std::string & sep = " "){
    int sep_size = sep.size();

    for(int i = 0; i + sep_size < str.size(); i++){
      if( sep == str.substr(i, sep_size) ){
        answer.push_back( str.substr(0, i) ); 
        str = str.substr(i + sep_size, str.size() - i - sep_size);
        i = 0;
      }
    }

    answer.push_back(str);
  }
  
  //=================================== OS, FOLDERS MANIPULATION =========================================================
  void get_all_files_in_folder(std::string input_folder, std::string pattern, std::vector<std::string> & answer){
    regex reg_incl( pattern );
    TSystemDirectory dir(input_folder.c_str(), input_folder.c_str()); 
    TList *files = dir.GetListOfFiles();
    if (not files) return;
    
    TSystemFile *file; 
    TString fname; 
    TIter next(files); 
    while ((file=(TSystemFile*)next())) { 
      fname = file->GetName(); 
      string name = fname.Data();
      if( !regex_match(name, reg_incl) ) continue;
      answer.push_back( name );
    }
  }
  
}
  
#endif
