
#ifndef mRootThread_hh
#define mRootThread_hh 1

#include "TThread.h"

namespace mRoot {

  class ThreadJob{
    public:
    ThreadJob(TThread* th){
      thread = th;
      is_finished = false; 
      is_started = false;
      is_failed = false;
      is_running = false;
      number_of_tries = 0;
      return_code = -1;
    }

    void Run(){
      is_started = true;
      is_running = true;
      is_failed = false;
      number_of_tries++;
      thread->Run();
    }

    void Check(){
      auto state = thread->GetState();
      // if( is_started ) cout << state << " " << TThread::kRunningState << " " << TThread::kTerminatedState << endl;
      if(state == TThread::kRunningState){
        is_finished = false; 
        is_started = true;
        is_failed = false;
        is_running = true;
      }
      if(state == TThread::kTerminatedState or state == TThread::kFinishedState or state == TThread::kCanceledState){
        is_finished = true; 
        is_started = true;
        is_failed = false;
        is_running = false;
      }
    }

    TThread* thread;
    int return_code, number_of_tries;
    bool is_finished, is_started, is_failed, is_running;
  };

  void threads_care(vector<TThread*> threads, int max_cores_to_use = 5, int number_of_tries = 5){
    // gSystem->Load("/usr/lib/libpthread.so");
    // gSystem->Load("$ROOTSYS/lib/libThread.so");

    vector<ThreadJob*> jobs;
    for(auto thread : threads) jobs.push_back( new ThreadJob(thread) );
    
    bool run_jobs = true;
    while(run_jobs){
      // TThread::Ps();
      // check number of running jobs
      int started_jobs  = 0, finished_jobs = 0, failed_jobs = 0, running_jobs = 0, waiting_jobs = 0;
      for(auto job : jobs){
        job->Check();
        if(job->is_finished) finished_jobs++;
        if(job->is_started)  started_jobs++;
        if(not job->is_started) waiting_jobs++;
        if(job->is_failed)   failed_jobs++;
        if(job->is_running)  running_jobs++;
      }
      msg("threads_care(): running =", running_jobs, ", finished =", finished_jobs, ", failed =", failed_jobs, ", waiting =", waiting_jobs);
     
      // if all working continue
      int jobs_to_submit = max_cores_to_use - running_jobs;
      if(jobs_to_submit <= 0){
        gSystem->Sleep(10000);
        continue;
      }

      cout << jobs_to_submit << endl;

      // run jobs
      for(auto job : jobs){
        if(not jobs_to_submit) break;
        if(not job->is_started and not job->is_finished){
          if(job->is_failed and number_of_tries <= job->number_of_tries) continue;
          msg("submit a thread ... ");
          job->Run();
          jobs_to_submit--;
        }
      }
      if(jobs_to_submit == max_cores_to_use and not waiting_jobs) run_jobs = false;

      gSystem->Sleep(10000);
    }
  }

};

#endif
