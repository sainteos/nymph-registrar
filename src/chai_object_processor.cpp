#include "chai_object_processor.h"
#include <iostream>
#include <sstream>
#include <list>
#include <cppast/cpp_namespace.hpp>
#include <cppast/parser.hpp>
#include <cppast/libclang_parser.hpp>
#include <cppast/compile_config.hpp>
#include <cppast/visitor.hpp>
#include <cppast/code_generator.hpp>
#include <cppast/cpp_entity_kind.hpp>
#include <cppast/cpp_forward_declarable.hpp>
#include <cppast/cpp_function.hpp>
#include <cppast/cpp_member_function.hpp>
#include <cppast/cpp_enum.hpp>
#include <cppast/cpp_type.hpp>
#include <cppast/cpp_class.hpp>
#include <cppast/cpp_preprocessor.hpp>
#include <cppast/cpp_function_template.hpp>

namespace detail {
  ModuleIterator::ModuleIterator(std::shared_ptr<ModuleNode> node) {
    store_sorted_nodes(node);
    nodes.push_back(nullptr);
  }

  ModuleIterator::ModuleIterator(const ModuleTree& tree) {
    store_sorted_nodes(tree);
    nodes.push_back(nullptr);
  }

  ModuleIterator::reference ModuleIterator::operator*() {
    return nodes[current];
  }

  ModuleIterator::pointer ModuleIterator::operator->() {
    return nodes[current];
  }

  ModuleIterator& ModuleIterator::operator++() {
    ++current;
    return *this;
  }

  ModuleIterator ModuleIterator::operator++(int) {
    auto tmp = *this;
    ++current;
    return tmp;
  }

  ModuleIterator& ModuleIterator::operator--() {
    --current;
    return *this;
  }

  ModuleIterator ModuleIterator::operator--(int) {
    auto tmp = *this;
    --current;
    return tmp;
  }

  bool ModuleIterator::operator==(const ModuleIterator& rhs) {
    return nodes[current] == rhs.nodes[rhs.current];
  }

  bool ModuleIterator::operator!=(const ModuleIterator& rhs) {
    return !(*this == rhs);
  }

  void ModuleIterator::store_sorted_nodes(const ModuleTree& tree) {
    for(auto&& node : tree.root->children) {
      nodes.push_back(node);
      for(auto c : node->children) {
        store_sorted_nodes(c);
      }
    }
  }

  void ModuleIterator::store_sorted_nodes(std::shared_ptr<ModuleNode> node) {
    if(node) {
      nodes.push_back(node);
      for(auto c : node->children) {
        store_sorted_nodes(c);
      }
    }
  }

  std::string stripScope(const std::string& s) {
    if(s.find_last_of("::") != std::string::npos) {
      return s.substr(s.find_last_of("::") + 1, s.size() - s.find_last_of("::") - 1);
    }
    else {
      return s;
    }
  }

  std::shared_ptr<ModuleNode> findNodeWithModuleName(const ModuleTree& start, const std::string& name) {
    auto node = std::find_if(start.begin(), start.end(), [&](std::shared_ptr<ModuleNode> n) {
      return name == n->module->getName();
    });

    return node != start.end() ? *node : nullptr;
  }

  std::shared_ptr<ModuleNode> findNodeBaseWithModuleName(const ModuleTree& start, const std::string& name) {
    auto node = std::find_if(start.begin(), start.end(), [&](std::shared_ptr<ModuleNode> n) {
      for(auto b : n->module->getRawBases()) {
        if(stripScope(b) == name) {
          return true;
        }
      }
      return false;
    });
    return node != start.end() ? *node : nullptr;
  }

  std::vector<std::shared_ptr<ModuleNode>> findAllNodesWithBaseModuleName(const ModuleTree& start, const std::string& name) {
    std::vector<std::shared_ptr<ModuleNode>> nodes;

    for(auto node : start) {
      if(node->module) {
        for(auto b : node->module->getRawBases()) {
          if(stripScope(b) == name) {
            nodes.push_back(node);
          }
        }
      }
    }

    return nodes;
  }

  ModuleTree::ModuleTree() : root(std::make_shared<ModuleNode>()) {

  }

  void ModuleTree::add(std::shared_ptr<ChaiModule> module) {
    auto new_node = std::make_shared<ModuleNode>();
    new_node->module = module;

    auto nodes_with_base = findAllNodesWithBaseModuleName(*this, new_node->module->getName());

    for(auto&& node : nodes_with_base) {
      remove(node);
      new_node->children.push_back(node);
      node->parent.reset();
      node->parent = new_node;
    }

    for(auto b : new_node->module->getRawBases()) {
      auto node = findNodeWithModuleName(stripScope(b));
      if(node) {
        if(!new_node->parent.expired()) {
          remove(new_node);
        }
        node->children.push_back(new_node);
        new_node->parent.reset();
        new_node->parent = node;
      }
    }

    if(new_node->parent.expired()) {
      root->children.push_back(new_node);
      new_node->parent = root;
    }
  }

  void ModuleTree::remove(std::shared_ptr<ModuleNode> node) {
    for(auto&& n : *this) {
      if(node == n) {
        if(!n->parent.expired()) {
          auto beg = n->parent.lock()->children.begin();
          auto end = n->parent.lock()->children.end();
          n->parent.lock()->children.erase(std::remove(beg, end, n));
          n->parent.reset();
          break;
        }
        if(n->parent.expired()) {
          root->children.erase(std::remove(root->children.begin(), root->children.end(), n));
          break;
        }
      }
    }
  }

  std::shared_ptr<ModuleNode> ModuleTree::findNodeWithModuleName(const std::string &name) {
    return detail::findNodeWithModuleName(*this, name);
  }

  ModuleTree::iterator ModuleTree::begin() {
    return iterator(*this);
  }

  ModuleTree::iterator ModuleTree::end() {
    return iterator(nullptr);
  }

  ModuleTree::iterator ModuleTree::begin() const {
    return iterator(*this);
  }

  ModuleTree::iterator ModuleTree::end() const {
    return iterator(nullptr);
  }

  std::ostream& operator<< (std::ostream& out, const ModuleTree& tree) {
    for(auto node : tree) {
      if(node->module) {
        out<<node->module->getNamespace()<<"::"<<node->module->getName()<<", ";
      }
      else {
        out<<"{{null node}}, ";
      }
      out<<"With children: {";
      for(auto child : node->children) {
        if(child && child->module) {
          out<<child->module->getNamespace()<<"::"<<child->module->getName()<<", ";
        }
      }
      out<<"}"<<std::endl;
    }
    return out;
  }
}

ChaiObjectProcessor::ChaiObjectProcessor() : compile_config(), current_namespace(""), verbose_parsing(false) {

}

ChaiObjectProcessor::ChaiObjectProcessor(const cppast::libclang_compile_config& config, const bool verbose_parsing) : compile_config(config), current_namespace(""), verbose_parsing(verbose_parsing) {

}

void ChaiObjectProcessor::processObjects(const std::vector<std::string>& filenames, const bool verbose_output){
  this->filenames = filenames;

  auto parsed_files = parseFiles(this->filenames, this->compile_config, verbose_output);

  for(auto& file : parsed_files) {
    if(verbose_output)
      std::cout<<"Processing "<<file->name()<<"..."<<std::endl;
    cppast::visit(*file, [&](const cppast::cpp_entity& ent, cppast::visitor_info info) {
      if(info.event != cppast::visitor_info::container_entity_exit) {
        if(ent.kind() == cppast::cpp_entity_kind::namespace_t) {
          processNamespace(ent, verbose_output);
        }
        else if(cppast::has_attribute(ent.attributes(), "scriptable")) {
          if(ent.kind() == cppast::cpp_entity_kind::enum_t) {
            processEnum(ent, verbose_output);
          }
          else if(ent.kind() == cppast::cpp_entity_kind::class_t) {
            processModule(ent, verbose_output);
          }
          else if(ent.kind() == cppast::cpp_entity_kind::constructor_t) {
            processConstructor(ent, verbose_output);
          }
          else if(ent.kind() == cppast::cpp_entity_kind::member_function_t) {
            processMemberFunction(ent, verbose_output);
          }
          else if(ent.kind() == cppast::cpp_entity_kind::function_t) {
            processStaticFunction(ent, verbose_output);
          }
          else if(ent.kind() == cppast::cpp_entity_kind::function_template_t)
          {
            processFunctionTemplate(ent, verbose_output);
          }
        }
      }
      else if(info.event == cppast::visitor_info::container_entity_exit && ent.kind() == cppast::cpp_entity_kind::namespace_t) {
        if(current_namespace.find_last_of("::") == std::string::npos)
          current_namespace.clear();
        else
          current_namespace = current_namespace.substr(0, current_namespace.find_last_of("::") - 1);
      }
      return true;
    });
  }

  std::cout<<"Total processed objects: "<<objects.size()<<std::endl;
}

std::stringstream ChaiObjectProcessor::generateRegistrations(const std::string& _namespace, const bool verbose_output) {
  std::stringstream output;

  if(verbose_output)
    std::cout<<"Writing prefix..."<<std::endl;

  output << "#ifndef GENERATED_REGISTRATIONS_H" << std::endl;
  output << "#define GENERATED_REGISTRATIONS_H" << std::endl;
  output << "// This file was generated by nymph-registrar." << std::endl;
  output << "// Do not edit it, as it will mess up your chaiscript registrations." << std::endl;
  output << "// Do not edit it, as it will be overwritten when nymph-registrar is re-ran." << std::endl;
  output << "#include <chaiscript/utility/utility.hpp>" << std::endl;
  output << "#include <vector>" << std::endl;
  output << "#include <map>" << std::endl;

  if(verbose_output)
    std::cout<<"Writing filenames..."<<std::endl;
  for(auto filename : filenames) {
    output << "#include \"" << filename << "\"" << std::endl;
  }
  if(_namespace != "")
    output << "namespace "<<_namespace<< " {"<<std::endl;
  output << "namespace generated {" << std::endl;
  if(verbose_output)
    std::cout<<"Writing namespaces..."<<std::endl;
  for(auto n : namespaces) {
    output << "using namespace " << n << ";" <<std::endl;
  }
  if(verbose_output)
    std::cout<<"Writing enums..."<<std::endl;
  for(auto& e : enums) {
    if(verbose_output)
      std::cout<<"Writing \""<<e->getName()<<"\"..."<<std::endl;
    output << e->getRegistryString() << std::endl;
  }
  if(verbose_output)
    std::cout<<"Writing functions..."<<std::endl;
  for(auto& f : functions) {
    if(verbose_output)
      std::cout<<"Writing \""<<f->getName()<<"\"..."<<std::endl;
    output << f->getRegistryString() << std::endl;
  }
  if(verbose_output)
    std::cout<<"Writing function templates..."<<std::endl;
  for(auto& ft : function_templates) {
    if(verbose_output)
      std::cout<<"Writing \""<<ft->getName()<<"\"..."<<std::endl;
    output << ft->getRegistryString() << std::endl;
  }
  if(verbose_output)
    std::cout<<"Writing modules..."<<std::endl;
  for(auto m : module_tree) {
    if(m->module) {
      if(verbose_output)
        std::cout<<"Writing \""<<m->module->getName()<<"\"..."<<std::endl;
      output << m->module->getRegistryString() << std::endl;
    }
  }

  output << "void registerModules(std::shared_ptr<chaiscript::ChaiScript> chai) {" << std::endl;

  for(auto& o : objects) {
    output << "  chai->add(" << o->getRegistryFunctionCall() << ");" << std::endl;
  }
  output << "}" << std::endl;
  output << "}" << std::endl;
  if(_namespace != "")
    output << "}" << std::endl;
  output << "#endif" << std::endl;
  if(verbose_output)
    std::cout<<"Finished writing!!!"<<std::endl;
  return output;
}

std::vector<std::unique_ptr<cppast::cpp_file>> ChaiObjectProcessor::parseFiles(const std::vector<std::string>& filenames, const cppast::libclang_compile_config& config, const bool verbose_output) {
  cppast::cpp_entity_index idx;
  std::vector<std::unique_ptr<cppast::cpp_file>> parsed_files;

  if(!verbose_parsing) {
    cppast::libclang_parser parser;
    for(auto f : filenames) {
      if(verbose_output)
        std::cout<<"Parsing file(nonverbosely): "<<f<<std::endl;
      auto file = parser.parse(idx, f, config);
      if(parser.error()) {
        std::cout<<"There was a parser error when parsing "<<f<<std::endl;
        parser.reset_error();
      }
      else {
        parsed_files.push_back(std::move(file));
      }
    }
  }
  else {
    cppast::stderr_diagnostic_logger logger;
    logger.set_verbose(true);
    cppast::libclang_parser parser(type_safe::ref(logger));
    for(auto f : filenames) {
      if(verbose_output)
        std::cout<<"Parsing file(verbosely): "<<f<<std::endl;
      auto file = parser.parse(idx, f, config);
      if(!file || parser.error()) {
        std::cout<<"There was a parser error when parsing "<<f<<std::endl;
        if(parser.error())
          parser.reset_error();
      }
      else {
        parsed_files.push_back(std::move(file));
      }
    }
  }

  return std::move(parsed_files);
}

void ChaiObjectProcessor::processNamespace(const cppast::cpp_entity& ent, const bool verbose_output) {
  auto& ns = static_cast<const cppast::cpp_namespace&>(ent);
  if(current_namespace.empty())
    current_namespace = ns.name();
  else
    current_namespace = current_namespace + "::" + ns.name();

  if(std::count(namespaces.begin(), namespaces.end(), current_namespace) == 0) {
    namespaces.push_back(current_namespace);
    if(verbose_output)
      std::cout<<"Found new namespace: "<<current_namespace<<std::endl;
  }
}

void ChaiObjectProcessor::processModule(const cppast::cpp_entity& ent, const bool verbose_output) {
  auto& cs = static_cast<const cppast::cpp_class&>(ent);
  if(!cs.is_declaration()) {
    auto _class = cs.name();
    auto module = ChaiObject::create<ChaiModule>(_class, current_namespace);

    if(verbose_output)
      std::cout<<"Found scriptable module: "<<_class<<" in namespace: "<<current_namespace<<" with base classes: ";

    for(auto& base : cs.bases()) {
      module->addRawBase(base.name());
      if(verbose_output)
        std::cout<<base.name()<<", ";
    }
    if(verbose_output)
      std::cout<<std::endl;

    module_tree.add(module);
    objects.push_back(module);
  }
}

void ChaiObjectProcessor::processEnum(const cppast::cpp_entity& ent, const bool verbose_output) {
  auto& en = static_cast<const cppast::cpp_enum&>(ent);
  if(!en.is_declaration()) {
    auto chai_enum = ChaiObject::create<ChaiEnum>(en.name(), current_namespace);
    for(auto it = en.begin(); it != en.end(); it++) {
      chai_enum->addValue(it->name());
    }

    if(en.parent().has_value() && en.parent().value().kind() == cppast::cpp_entity_kind::class_t) {
      chai_enum->inClass(en.parent().value().name());
    }
    if(verbose_output)
      std::cout<<"Found enum "<<en.name()<<std::endl;
    objects.push_back(chai_enum);
    enums.push_back(chai_enum);
  }
}

void ChaiObjectProcessor::processMemberFunction(const cppast::cpp_entity& ent, const bool verbose_output) {
  auto& fn = static_cast<const cppast::cpp_member_function&>(ent);

  //parent is the template?
  if(fn.parent().value().name() == fn.name())
    return;

  auto module = findModuleByName(fn.parent().value().name());

  if(!module) {
    throw(std::runtime_error("Couldn't find module: " + fn.parent().value().name() + " for member function!"));
  }

  //We don't want to process function templates twice
  if(module->hasFunction(fn.parent().value().name()) && module->getFunction(fn.parent().value().name())->getFunctionType() == FunctionType::MEMBER_TEMPLATE) {
    return;
  }

  auto function = std::make_shared<ChaiFunction>(fn.name(), current_namespace);
  function->setReturnType(cppast::to_string(fn.return_type()));
  function->isMethodOf(module);

  for(auto& param : fn.parameters()) {
    function->addArgument(cppast::to_string(param.type()), param.name());
  }
  if(verbose_output)
    std::cout<<"Found scriptable function: "<<function->getName()<<std::endl;;
  module->addFunction(std::move(function));
}

namespace detail {
  std::vector<std::string> split_args(const std::string& arg_list) {
    std::vector<std::string> out;

    auto unprocessed_args = arg_list;

    while(unprocessed_args.find_first_of(",") != std::string::npos) {
      out.push_back(unprocessed_args.substr(0, unprocessed_args.find_first_of(",")));
      unprocessed_args.erase(0, unprocessed_args.find_first_of(",") + 1);
    }
    if(!unprocessed_args.empty())
      out.push_back(unprocessed_args);
    return out;
  }
}

void ChaiObjectProcessor::processFunctionTemplate(const cppast::cpp_entity& ent, const bool verbose_output) {
  auto& ft = static_cast<const cppast::cpp_function_template&>(ent);

  auto function_template = std::make_shared<ChaiFunctionTemplate>(ft.name(), current_namespace);
  auto module = findModuleByName(ft.parent().value().name());
  if(module) {
    function_template->isMethodOf(module);
  }
  function_template->setReturnType(cppast::to_string(static_cast<const cppast::cpp_function&>(ft.function()).return_type()));

  for(auto& param : ft.function().parameters()) {
    function_template->addArgument(cppast::to_string(param.type()), param.name());
  }

  for(auto& param : ft.parameters()) {
    function_template->addTemplateVariable(param.name());
  }

  auto args = cppast::has_attribute(ent.attributes(), "scriptable").value().arguments();
  auto args_str = args.value().as_string();
  std::string subloc;

  //Substitution Location
  if(args_str.find_first_of(',') < args_str.find_first_of('{')) {
    subloc = args_str.substr(0, args_str.find_first_of(','));
    args_str.erase(0, args_str.find_first_of(',') + 1);

    if(subloc == "BEFORE") {
      function_template->setSubstitutionLocation(SubstitutionLocation::BEFORE);
    }
    else if(subloc == "AFTER") {
      function_template->setSubstitutionLocation(SubstitutionLocation::AFTER);
    }
    else if(subloc == "MUTATOR") {
      function_template->setSubstitutionLocation(SubstitutionLocation::MUTATOR);
    }
  }

  int num_arg_lists = std::count(args_str.begin(), args_str.end(), '{');
  int closing_braces = std::count(args_str.begin(), args_str.end(), '}');

  if(num_arg_lists != closing_braces) {
    throw(std::runtime_error("Open/close brace(\"{\"}) mismatch!"));
  }

  if(function_template->getNumTemplateVariables() != num_arg_lists) {
    std::stringstream out;
    out << "Mismatch number of template arguments: "<<function_template->getNumTemplateVariables()<<" to attribute argument lists: "<<num_arg_lists;
    throw(std::runtime_error(out.str()));
  }

  for(int i = 0; i < num_arg_lists; i++) {
    //Copy off the arg list
    auto arg_list = args_str.substr(args_str.find_first_of("{") + 1, args_str.find_first_of("}") - args_str.find_first_of("{") - 1);
    //Erase it from the original args str
    args_str.erase(args_str.find_first_of("{"), args_str.find_first_of("}") - args_str.find_first_of("{") + 1);
    //Erase comma
    if(args_str.find_first_of(",") != std::string::npos)
      args_str.erase(args_str.find_first_of(","), 1);
    auto args = detail::split_args(arg_list);
    for(auto arg : args) {
      function_template->addSubstitutionType(function_template->getTemplateVariables()[i], arg);
    }
  }
  if(verbose_output)
    std::cout<<"Found scriptable function template: "<<function_template->getName()<<std::endl;
  if(module) {
    module->addFunction(function_template);
  }
  else {
    function_templates.push_back(function_template);
  }
}

void ChaiObjectProcessor::processConstructor(const cppast::cpp_entity& ent, const bool verbose_output) {
  auto& ct = static_cast<const cppast::cpp_constructor&>(ent);

  auto constructor = std::make_unique<ChaiFunction>(ct.name(), current_namespace);
  auto module = findModuleByName(ct.parent().value().name());
  if(!module) {
    throw(std::runtime_error("Couldn't find module: " + ct.parent().value().name() + " for constructor!"));
  }
  constructor->setReturnType("");
  constructor->isMethodOf(module);
  constructor->setConstructor(true);

  for(auto& param : ct.parameters()) {
    constructor->addArgument(cppast::to_string(param.type()), param.name());
  }

  if(verbose_output)
    std::cout<<"Found scriptable constructor: "<<constructor->getName()<<std::endl;
  module->addConstructor(std::move(constructor));
}

void ChaiObjectProcessor::processStaticFunction(const cppast::cpp_entity& ent, const bool verbose_output) {
  auto& fn = static_cast<const cppast::cpp_function&>(ent);
  auto function = std::make_unique<ChaiFunction>(fn.name(), current_namespace);
  function->setReturnType(cppast::to_string(fn.return_type()));
  function->setStatic(cppast::is_static(fn.storage_class()));

  for(auto& param : fn.parameters()) {
    function->addArgument(cppast::to_string(param.type()), param.name());
  }

  if(verbose_output) {
    std::cout<<"Found scriptable"<<(function->isStatic() ? "(static)" : "")<<" function: "<<function->getName()<<std::endl;
  }
  if(function->isStatic()) {
    auto module = findModuleByName(fn.parent().value().name());
    function->isMethodOf(module);
    module->addFunction(std::move(function));
  }
}

std::shared_ptr<ChaiModule> ChaiObjectProcessor::findModuleByName(const std::string& name) {
  return module_tree.findNodeWithModuleName(name) ? module_tree.findNodeWithModuleName(name)->module : nullptr;
}
