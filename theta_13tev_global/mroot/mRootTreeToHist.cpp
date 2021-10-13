

#ifndef mRootTreeToHist_hh
#define mRootTreeToHist_hh 1

namespace mRoot{

  class EventsExcluder {
    public:
    EventsExcluder(string fname){
      original_fname = fname;
      exclude_events = parce_train_events( fname );
      reverse = false;
    }

    void Print(){
      cout << "EventsExcluder( " << original_fname << " )"<< endl;
      for(auto iter : *exclude_events){
        cout << " " << iter.first << " " << iter.second->size() << endl;
      }
    }

    void SetExcludedEventsFile(string fname){
      for(auto iter : *exclude_events)
        if(iter.first == fname){ 
          active_events = iter.second;
          active_event_number = active_events->at(0);
          active_event_counter = 0;
          return;
        }
      active_events = nullptr;
    }

    bool IsExcluded(int event_number){
      if(active_events == nullptr) return false;
      if(event_number != active_event_number) return false;
        
      active_event_counter++;
      if(active_event_counter < active_events->size())
        active_event_number  = active_events->at( active_event_counter );
      else active_events = nullptr;
      return true;
    }

    void PrintReport(){
      if(active_events == nullptr){
        cout << "EventsExcluder ok" << endl;
        return;
      }
      cout << "EventsExcluder not ok - only" << active_event_counter << "/" << active_events->size()  << " excluded" << endl;
    }

    bool IsExcludedMod(int event_number){
      if(reverse) return not IsExcluded(event_number);
      return IsExcluded(event_number);
    }

    vector<pair<string, vector<int>*>> *parce_train_events(string fname = "bnn_sm7_trainEvents.txt"){
      vector<pair<string, vector<int>*>> * answer = new vector<pair<string, vector<int>*>>;
      vector<int> * current_events;
      string current_name = "";

      regex name_pattern( ".+\.root" );

      regex sep(" ", regex_constants::egrep);
      smatch sm;

      cout << "parce_train_events() ... " << endl;
      int i = 0;
      std::ifstream infile(fname.c_str());
      for( std::string line; std::getline( infile, line ); ){
        if( regex_match(line, name_pattern) ){ // find new name line
          if(current_name != "") answer->push_back( make_pair(current_name, current_events) );
          current_events = new vector<int>();
          current_name = line;
          cout << "add " << current_name << endl;
          continue;
        }

        auto str = split_string(line, " ");
        if(str.size() > 8){
          for(auto s : str) current_events->push_back( atoi(s.c_str()) );
        }
      }
      answer->push_back( make_pair(current_name, current_events) );

      cout << "parce_train_events() ... done" << endl;
      return answer;
    }

    string original_fname;
    vector<int>* active_events;
    int active_event_number;
    int active_event_counter;
    vector<pair<string, vector<int>*>> * exclude_events;

    bool reverse;
  };

  //Search for ranges of values for hists plotter
  auto hist_range (string prefix,             // path to input files
          vector<string> input_file_names,  // vector of names of input files
          string tree_name,                // name of tree
          string value_rule,               // formula of value to evaluate
          EventsExcluder * event_excluder = nullptr  // contain list of events per file
                   ) {
      cout << "Finding hists range..." << endl;
      int event_index = 0;
      double rmin = 1., rmax = 0.;
      double value;
      pair <double, double> p;
      for (auto name: input_file_names) {
          TFile *file = TFile::Open((prefix + name).c_str());
          if (!file or file->IsZombie()) {
              cerr << __FUNCTION__ << ": can't open file name \"" << name << "\", skip ... " << endl;
              continue;
          }

          TTreeReader *reader = new TTreeReader(tree_name.c_str(), file);
          if (!reader->GetTree()) {
              cerr << __FUNCTION__ << ": can't get ttree \"" << tree_name << "\" in file \"" << file->GetName()
                   << "\", skip ... " << endl;
              file->Close();
              continue;
          }

          TTreeFormula value_f(value_rule.c_str(), value_rule.c_str(), reader->GetTree());

          if (value_f.GetNdim() == 0) {
              reader->GetTree()->Print();
              return;
          }

          if (event_excluder != nullptr) event_excluder->SetExcludedEventsFile(name);
          event_index = -1;

          while (reader->Next()) {
              event_index++;
              value = value_f.EvalInstance();
              if (value > rmax) {
                  rmax = value;
              }
              if (value < rmin) {
                  rmin = value;
              }
              if (event_excluder != nullptr and event_excluder->IsExcludedMod(event_index)) {
                  continue;
              }
          }

      }
      cout << "rmin " << rmin << "rmax " << rmax << endl;
      p = make_pair(rmin, rmax);
      return p;
  }


  // multiple files with same ttree and value_rule -> single hist -> save in file
  // fill_hist(hist_name, nbins, rmax, rmin, output_file, prefix, input_file_names, tree_name, value_rule, weight_rule, event_excluder){
  void fill_hist(string hist_name, // input TH1D name
    int nbins,                 // number of bins
    double rmin,               // x-axis range min
    double rmax,               // x-axis range max
    TFile * output_file,       // output file
    string prefix,             // path to input files
    vector<string> input_file_names, // vector of names of input files
    string tree_name,                // name of tree
    string value_rule,               // formula of value to evaluate
    string weight_rule,              // formula of weight to evaluate
    EventsExcluder * event_excluder = nullptr  // contain list of events per file
  ){
    cout << " fill_hist(): process " << hist_name << " with value rule = " << "\"" << value_rule << "\"" << ", weight rule = " << "\"" << weight_rule << "\"" << endl;
    // open file
    output_file->cd();
    TH1D * hist = new TH1D(hist_name.c_str(), hist_name.c_str(), nbins, rmin, rmax);
    hist->Sumw2();
    int prev_integral = 0;

    double totl_weight = 0;
    double excl_weight = 0;
    double weight = 0;
    int event_index = 0;
    int print_n_entries = 0;

    for(auto name : input_file_names){
      TFile * file = TFile::Open( (prefix + name).c_str() );
      if(!file or file->IsZombie()){
        cerr << __FUNCTION__ << ": can't open file name \"" << name << "\", skip ... " << endl;
        continue;
      }

      TTreeReader * reader = new TTreeReader(tree_name.c_str(), file);
      if(!reader->GetTree()){
        cerr << __FUNCTION__ << ": can't get ttree \"" << tree_name << "\" in file \"" << file->GetName() << "\", skip ... " << endl;
        file->Close();
        continue;
      }

      // reader->GetTree()->Print();
      cout << "process ... " << name << " " << reader->GetTree()->GetEntries() << endl;

      TTreeFormula value_f(value_rule.c_str(), value_rule.c_str(), reader->GetTree());
      TTreeFormula weight_f(weight_rule.c_str(), weight_rule.c_str(), reader->GetTree());

      if( value_f.GetNdim()==0 or weight_f.GetNdim()==0){
        reader->GetTree()->Print();
        return;
      }

      if(event_excluder != nullptr) event_excluder->SetExcludedEventsFile( name );
      event_index = -1;

      while(reader->Next()){
        event_index++;
        weight = weight_f.EvalInstance();
        totl_weight += weight;
        if(event_excluder != nullptr and event_excluder->IsExcludedMod( event_index )){
          excl_weight += weight;
          continue;
        }
        // if(weight < 0.) cout << weight << endl;
        hist->Fill(value_f.EvalInstance(), weight);
        if(print_n_entries > 0 and weight > 0){ 
          msg(value_f.EvalInstance(), weight);
          print_n_entries--;
        };
      }

      file->Close();
      if((int)hist->Integral() == prev_integral){
        cerr << __FUNCTION__ << ": fill no events from file " << name << " to hist " << hist_name << ", continue ... " << endl;
        continue;
      }
      prev_integral += (int)hist->Integral();
      if(event_excluder) event_excluder->PrintReport();
      file->Close();
    }

    cout << __FUNCTION__ << " " << hist_name << " sum of weights = " << totl_weight << " integral = " << (int)hist->Integral() << endl;

    // now we need to reweight hists because of excluded events
    cout << totl_weight << " " << totl_weight << endl;
    if( totl_weight <= excl_weight or totl_weight <= 0 ){
      cerr << __FUNCTION__ << ": something wrong with events weights, can't rescale " << hist_name << " - " << totl_weight << ", " << excl_weight << ", continue ... " << endl;
    } else {
      cout << __FUNCTION__ << ": reweight factor for " << hist_name << " = " << totl_weight / (totl_weight - excl_weight) << endl;
      hist->Scale( totl_weight / (totl_weight - excl_weight) );
    }
    
    // save, exit
    output_file->cd();
    hist->Write();
  }

  void fill_hist_sys(string hist_name, // input TH1D name
    int nbins,                 // number of bins
    double rmin,               // x-axis range min
    double rmax,               // x-axis range max
    TFile * output_file,       // output file
    string prefix,             // path to input files
    vector<string> input_file_names, // vector of names of input files
    string tree_name,                // name of tree
    string value_rule,               // formula of value to evaluate
    string weight_rule_up,                // formula of weight to evaluate
    string weight_rule_down,              // formula of weight to evaluate
    EventsExcluder * event_excluder = nullptr  // contain list of events per file
  ){
    // cout << weight_rule_up << " " << weight_rule_down << endl;
    fill_hist(hist_name+"Up",   nbins, rmin, rmax, output_file, prefix, input_file_names, tree_name, value_rule, weight_rule_up, event_excluder);
    fill_hist(hist_name+"Down", nbins, rmin, rmax, output_file, prefix, input_file_names, tree_name, value_rule, weight_rule_down, event_excluder);
  }













  // multiple files with same ttree and value_rule -> single hist -> save in file
  // fill_hist(hist_name, nbins, rmax, rmin, output_file, prefix, input_file_names, tree_name, value_rule, weight_rule, event_excluder){
  void fill_hist(string hist_name, // input TH1D name
    int nbins,                 // number of bins
    double rmin,               // x-axis range min
    double rmax,               // x-axis range max
    vector<TH1D*> * out_hists,       // output file
    string prefix,             // path to input files
    vector<string> input_file_names, // vector of names of input files
    string tree_name,                // name of tree
    string value_rule,               // formula of value to evaluate
    string weight_rule,              // formula of weight to evaluate
    EventsExcluder * event_excluder = nullptr  // contain list of events per file
  ){
    cout << " fill_hist(): process " << hist_name << " with value rule = " << "\"" << value_rule << "\"" << ", weight rule = " << "\"" << weight_rule << "\"" << endl;
    TH1D * hist = new TH1D(hist_name.c_str(), hist_name.c_str(), nbins, rmin, rmax);
    hist->Sumw2();
    int prev_integral = 0;

    double totl_weight = 0;
    double excl_weight = 0;
    double weight = 0;
    int event_index = 0;
    int print_n_entries = 0;

    for(auto name : input_file_names){
      TFile * file = TFile::Open( (prefix + name).c_str() );
      if(!file or file->IsZombie()){
        cerr << __FUNCTION__ << ": can't open file name \"" << name << "\", skip ... " << endl;
        continue;
      }

      TTreeReader * reader = new TTreeReader(tree_name.c_str(), file);
      if(!reader->GetTree()){
        cerr << __FUNCTION__ << ": can't get ttree \"" << tree_name << "\" in file \"" << file->GetName() << "\", skip ... " << endl;
        file->Close();
        continue;
      }

      // reader->GetTree()->Print();

      TTreeFormula value_f(value_rule.c_str(), value_rule.c_str(), reader->GetTree());
      TTreeFormula weight_f(weight_rule.c_str(), weight_rule.c_str(), reader->GetTree());

      if( value_f.GetNdim()==0 or weight_f.GetNdim()==0){
        reader->GetTree()->Print();
        file->Close();
        return;
      }

      if(event_excluder != nullptr) event_excluder->SetExcludedEventsFile( name );
      event_index = -1;
      int number_of_skiped_events = 0;
      int number_of_written_events = 0;

      while(reader->Next()){
        event_index++;
        weight = weight_f.EvalInstance();

        if(print_n_entries > 0 ){ 
          msg(value_f.EvalInstance(), weight);
          print_n_entries--;
        };
        if( TMath::IsNaN(weight) ){
          msg("Event #", event_index, "is nan");
          continue;
        }

        totl_weight += weight;
        if(event_excluder != nullptr and event_excluder->IsExcludedMod( event_index )){
          excl_weight += weight;
          number_of_skiped_events++;
          continue;
        }
        number_of_written_events++;
        // if(weight < 0.) cout << weight << endl;
        hist->Fill(value_f.EvalInstance(), weight);
      }
      file->Close();

      if((int)hist->Integral() == prev_integral){
        cerr << __FUNCTION__ << ": fill no events from file " << name << " to hist " << hist_name << ", continue ... " << endl;
        continue;
      }
      prev_integral += (int)hist->Integral();

      cout << "exclude " << number_of_skiped_events << " events, save " << number_of_written_events << " events" << endl;
      if(event_excluder) event_excluder->PrintReport();
    }

    // now we need to reweight hists because of excluded events
    cout << totl_weight << " " << totl_weight << endl;
    if( totl_weight <= excl_weight or totl_weight <= 0 ){
      cerr << __FUNCTION__ << ": something wrong with events weights, can't rescale " << hist_name << " - " << totl_weight << ", " << excl_weight << ", continue ... " << endl;
    } else {
      cout << __FUNCTION__ << ": reweight factor for " << hist_name << " = " << totl_weight / (totl_weight - excl_weight) << endl;
      hist->Scale( totl_weight / (totl_weight - excl_weight) );
    }
    
    // save, exit
    out_hists->push_back( hist );
  }

  void fill_hist_sys(string hist_name, // input TH1D name
    int nbins,                 // number of bins
    double rmin,               // x-axis range min
    double rmax,               // x-axis range max
    vector<TH1D*>*out_hists,       // output file
    string prefix,             // path to input files
    vector<string> input_file_names, // vector of names of input files
    string tree_name,                // name of tree
    string value_rule,               // formula of value to evaluate
    string weight_rule_up,                // formula of weight to evaluate
    string weight_rule_down,              // formula of weight to evaluate
    EventsExcluder * event_excluder = nullptr  // contain list of events per file
  ){
    // cout << weight_rule_up << " " << weight_rule_down << endl;
    fill_hist(hist_name+"Up",   nbins, rmin, rmax, out_hists, prefix, input_file_names, tree_name, value_rule, weight_rule_up, event_excluder);
    fill_hist(hist_name+"Down", nbins, rmin, rmax, out_hists, prefix, input_file_names, tree_name, value_rule, weight_rule_down, event_excluder);
  }

  void fill_hist_2d(string hist_name, // input TH1D name
    int nbins,                 // number of bins
    double rmin,               // x-axis range min
    double rmax,               // x-axis range max
    vector<TH2D*> * out_hists,       // output file
    string prefix,             // path to input files
    vector<string> input_file_names, // vector of names of input files
    string tree_name,                // name of tree
    string value_rule_x,               // formula of value to evaluate
    string value_rule_y,               // formula of value to evaluate
    string weight_rule,              // formula of weight to evaluate
    EventsExcluder * event_excluder = nullptr  // contain list of events per file
  ){
    cout << " fill_hist(): process " << hist_name << " with value rule = " << "\"" << value_rule_x << ", " << value_rule_y << "\"" << ", weight rule = " << "\"" << weight_rule << "\"" << endl;
    TH2D * hist = new TH2D(hist_name.c_str(), hist_name.c_str(), nbins, rmin, rmax, nbins, rmin, rmax);
    hist->Sumw2();
    int prev_integral = 0;

    double totl_weight = 0;
    double excl_weight = 0;
    double weight = 0;
    int event_index = 0;
    int print_n_entries = 0;

    for(auto name : input_file_names){
      TFile * file = TFile::Open( (prefix + name).c_str() );
      if(!file or file->IsZombie()){
        cerr << __FUNCTION__ << ": can't open file name \"" << name << "\", skip ... " << endl;
        continue;
      }

      TTreeReader * reader = new TTreeReader(tree_name.c_str(), file);
      if(!reader->GetTree()){
        cerr << __FUNCTION__ << ": can't get ttree \"" << tree_name << "\" in file \"" << file->GetName() << "\", skip ... " << endl;
        file->Close();
        continue;
      }

      // reader->GetTree()->Print();

      TTreeFormula value_x_f(value_rule_x.c_str(), value_rule_x.c_str(), reader->GetTree());
      TTreeFormula value_y_f(value_rule_y.c_str(), value_rule_y.c_str(), reader->GetTree());
      TTreeFormula weight_f(weight_rule.c_str(), weight_rule.c_str(), reader->GetTree());

      if( value_x_f.GetNdim()==0 or value_y_f.GetNdim()==0 or weight_f.GetNdim()==0){
        reader->GetTree()->Print();
        file->Close();
        return;
      }

      if(event_excluder != nullptr) event_excluder->SetExcludedEventsFile( name );
      event_index = -1;
      int number_of_skiped_events = 0;
      int number_of_written_events = 0;

      while(reader->Next()){
        event_index++;
        weight = weight_f.EvalInstance();

        if(print_n_entries > 0 ){ 
          msg(value_x_f.EvalInstance(), value_y_f.EvalInstance(), weight);
          print_n_entries--;
        };
        if( TMath::IsNaN(weight) ){
          msg("Event #", event_index, "is nan");
          continue;
        }

        totl_weight += weight;
        if(event_excluder != nullptr and event_excluder->IsExcludedMod( event_index )){
          excl_weight += weight;
          number_of_skiped_events++;
          continue;
        }
        number_of_written_events++;
        // if(weight < 0.) cout << weight << endl;
        hist->Fill(value_x_f.EvalInstance(), value_y_f.EvalInstance(), weight);
      }

      file->Close();
      if((int)hist->Integral() == prev_integral){
        cerr << __FUNCTION__ << ": fill no events from file " << name << " to hist " << hist_name << ", continue ... " << endl;
        continue;
      }
      prev_integral += (int)hist->Integral();

      cout << "exclude " << number_of_skiped_events << " events, save " << number_of_written_events << " events" << endl;
      if(event_excluder) event_excluder->PrintReport();
    }

    // now we need to reweight hists because of excluded events
    cout << totl_weight << " " << totl_weight << endl;
    if( totl_weight <= excl_weight or totl_weight <= 0 ){
      cerr << __FUNCTION__ << ": something wrong with events weights, can't rescale " << hist_name << " - " << totl_weight << ", " << excl_weight << ", continue ... " << endl;
    } else {
      cout << __FUNCTION__ << ": reweight factor for " << hist_name << " = " << totl_weight / (totl_weight - excl_weight) << endl;
      hist->Scale( totl_weight / (totl_weight - excl_weight) );
    }
    
    // save, exit
    out_hists->push_back( hist );
  }

  void fill_hist_2d_sys(string hist_name, // input TH1D name
    int nbins,                 // number of bins
    double rmin,               // x-axis range min
    double rmax,               // x-axis range max
    vector<TH2D*>*out_hists,       // output file
    string prefix,             // path to input files
    vector<string> input_file_names, // vector of names of input files
    string tree_name,                // name of tree
    string value_rule_x,               // formula of value to evaluate
    string value_rule_y,               // formula of value to evaluate
    string weight_rule_up,                // formula of weight to evaluate
    string weight_rule_down,              // formula of weight to evaluate
    EventsExcluder * event_excluder = nullptr  // contain list of events per file
  ){
    // cout << weight_rule_up << " " << weight_rule_down << endl;
    fill_hist_2d(hist_name+"Up",   nbins, rmin, rmax, out_hists, prefix, input_file_names, tree_name, value_rule_x, value_rule_y, weight_rule_up, event_excluder);
    fill_hist_2d(hist_name+"Down", nbins, rmin, rmax, out_hists, prefix, input_file_names, tree_name, value_rule_x, value_rule_y, weight_rule_down, event_excluder);
  }




















  // ===============
  TH1D* fill_hist(string hist_name, // input TH1D name
    int nbins,                 // number of bins
    double rmin,               // x-axis range min
    double rmax,               // x-axis range max
    string prefix,             // path to input files
    vector<string> input_file_names, // vector of names of input files
    string tree_name,                // name of tree
    string value_rule,               // formula of value to evaluate
    string weight_rule,              // formula of weight to evaluate
    EventsExcluder * event_excluder = nullptr  // contain list of events per file
  ){
    cout << " fill_hist(): process " << hist_name << " with value rule = " << "\"" << value_rule << "\"" << ", weight rule = " << "\"" << weight_rule << "\"" << endl;
    // open file
    TH1D * hist = new TH1D(hist_name.c_str(), hist_name.c_str(), nbins, rmin, rmax);
    hist->Sumw2();
    int prev_integral = 0;

    double totl_weight = 0;
    double excl_weight = 0;
    double weight = 0;
    int event_index = 0;
    int print_n_entries = 0;

    for(auto name : input_file_names){
      cout << "process " <<  name << endl;
      TFile * file = TFile::Open( (prefix + name).c_str() );
      if(!file or file->IsZombie()){
        cerr << __FUNCTION__ << ": can't open file name \"" << name << "\", skip ... " << endl;
        continue;
      }

      TTreeReader * reader = new TTreeReader(tree_name.c_str(), file);
      if(!reader->GetTree()){
        cerr << __FUNCTION__ << ": can't get ttree \"" << tree_name << "\" in file \"" << file->GetName() << "\", skip ... " << endl;
        file->Close();
        continue;
      }

      // reader->GetTree()->Print();

      TTreeFormula value_f(value_rule.c_str(), value_rule.c_str(), reader->GetTree());
      TTreeFormula weight_f(weight_rule.c_str(), weight_rule.c_str(), reader->GetTree());

      if( value_f.GetNdim()==0 or weight_f.GetNdim()==0){
        reader->GetTree()->Print();
        continue;
      }

      if(event_excluder != nullptr) event_excluder->SetExcludedEventsFile( name );
      event_index = -1;

      while(reader->Next()){
        event_index++;
        weight = weight_f.EvalInstance();
        totl_weight += weight;
        if(event_excluder != nullptr and event_excluder->IsExcludedMod( event_index )){
          excl_weight += weight;
          continue;
        }
        // if(weight < 0.) cout << weight << endl;
        hist->Fill(value_f.EvalInstance(), weight);
        if(print_n_entries > 0 and weight > 0){ 
          msg(value_f.EvalInstance(), weight);
          print_n_entries--;
        };
      }

      file->Close();
      if((int)hist->Integral() == prev_integral){
        cerr << __FUNCTION__ << ": fill no events from file " << name << " to hist " << hist_name << ", continue ... " << endl;
        continue;
      }
      prev_integral += (int)hist->Integral();
      file->Close();
    }

    // now we need to reweight hists because of excluded events
    if( totl_weight <= excl_weight){
      cerr << __FUNCTION__ << ": something wrong with events weights, can't rescale " << hist_name << " - " << totl_weight << ", " << excl_weight << ", continue ... " << endl;
    } else {
      cout << __FUNCTION__ << ": reweight factor for " << hist_name << " = " << totl_weight / (totl_weight - excl_weight) << endl;
      hist->Scale( totl_weight / (totl_weight - excl_weight) );
    }
    
    // save, exit
    return hist;
  }
};

#endif










