#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <stack>
#include <list>
#include <sys/stat.h>
#include <yaml-cpp/yaml.h>
#include "chai_library.h"

int main(int argc, char** argv) {
  std::string include_dir;
  bool use_command_line_args = true;
  std::string config_location = "";
  bool parse_verbosely = false;
  bool process_verbosely = false;
  std::string output_directory = "./";
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
    std::cout<<"= -N: This tells the tool to create registra- ="<<std::endl;
    std::cout<<"= tions within this namespace/folder.         ="<<std::endl;
    return 0;
  }

  if(argc == 2) {
    auto arg = std::string(argv[1]);
    if(arg.find(".yml")) {
      use_command_line_args = false;
    }
    config_location = arg;
  }

  if(use_command_line_args) {
    std::string directory_to_be_registered = std::string(argv[argc - 1]);
    std::vector<std::string> args;
    std::vector<std::string> includes;
    std::vector<std::string> exclusions;
    std::string output_namespace = "";

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
      else if(arg.find("-O") != std::string::npos) {
        arg.erase(0, 2);
        std::cout<<"Output directory: "<<arg<<std::endl;
        output_directory = arg;
      }
      else if(arg.find("-N") != std::string::npos) {
        arg.erase(0, 2);
        std::cout<<"Registrations will be put in namespace "<<arg<<std::endl;
        output_namespace = arg;
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

    auto library = ChaiLibrary("", directory_to_be_registered, output_directory, includes, exclusions, output_namespace, parse_verbosely, process_verbosely);
    library.process();
  }
  else {
    std::vector<ChaiLibrary> libraries;
    auto config = YAML::LoadFile(config_location);
    parse_verbosely = config["verbose_parsing"].as<bool>();
    process_verbosely = config["verbose_processing"].as<bool>();
    output_directory = config["output_directory"].as<std::string>();

    std::cout<<"Parse: "<<parse_verbosely<<" Process: "<<process_verbosely<<" Output: "<<output_directory<<std::endl;

    for(auto lib : config["libraries"]) {
      auto name = lib["name"].as<std::string>();
      auto location = lib["location"].as<std::string>();
      std::vector<std::string> includes;
      std::vector<std::string> excludes;
      if(lib["includes"]){
        if(lib["includes"].IsSequence()){
          includes = lib["includes"].as<std::vector<std::string>>();
        }
        else {
          includes.push_back(lib["includes"].as<std::string>());
        }
      }
      if(lib["excludes"]) {
        if(lib["excludes"].IsSequence()) {
          excludes = lib["excludes"].as<std::vector<std::string>>();
        }
        else {
          excludes.push_back(lib["excludes"].as<std::string>());
        }
      }

      auto ns = lib["namespace"].as<std::string>();

      std::cout<<"Library: "<<name<<" Location: "<<location<<" Namespace: "<<ns<<std::endl;
      std::cout<<"Includes: ";
      for(auto i : includes) {
        std::cout<<i<<", ";
      }
      std::cout<<std::endl;
      for(auto e : excludes) {
        std::cout<<e<<", ";
      }
      std::cout<<std::endl;
      libraries.emplace_back(name, location, output_directory, includes, excludes, ns, parse_verbosely, process_verbosely);
    }

    for(auto& l : libraries) {
      l.process();
    }
  }

  return 0;
}
