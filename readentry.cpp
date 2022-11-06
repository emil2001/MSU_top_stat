TFile file("SM_theta.root");
TTreeReader reader("chain_1", &file);
TTreeReaderValue<int> weight(reader, "weight"); // template type must match datatype
TTreeReaderValue<double> lumi(reader, "lumi"); // name must match branchname
int i = 0;
while (reader.Next()) {
    i++;
    cout << *lumi << endl;
    if (i == 30000)
        break;
}