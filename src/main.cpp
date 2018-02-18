#include <vector>
#include <string>
#include <regex>
#include <memory>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <stack>
#include <list>
#include <fstream>
#include <tinydir.h>
#include <cppast/libclang_parser.hpp>
#include "chai_object_processor.h"


std::vector<std::string> grabFiles(const std::string& directory, const std::vector<std::string>& exclusions, const bool recursive = true) {
  std::vector<std::string> files;

  tinydir_dir dir;
  tinydir_open(&dir, directory.c_str());

  while(dir.has_next) {
    tinydir_file file;
    tinydir_readfile(&dir, &file);

    std::regex h_match("[^[:space:]]+[.]h(?:pp)?");
    if(std::none_of(exclusions.cbegin(), exclusions.cend(), [&](const std::string& exc){return exc == std::string(file.name);})) {
      if(recursive && file.is_dir && std::string(file.name) != "." && std::string(file.name) != "..") {
        auto dir_files = grabFiles(directory + file.name + "/", exclusions, recursive);
        files.insert(files.end(), dir_files.begin(), dir_files.end());
      }
      else if(!file.is_dir && std::regex_match(std::string(file.name), h_match) ) {
        files.push_back(std::string(directory + file.name));
      }
    }

    tinydir_next(&dir);
  }
  return files;
}

int main(int argc, char** argv) {
  std::string include_dir;
  if(argc < 2) {
    std::cout<<"=                    USAGE:                   ="<<std::endl;
    std::cout<<"nymph-registrar <args> [directory to register]"<<std::endl;
    std::cout<<"----------------Argument list------------------"<<std::endl;
    std::cout<<"= -I/path/to/includes: This is so you can let ="<<std::endl;
    std::cout<<"= clang know includes needed to parse the li- ="<<std::endl;
    std::cout<<"= brary that is being registered.             ="<<std::endl;
    std::cout<<"=                                             ="<<std::endl;
    std::cout<<"= -E/path/to/exclude: This lets you specify   ="<<std::endl;
    std::cout<<"= files and directories to be excluded from   ="<<std::endl;
    std::cout<<"= processing.                                 ="<<std::endl;
    std::cout<<"=                                             ="<<std::endl;
    std::cout<<"= -O/path/to/output/dir: This is where the    ="<<std::endl;
    std::cout<<"= tool will output the generated reigstration ="<<std::endl;
    std::cout<<"= header(s).                                  ="<<std::endl;
    std::cout<<"=                                             ="<<std::endl;
    std::cout<<"= -V: This tells the tool to output verbose   ="<<std::endl;
    std::cout<<"= output as it processes.                     ="<<std::endl;
    std::cout<<"=                                             ="<<std::endl;
    std::cout<<"= -VV: This tells the tool to output verbose  ="<<std::endl;
    std::cout<<"= output as it parses and processes.          ="<<std::endl;
    std::cout<<"=                                             ="<<std::endl;
    std::cout<<"= -X: This tells the tool to expand the outp- ="<<std::endl;
    std::cout<<"= ut into multiple files instead of one.      ="<<std::endl;
    std::cout<<"=                                             ="<<std::endl;
    std::cout<<"= -S: This tells the tool to output the resu- ="<<std::endl;
    std::cout<<"= lt to stdout.                               ="<<std::endl;
    return 0;
  }

  std::string directory_to_be_registered = std::string(argv[argc - 1]);
  std::vector<std::string> args;
  std::vector<std::string> includes;
  std::vector<std::string> exclusions;
  std::string output_directory = "./";
  bool expand_files = false;
  bool print_to_stdout = false;
  bool parse_verbosely = false;
  bool process_verbosely = false;

  for(int i=1; i<argc-1; i++) {
    args.push_back(argv[i]);
  }

  for(auto arg : args) {
    if(arg.find("-I") != std::string::npos) {
      arg.erase(0, 2);
      std::cout<<"Include directory: "<<arg<<std::endl;
      includes.push_back(arg);
    }
    else if(arg.find("-E") != std::string::npos) {
      arg.erase(0, 2);
      std::cout<<"Added to exclusions: "<<arg<<std::endl;
      exclusions.push_back(arg);
    }
    else if(arg.find("-S") != std::string::npos) {
      std::cout<<"Outputting generation to stdout."<<std::endl;
      print_to_stdout = true;
    }
    else if(arg.find("-O") != std::string::npos) {
      arg.erase(0, 2);
      std::cout<<"Output directory: "<<arg<<std::endl;
      output_directory = arg;
    }
    else if(arg.find("-X") != std::string::npos) {
      std::cout<<"Output will be expanded to multiple files."<<std::endl;
      expand_files = true;
    }
    else if(arg.find("-V") != std::string::npos && arg.find("-VV") == std::string::npos) {

      std::cout<<"Processing verbosely."<<std::endl;
      process_verbosely = true;
    }
    else if(arg.find("-VV") != std::string::npos) {
      std::cout<<"Parsing and Processing verbosely"<<std::endl;
      parse_verbosely = true;
      process_verbosely = true;
    }
  }

  cppast::libclang_compile_config config;
  cppast::compile_flags flags;

  config.fast_preprocessing(false);

  config.set_flags(cppast::cpp_standard::cpp_14, flags);

  for(auto i : includes) {
    config.add_include_dir(i);
  }

  exclusions.push_back("generated");
  std::cout<<"Added to exclusions: "<<exclusions.back()<<std::endl;

  auto files = grabFiles(directory_to_be_registered, exclusions);

  auto object_processor = ChaiObjectProcessor(config, parse_verbosely);

  object_processor.processObjects(files, process_verbosely);

  auto registrations = object_processor.generateRegistrations(process_verbosely);

  if(print_to_stdout) {
    std::cout<<registrations.str();
  }

  std::ofstream output_file(output_directory + "generated_registrations.h");
  output_file << registrations.str();
  output_file.flush();
  output_file.close();

  return 0;
}
