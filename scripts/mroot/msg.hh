
#ifndef MSG_HH 
#define MSG_HH 1

namespace mRoot {

  // ======= msg ====================================================================
  void msg(){ std::cout << "\n" << std::endl; };
  template<typename T> void msg(T t){
    std::cout << t << std::endl;
  }

  template<typename T, typename... Args> void msg(T t, Args... args){
    std::cout << t << " ";
    msg(args...);
  }

  // ======= msg_sep ====================================================================
  template<typename T> void msg_sep(const string & separator, const string & last_symbol, T t){ std::cout << t << last_symbol; };
  template<typename T, typename... Args> void msg_sep(const string & separator, const string & last_symbol, T t, Args... args){
    std::cout << t << separator;
    msg_sep(separator, last_symbol, args...);
  }

  // ======= msg_nll ====================================================================
  template<typename T> void msg_nnl(T t){ std::cout << t << " "; };
  template<typename T, typename... Args> void msg_nnl(T t, Args... args){
    std::cout << t << " ";
    msg_nnl(args...);
  }

  // ======= msg_err ====================================================================
  void msg_err(){ std::cerr << "\n" << std::endl; };
  template<typename T> void msg_err(T t){
    std::cerr << t << std::endl;
  }

  template<typename T, typename... Args> void msg_err(T t, Args... args){
    std::cerr << t << " ";
    msg_err(args...);
  }

  // ======= error macros  ==================================================================== TODO hide them for release

  enum verbose{
    SILENCE=0,
    ERROR,
    WARNING,
    INFO,
    DEBUG,
    VERBOSE
  };
  
  #define MSG_ERROR(...)   if(verbose_lvl >= ge::verbose::ERROR)   msg_err("ERROR:", __VA_ARGS__)
  #define MSG_WARNING(...) if(verbose_lvl >= ge::verbose::WARNING) msg_err("WARNING:", __VA_ARGS__)
  #define MSG_INFO(...)    if(verbose_lvl >= ge::verbose::INFO)    msg(__VA_ARGS__)
  #define MSG_DEBUG(...)   if(verbose_lvl >= ge::verbose::DEBUG)   msg("debug info:", __VA_ARGS__)
  #define MSG_VERBOSE(...) if(verbose_lvl >= ge::verbose::VERBOSE) msg("verbose info:", __VA_ARGS__)

  // ======= msg progress  ==================================================================== 
  void msg_progress(double progress, int barWidth = 50){
    if(progress > 1.) progress = 1.;
    if(progress < 0.) progress = 0.;

    std::cout << "[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
      if (i < pos) std::cout << "=";
      else if (i == pos) std::cout << ">";
      else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    std::cout.flush();
  }
};

#endif















