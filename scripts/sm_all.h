//
// Created by emlg on 11.07.2021.
//

#ifndef TEST1_SM_ALL_H
#define TEST1_SM_ALL_H

#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#include "TH1D.h"
#include "TFile.h"
#include "TTree.h"
#include "TMath.h"
#include "THStack.h"
#include "TKey.h"

using namespace std;

int histsPlot(string mode, TString inputFileName, float scaleFactor=0.);

#endif //TEST1_SM_ALL_H
