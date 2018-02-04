#ifndef CHAI_OBJECT_PROCESSOR_H
#define CHAI_OBJECT_PROCESSOR_H
#include <memory>
#include <sstream>
#include <list>
#include <vector>
#include <cppast/cpp_entity.hpp>
#include <cppast/libclang_parser.hpp>
#include "chai_object.h"
#include "chai_module.h"
#include "chai_enum.h"
#include "chai_function.h"

class ChaiObjectProcessor {
private:
  cppast::libclang_compile_config compile_config;
  std::vector<std::string> filenames;
  std::list<std::string> namespaces;
  std::vector<std::shared_ptr<ChaiObject>> objects;
  std::vector<std::shared_ptr<ChaiEnum>> enums;
  std::vector<std::shared_ptr<ChaiModule>> modules;
  std::vector<std::shared_ptr<ChaiFunction>> functions;

  std::string current_namespace;

  std::vector<std::unique_ptr<cppast::cpp_file>> parseFiles(std::vector<std::string>& filenames, const cppast::libclang_compile_config& config, const bool verbose_output = false);
  void processNamespace(const cppast::cpp_entity& ent, const bool verbose_output = false);
  void processModule(const cppast::cpp_entity& ent, const bool verbose_output = false);
  void processEnum(const cppast::cpp_entity& ent, const bool verbose_output = false);
  void processMemberFunction(const cppast::cpp_entity& ent, const bool verbose_output = false);
  void processConstructor(const cppast::cpp_entity& ent, const bool verbose_output = false);
  void processStaticFunction(const cppast::cpp_entity& ent, const bool verbose_output = false);

  std::shared_ptr<ChaiModule> findModuleByName(const std::string& name) const;
public:
  ChaiObjectProcessor() = delete;
  ChaiObjectProcessor(const cppast::libclang_compile_config& config);
  void processObjects(std::vector<std::string> filenames, const bool verbose_output = false);
  std::stringstream generateRegistrations(const bool expanded = false, const bool verbose_output = false) const;

};

#endif
