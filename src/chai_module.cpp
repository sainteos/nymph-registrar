#include <sstream>
#include <iostream>
#include <memory>
#include <list>
#include "chai_module.h"

ChaiModule::ChaiModule(const std::string& _class, const std::string& _namespace) : ChaiObject(_class, _namespace) {

}

void ChaiModule::addFunction(std::shared_ptr<ChaiFunction> function) {
  if(hasFunction(function->getName())) {
    auto func = std::find_if(functions.begin(), functions.end(), [&](const std::shared_ptr<ChaiFunction>& f) { return function->getName() == f->getName(); });

    (*func)->setOverloaded(true);
    function->setOverloaded(true);
  }
  functions.push_back(std::move(function));
}

void ChaiModule::addConstructor(std::unique_ptr<ChaiFunction> constructor) {
  if(constructors.size() == 1) {
    constructors.back()->setOverloaded(true);
  }
  if(constructors.size() >= 1) {
    constructor->setOverloaded(true);
  }
  constructors.push_back(std::move(constructor));
}

bool ChaiModule::hasFunction(const std::string& name) const {
  return getFunction(name) != nullptr;
}

std::shared_ptr<ChaiFunction> ChaiModule::getFunction(const std::string& name) const {
  auto func = std::find_if(functions.begin(), functions.end(), [&](const std::shared_ptr<ChaiFunction>& f) { return name == f->getName(); });
  return func != functions.end() ? *func : nullptr;
}

void ChaiModule::addRawBase(const std::string& name) {
  raw_bases.push_back(name);
}

void ChaiModule::addRawBases(const std::vector<std::string>& bases) {
  raw_bases = bases;
}

std::vector<std::string> ChaiModule::getRawBases() const noexcept {
  return raw_bases;
}

std::vector<std::shared_ptr<ChaiModule>> ChaiModule::getBases(const std::string& ns, const std::string& cs) const {
  std::vector<std::shared_ptr<ChaiModule>> bases;

  if(hasObjectBeenRegistered(cs, ns)) {
    std::cout<<"Object has been registered: "<<ns<<"::"<<cs<<std::endl;
    auto m = std::dynamic_pointer_cast<ChaiModule>(getRegisteredObject(cs, ns));
    if(m) {
      bases.push_back(m);
      auto m_bases = m->getAllBases();
      if(!m_bases.empty()) {
        bases.insert(bases.end(), m_bases.begin(), m_bases.end());
      }
    }
  }
  else {
    std::cout<<"Object has not been registered: "<<ns<<"::"<<cs<<std::endl;
  }
  return bases;
}

std::list<std::shared_ptr<ChaiModule>> ChaiModule::getAllBases() const {
  std::list<std::shared_ptr<ChaiModule>> bases;
  for(auto b : raw_bases) {
    std::string ns;
    std::string cs;
    if(b.find_last_of("::") != std::string::npos) {
      ns = b.substr(0, b.find_last_of("::")-1);
      cs = b.substr(b.find_last_of("::")+1, b.size() - b.find_last_of("::"));
      auto b_bases = getBases(ns, cs);
      if(!b_bases.empty()) {
        bases.insert(bases.end(), b_bases.begin(), b_bases.end());
      }
    }
    else {
      cs = b;
      std::string n = getNamespace().substr(0, ns.find_last_of("::") - 1);
      do {
        auto b_bases = getBases(n, cs);

        if(!b_bases.empty()) {
          bases.insert(bases.end(), b_bases.begin(), b_bases.end());
          break;
        }
        if(n.find_last_of("::") != std::string::npos)
          n = getNamespace().substr(0, n.find_last_of("::") - 1);
        else
          n = "";
      } while(!n.empty());

      auto n_bases = getBases("", cs);

      if(!n_bases.empty()) {
        bases.insert(bases.end(), n_bases.begin(), n_bases.end());
      }
    }
  }
  bases.unique();
  return bases;
}

bool ChaiModule::operator<(ChaiObject& other) const {
  try {
    auto& other_module = dynamic_cast<ChaiModule&>(other);
    return this->raw_bases.size() < other_module.raw_bases.size();
  } catch(std::bad_cast e) {
    return false;
  }
}

std::string ChaiModule::getRegistryString() {
  std::stringstream reg;
  reg << "chaiscript::ModulePtr " << getRegistryFunctionCall() << " {\n";
  reg << "  chaiscript::ModulePtr module = std::make_shared<chaiscript::Module>();\n";
  reg << "  chaiscript::utility::add_class<" << getName() << ">(*module, std::string(\"" << getName() << "\"),\n";
  reg << "  {\n";

  //Add module constructors
  for(auto& c : constructors) {
    reg << c->getRegistryString();
  }

  if(constructors.size() > 0) {
    reg.seekp(static_cast<long>(reg.tellp()) - 2);
    reg << "\n";
  }
  reg << "  },\n";
  reg << "  {\n";

  //Add module functions
  for(auto& f : functions) {
    reg << f->getRegistryString();
  }
  if(functions.size() > 0) {
    reg.seekp(static_cast<long>(reg.tellp()) - 2);
    reg << "\n";
  }
  reg << "  });\n";

  auto bases = getAllBases();

  for(auto b : bases) {
    reg << "  module->add(chaiscript::base_class<" << b->getName() << ", " << getName() << ">());\n";
  }

  reg << "  return module;\n";
  reg << "}\n";

  return reg.str();
}
