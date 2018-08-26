#ifndef UTIL_H
#define UTIL_H
#include <vector>
#include <string>
#include <functional>
namespace util {
  int num_cores();
  void list_files(std::string dirname, std::vector<std::string>& filenames, std::function<bool(std::string)> predicate);
  extern char slash;
  std::string string_to_lower(std::string);
}
#endif
