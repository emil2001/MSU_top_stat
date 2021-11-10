
#ifndef PMLIB_STAT_ANALYSES_HH
#define PMLIB_STAT_ANALYSES_HH 1

namespace pm {
  
  double get_th1_limit_upper(TH1 * hist, double frac){
    int bin, nbins = hist->GetNbinsX();
    double subsum = 0;
    double sum = hist->Integral();
    for(bin = nbins; bin > 0; bin--){
      subsum += hist->GetBinContent( bin );
      if(subsum >= sum*frac) break;
    }
    return hist->GetBinLowEdge(bin);
  }
  
  struct Limits{
    double l_1s, c_1s, u_1s, l_2s, c_2s, u_2s;
    Limits(){
      l_1s = 0;
      c_1s = 0;
      u_1s = 0;
      l_2s = 0;
      c_2s = 0; 
      u_2s = 0;
    }

    Limits& operator += (Limits const & l) {
      l_1s += l.l_1s;
      c_1s += l.c_1s;
      u_1s += l.u_1s;
      l_2s += l.l_2s;
      c_2s += l.c_2s;
      u_2s += l.u_2s;
      return *this;
    }
    void Print() const { cout  << "Limits(" << l_2s << " " << l_1s << " " << c_1s << " " << u_1s << " " << u_2s << ")" << endl; }
  };
  
  Limits get_th1_limits(TH1 * hist){
    Limits answer;
    double CL_2sigma = 1. - 0.0455;
    answer.l_2s = get_th1_limit_upper(hist, CL_2sigma + (1.-CL_2sigma)*0.5);
    answer.c_2s = get_th1_limit_upper(hist, 0.5);
    answer.u_2s = get_th1_limit_upper(hist, (1.-CL_2sigma)*0.5);
    double CL_1sigma = 1. - 0.3173;
    answer.l_1s = get_th1_limit_upper(hist, CL_1sigma + (1.-CL_1sigma)*0.5);
    answer.c_1s = get_th1_limit_upper(hist, 0.5);
    answer.u_1s = get_th1_limit_upper(hist, (1.-CL_1sigma)*0.5);
    return answer;
  }
  
}

#endif
