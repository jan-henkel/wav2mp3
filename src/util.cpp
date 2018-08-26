#ifdef _WIN32
#include <Windows.h>
#elif __linux__
#include <unistd.h>
#include <dirent.h>
#endif

#include <functional>
#include <vector>
#include "util.h"

namespace util {
  using std::vector;
  using std::string;
  using std::function;

#if _WIN32
  char slash='\\';
#else
  char slash='/';
#endif
  

#ifdef _WIN32
  int num_cores() {
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
  }
#endif

#ifdef __linux__
  int num_cores() {
    return sysconf(_SC_NPROCESSORS_ONLN);
  }
#endif

#ifdef _WIN32
  void list_files(string dirname, vector<string>& filenames, function<bool(string)> predicate) {
    std::string pat(dirname);
    pat.append("\\*");
    WIN32_FIND_DATA fdata;
    HANDLE hfind;
    if ((hfind = FindFirstFile(pat.c_str(), &fdata)) != INVALID_HANDLE_VALUE) {
      do {
	string filename(fdata.cFileName);
	if(predicate(filename))
	  filenames.push_back(filename);
      } while (FindNextFile(hfind, &fdata) != 0);
      FindClose(hfind);
    }
  }
#endif

#ifdef __linux__
  void list_files(string dirname, vector<string>& filenames, function<bool(string)> predicate) {
    DIR* dirp = opendir(dirname.c_str());
    struct dirent * dp;
    while ((dp = readdir(dirp)) != NULL) {
      string filename(dp->d_name);
      if(dp->d_type != DT_DIR) {
	if(predicate(filename))
	  filenames.push_back(filename);
      }
    }
    closedir(dirp);
  }
#endif
  
}
