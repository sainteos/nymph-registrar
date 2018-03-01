#ifndef CHAI_OBJECT_PROCESSOR_H
#define CHAI_OBJECT_PROCESSOR_H
#include <memory>
#include <sstream>
#include <list>
#include <deque>
#include <vector>
#include <iterator>
#include <cppast/cpp_entity.hpp>
#include <cppast/libclang_parser.hpp>
#include "chai_object.h"
#include "chai_module.h"
#include "chai_enum.h"
#include "chai_function.h"
#include "chai_function_template.h"
namespace detail {
  struct ModuleNode {
    std::shared_ptr<ChaiModule> module;
    std::weak_ptr<ModuleNode> parent;
    std::deque<std::shared_ptr<ModuleNode>> children;
  };

  class ModuleTree;

  class ModuleIterator {
  public:
    using difference_type = std::ptrdiff_t;
    using pointer = std::shared_ptr<ModuleNode>;
    using reference = std::shared_ptr<ModuleNode>&;
    using iterator_category = std::bidirectional_iterator_tag;

    ModuleIterator() = default;
    explicit ModuleIterator(std::shared_ptr<ModuleNode> node);
    explicit ModuleIterator(const ModuleTree& tree);

    reference operator*();
    pointer operator->();

    ModuleIterator& operator++();
    ModuleIterator operator++(int);

    ModuleIterator& operator--();
    ModuleIterator operator--(int);

    bool operator==(const ModuleIterator& rhs);
    bool operator!=(const ModuleIterator& rhs);

  private:
    using Nodes = std::deque<std::shared_ptr<ModuleNode>>;

    void store_sorted_nodes(std::shared_ptr<ModuleNode> node);
    void store_sorted_nodes(const ModuleTree& tree);

    Nodes nodes;

    Nodes::size_type current { 0 };
  };

  class ModuleTree {
  using iterator = ModuleIterator;
  using citerator = const ModuleIterator;
  private:
    std::shared_ptr<ModuleNode> root;
    friend class ModuleIterator;
  public:
    ModuleTree();
    void add(std::shared_ptr<ChaiModule> module);
    void remove(std::shared_ptr<ModuleNode> node);
    std::shared_ptr<ModuleNode> findNodeWithModuleName(const std::string& name);

    iterator begin();
    iterator begin() const;
    iterator end();
    iterator end() const;
  };
}

class ChaiObjectProcessor {
private:
  cppast::libclang_compile_config compile_config;
  std::vector<std::string> filenames;
  std::list<std::string> namespaces;
  std::vector<std::shared_ptr<ChaiObject>> objects;
  std::vector<std::shared_ptr<ChaiEnum>> enums;
  detail::ModuleTree module_tree;
  std::vector<std::shared_ptr<ChaiFunction>> functions;
  std::vector<std::shared_ptr<ChaiFunctionTemplate>> function_templates;

  std::string current_namespace;

  bool verbose_parsing;

  std::vector<std::unique_ptr<cppast::cpp_file>> parseFiles(const std::vector<std::string>& filenames, const cppast::libclang_compile_config& config, const bool verbose_parsing = false);
  void processNamespace(const cppast::cpp_entity& ent, const bool verbose_output = false);
  void processModule(const cppast::cpp_entity& ent, const bool verbose_output = false);
  void processEnum(const cppast::cpp_entity& ent, const bool verbose_output = false);
  void processMemberFunction(const cppast::cpp_entity& ent, const bool verbose_output = false);
  void processFunctionTemplate(const cppast::cpp_entity& ent, const bool verbose_output = false);
  void processConstructor(const cppast::cpp_entity& ent, const bool verbose_output = false);
  void processStaticFunction(const cppast::cpp_entity& ent, const bool verbose_output = false);

  std::shared_ptr<ChaiModule> findModuleByName(const std::string& name);
public:
  ChaiObjectProcessor();
  ChaiObjectProcessor(const cppast::libclang_compile_config& config, const bool verbose_processing = false);
  void processObjects(const std::vector<std::string>& filenames, const bool verbose_output = false);
  std::stringstream generateRegistrations(const std::string& _namespace, const bool verbose_output = false);

};

#endif
