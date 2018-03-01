#ifndef CHAI_LIBRARY_H
#define CHAI_LIBRARY_H
#include <string>
#include <vector>
#include <cppast/libclang_parser.hpp>
#include "chai_object_processor.h"
class ChaiLibrary {
private:
  std::string name;
  std::string location;
  std::string output;
  std::vector<std::string> includes;
  std::string _namespace;
  std::vector<std::string> exclusions;
  bool verbose_processing;

  cppast::libclang_compile_config config;
  cppast::compile_flags flags;

  ChaiObjectProcessor processor;
public:
  ChaiLibrary() = delete;
  ChaiLibrary(const std::string& name, const std::string& location, const std::string& output,
    const std::vector<std::string>& includes, const std::vector<std::string>& exclusions, const std::string ns, const bool parse_verbosely = false,
    const bool process_verbosely = false);

  void process();
};

#endif
