#include <sstream>
#include <iostream>
#include <memory>
#include <set>
#include "chai_module.h"

ChaiModule::ChaiModule(const std::string& _class, const std::string& _namespace) : ChaiObject(_class, _namespace) {

}

void ChaiModule::addFunction(std::unique_ptr<ChaiFunction> function) {
  if(hasFunction(function->getName())) {
    auto func = std::find_if(functions.begin(), functions.end(), [&](const std::unique_ptr<ChaiFunction>& f) { return function->getName() == f->getName(); });

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
  if(functions.size() == 0)
    return false;

  auto func = std::find_if(functions.begin(), functions.end(), [&](const std::unique_ptr<ChaiFunction>& f) { return name == f->getName(); });
  return func != functions.end();
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

std::set<std::shared_ptr<ChaiModule>> ChaiModule::getAllBases() const {
  std::set<std::shared_ptr<ChaiModule>> bases;
  for(auto b : raw_bases) {
    std::string ns;
    std::string cs;
    if(b.find_last_of("::") != std::string::npos) {
      ns = b.substr(0, b.find_last_of("::")-1);
      cs = b.substr(b.find_last_of("::")+1, b.size() - b.find_last_of("::"));
      if(hasObjectBeenRegistered(cs, ns))
      {
        auto m = std::dynamic_pointer_cast<ChaiModule>(getRegisteredObject(cs, ns));
        if(m) {
          bases.insert(m);
          auto m_bases = m->getAllBases();
          if(!m_bases.empty()) {
            bases.insert(m_bases.begin(), m_bases.end());
          }
        }
      }
    }
    else {
      cs = b;
      ns = getNamespace();
      if(hasObjectBeenRegistered(cs, ns))
      {
        auto m = std::dynamic_pointer_cast<ChaiModule>(getRegisteredObject(cs, ns));
        if(m) {
          bases.insert(m);
          auto m_bases = m->getAllBases();
          if(!m_bases.empty()) {
            bases.insert(m_bases.begin(), m_bases.end());
          }
        }
      }
      else if(hasObjectBeenRegistered(cs, "")) {
        auto m = std::dynamic_pointer_cast<ChaiModule>(getRegisteredObject(cs, ""));
        if(m) {
          bases.insert(m);
          auto m_bases = m->getAllBases();
          if(!m_bases.empty()) {
            bases.insert(m_bases.begin(), m_bases.end());
          }
        }
      }
    }
  }

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

std::string ChaiModule::getRegistryString() const {
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
