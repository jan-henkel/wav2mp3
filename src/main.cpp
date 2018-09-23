#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include "util.h"
#include "pthread_raii.h"
#include "convert.h"

using std::cout;
using std::endl;
using std::cerr;
using std::string;
using std::vector;
const string wav_ext=".wav";
const string mp3_ext=".mp3";

using namespace pthread_raii;

vector<string> filenames;
string dirname;
pmutex m_stack;
pmutex m_io;

void do_work() {
  while(true) {
    string filename;
    
    {
      //access to the filenames stack is serialized with a mutex
      plock_guard g(m_stack);
      if(filenames.empty())
        return;
      filename=filenames.back();
      filenames.pop_back();
    }

    try {
      convert::convert(dirname+filename, dirname+filename.substr(0,filename.size()-wav_ext.size())+mp3_ext);
    }
    catch(std::runtime_error& e) {
      plock_guard g(m_io);
      std::cerr<<"File "<<filename<<": "<<e.what()<<endl;
    }
  }
}


int main(int argc, char** argv) {
  
  if(argc<2)
    dirname=".";
  else
    dirname=string(argv[1]);
  if(dirname[dirname.size()-1]!=util::slash)
    dirname+=util::slash;
  
  int n_cores=util::num_cores();

  auto ends_with_wav=[](string s) { return wav_ext.size()<=s.size() && util::string_to_lower(s.substr(s.size()-wav_ext.size()))==wav_ext;};
  util::list_files(dirname, filenames, ends_with_wav);

  vector<pthread> threads;
  threads.reserve(n_cores);

  while(n_cores--)
    threads.emplace_back(do_work);
}
