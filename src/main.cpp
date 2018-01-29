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
#include <cppast/parser.hpp>
#include <cppast/libclang_parser.hpp>
#include <cppast/compile_config.hpp>
#include <cppast/visitor.hpp>
#include <cppast/code_generator.hpp>
#include <cppast/cpp_entity_kind.hpp>
#include <cppast/cpp_forward_declarable.hpp>
#include <cppast/cpp_namespace.hpp>
#include <cppast/cpp_member_function.hpp>
#include <cppast/cpp_enum.hpp>
#include "chai_module.h"
#include "chai_enum.h"

std::vector<std::string> grabFiles(const std::string& directory, const bool recursive = true) {
  std::vector<std::string> files;

  tinydir_dir dir;
  tinydir_open(&dir, directory.c_str());

  while(dir.has_next) {
    tinydir_file file;
    tinydir_readfile(&dir, &file);

    std::regex h_match("[^[:space:]]+[.]h(?:pp)?");

    if(recursive && file.is_dir && std::string(file.name) != "." && std::string(file.name) != "..") {
      auto dir_files = grabFiles(directory + file.name + "/");
      files.insert(files.end(), dir_files.begin(), dir_files.end());
    }
    else if(!file.is_dir && std::regex_match(std::string(file.name), h_match) ) {
      files.push_back(std::string(directory + file.name));
    }

    tinydir_next(&dir);
  }
  return files;
}

std::vector<std::unique_ptr<cppast::cpp_file>> parseFiles(std::vector<std::string> filenames, const cppast::libclang_compile_config& config) {
  cppast::cpp_entity_index idx;
  cppast::libclang_parser parser;
  std::vector<std::unique_ptr<cppast::cpp_file>> parsed_files;
  for(auto f : filenames) {
    std::cout<<"Parsing file: "<<f<<std::endl;
    auto file = parser.parse(idx, f, config);
    if(parser.error()) {
      std::cout<<"There was a parser error when parsing "<<f<<std::endl;
      parser.reset_error();
    }
    else {
      parsed_files.push_back(std::move(file));
    }
  }
  return std::move(parsed_files);
}

std::vector<std::shared_ptr<ChaiObject>> processFile(std::unique_ptr<cppast::cpp_file> file) {
  std::string _namespace;
  std::string _class;
  std::vector<std::shared_ptr<ChaiObject>> objects;
  std::stack<std::shared_ptr<ChaiModule>> modules;
  static std::list<std::string> namespaces;

  cppast::visit(*file, cppast::blacklist<cppast::cpp_entity_kind::include_directive_t, cppast::cpp_entity_kind::macro_definition_t>(), [&](const cppast::cpp_entity& ent, cppast::visitor_info info) {
    if(info.event != cppast::visitor_info::container_entity_exit) {
      if(ent.kind() == cppast::cpp_entity_kind::namespace_t) {
        auto& ns = static_cast<const cppast::cpp_namespace&>(ent);
        if(_namespace.empty())
          _namespace = ns.name();
        else
          _namespace = _namespace + "::" + ns.name();

        if(std::count(namespaces.begin(), namespaces.end(), _namespace) == 0) {
          std::cout<<"New namespace -- "<<_namespace<<std::endl;
          namespaces.push_back(_namespace);
        }
      }
      else if(ent.kind() == cppast::cpp_entity_kind::enum_t) {
        auto& en = static_cast<const cppast::cpp_enum&>(ent);
        if(cppast::has_attribute(en.attributes(), "scriptable")) {

          auto chai_enum = ChaiObject::create<ChaiEnum>(en.name(), _namespace);
          for(auto it = en.begin(); it != en.end(); it++) {
            chai_enum->addValue(it->name());
          }
          objects.push_back(chai_enum);
          std::cout<<"Found enum "<<en.name()<<std::endl;
        }
      }
      else if(ent.kind() == cppast::cpp_entity_kind::class_t) {
        auto& cs = static_cast<const cppast::cpp_class&>(ent);
        if(cppast::has_attribute(cs.attributes(), "scriptable")) {
          _class = cs.name();
          modules.emplace(ChaiObject::create<ChaiModule>(_class, _namespace));
          objects.push_back(modules.top());

          std::cout<<"Found scriptable module: "<<_class<<" with base classes: ";

          for(auto& base : cs.bases()) {
            modules.top()->addRawBase(base.name());
            std::cout<<base.name()<<", ";
          }
          std::cout<<std::endl;
        }
      }
      else if(ent.kind() == cppast::cpp_entity_kind::constructor_t) {
        auto& ct = static_cast<const cppast::cpp_constructor&>(ent);
        if(cppast::has_attribute(ct.attributes(), "scriptable")) {

          auto constructor = ChaiObject::create<ChaiFunction>(ct.name(), _namespace);
          constructor->setReturnType("");
          constructor->isMethodOf(modules.top());

          for(auto& param : ct.parameters()) {
            constructor->addArgument(cppast::to_string(param.type()), param.name());
          }

          std::cout<<"Found scriptable constructor: "<<constructor->getName()<<std::endl;
          modules.top()->addConstructor(constructor);
        }

      }
      else if(!modules.empty() && ent.kind() == cppast::cpp_entity_kind::member_function_t) {
        auto& fn = static_cast<const cppast::cpp_member_function&>(ent);
        if(cppast::has_attribute(fn.attributes(), "scriptable")) {

          auto function = ChaiObject::create<ChaiFunction>(fn.name(), _namespace);
          function->setReturnType(cppast::to_string(fn.return_type()));
          function->isMethodOf(modules.top());

          for(auto& param : fn.parameters()) {
            function->addArgument(cppast::to_string(param.type()), param.name());
          }
          std::cout<<"Found scriptable function: "<<function->getName()<<std::endl;;
          modules.top()->addFunction(function);
        }
      }
    }
    else if(info.event == cppast::visitor_info::container_entity_exit && ent.kind() == cppast::cpp_entity_kind::namespace_t) {
      if(_namespace.find_last_of("::") == std::string::npos)
        _namespace = "";
      else
        _namespace = _namespace.substr(0, _namespace.find_last_of("::"));
    }
    return true;
  });
  std::cout<<"Processed: "<<file->name()<<" into "<<objects.size()<<" modules."<<std::endl;

  return objects;
}

std::stringstream generateRegistrations(std::vector<std::shared_ptr<ChaiObject>> objects, std::vector<std::string> filenames, const bool expanded = false) {
  std::stringstream output;
  std::sort(objects.begin(), objects.end());

  output << "#ifndef GENERATED_REGISTRATIONS_H" << std::endl;
  output << "#define GENERATED_REGISTRATIONS_H" << std::endl;
  output << "// This file was generated by nymph-registrar." << std::endl;
  output << "// Do not edit it, as it will mess up your chaiscript registrations." << std::endl;
  output << "// Do not edit it, as it will be overwritten when nymph-registrar is re-ran." << std::endl;
  output << "#include <chaiscript/utility/utility.hpp>" << std::endl;
  output << "#include <vector>" << std::endl;
  output << "#include <map>" << std::endl;

  for(auto filename : filenames) {
    output << "#include \"" << filename << "\"" << std::endl;
  }

  output << "namespace generated {" << std::endl;

  for(auto& o : objects) {
    output << o->getRegistryString() << std::endl;
  }

  output << "void registerModules(std::shared_ptr<chaiscript::ChaiScript> chai) {" << std::endl;

  for(auto& o : objects) {
    output << "  chai->add(" << o->getRegistryFunctionCall() << ");" << std::endl;
  }
  output << "}" << std::endl;
  output << "}" << std::endl;
  output << "#endif" << std::endl;
  return output;
}


int main(int argc, char** argv) {
  std::string include_dir;
  cppast::libclang_compile_config config;

  cppast::compile_flags flags;

  //flags |= cppast::compile_flag::gnu_extensions;
  config.set_flags(cppast::cpp_standard::cpp_14, flags);
  if(argc < 2) {
    std::cout<<"=                    USAGE:                   ="<<std::endl;
    std::cout<<"nymph-registrar <args> [directory to register]"<<std::endl;
    std::cout<<"----------------Argument list------------------"<<std::endl;
    std::cout<<"= -I/path/to/includes: This is so you can let ="<<std::endl;
    std::cout<<"= clang know includes needed to parse the li- ="<<std::endl;
    std::cout<<"= brary that is being registered.             ="<<std::endl;
    std::cout<<"=                                             ="<<std::endl;
    std::cout<<"= -O/path/to/output/dir: This is where the    ="<<std::endl;
    std::cout<<"= tool will output the generated reigstration ="<<std::endl;
    std::cout<<"= header(s).                                  ="<<std::endl;
    std::cout<<"=                                             ="<<std::endl;
    std::cout<<"= -X: This tells the tool to expand the outp- ="<<std::endl;
    std::cout<<"= ut into multiple files instead of one.      ="<<std::endl;
    return 0;
  }

  std::string directory_to_be_registered = std::string(argv[argc - 1]);
  std::vector<std::string> args;
  std::string output_directory = "./";
  bool expand_files = false;
  bool print_to_stdout = false;

  for(int i=1; i<argc-1; i++) {
    args.push_back(argv[i]);
  }

  for(auto arg : args) {
    if(arg.find("-I") != std::string::npos) {
      arg.erase(0, 2);
      std::cout<<"Include directory: "<<arg<<std::endl;
      config.add_include_dir(arg);
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
  }

  auto files = grabFiles(directory_to_be_registered);

  std::vector<std::shared_ptr<ChaiObject>> processed_objects;

  auto parsed_files = parseFiles(files, config);

  for(auto& file : parsed_files) {
    auto processed_modules = processFile(std::move(file));
    processed_objects.insert(processed_objects.end(), processed_modules.begin(), processed_modules.end());
  }

  std::cout<<"Total processed objects: "<<processed_objects.size()<<std::endl;

  auto registrations = generateRegistrations(processed_objects, files);

  if(print_to_stdout) {
    std::cout<<registrations.str();
  }

  std::ofstream output_file(output_directory + "generated_registrations.h");
  output_file << registrations.str();
  output_file.flush();
  output_file.close();

  return 0;
}
