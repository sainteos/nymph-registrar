#include <sstream>
#include <iostream>
#include "chai_module.h"

ChaiModule::ChaiModule(const std::string& _class, const std::string& _namespace) : ChaiObject(_class, _namespace) {

}

void ChaiModule::addFunction(std::shared_ptr<ChaiFunction> function) {
  if(function_names.count(function->getName()) == 0) {
    function_names[function->getName()] = function;
  }
  else {
    function->setOverloaded(true);
    if(!function_names[function->getName()]->isOverloaded()) {
      function_names[function->getName()]->setOverloaded(true);
    }
  }
  functions.push_back(function);
}

std::vector<std::shared_ptr<ChaiFunction>>& ChaiModule::getFunctions() noexcept {
  return functions;
}

void ChaiModule::addConstructor(std::shared_ptr<ChaiFunction> constructor) {
  if(constructors.size() == 1) {
    constructors.back()->setOverloaded(true);
  }
  if(constructors.size() >= 1) {
    constructor->setOverloaded(true);
  }
  constructors.push_back(constructor);
}

std::vector<std::shared_ptr<ChaiFunction>>& ChaiModule::getConstructors() noexcept {
  return constructors;
}

void ChaiModule::addRawBase(const std::string& name) {
  raw_bases.push_back(name);
}

void ChaiModule::addRawBases(std::vector<std::string> bases) {
  raw_bases = bases;
}

bool ChaiModule::operator<(const ChaiObject& other) const {
  try {
    auto other_module = dynamic_cast<const ChaiModule&>(other);
    return this->raw_bases.size() < other_module.raw_bases.size();
  } catch(std::bad_cast e) {
    return false;
  }
}

std::string ChaiModule::getRegistryString() const {
  std::stringstream reg;
  reg << "chaiscript::ModulePtr " << getRegistryFunctionCall() << " {\n";
  reg << "  chaiscript::ModulePtr module = std::make_shared<chaiscript::Module>();\n";
  reg << "  chaiscript::utility::add_class<" << (getNamespace() != "" ? (getNamespace() + "::") : "" ) << getName() << ">(*module, std::string(\"" << getName() << "\"),\n";
  reg << "  {\n";

  //Add module constructors
  for(auto& c : constructors) {
    reg << "    {" << c->getRegistryString() << "},\n";
  }

  if(constructors.size() > 0) {
    reg.seekp(static_cast<long>(reg.tellp()) - 2);
    reg << "\n";
  }
  reg << "  },\n";
  reg << "  {\n";

  //Add module functions
  for(auto& f : functions) {
    reg << "    {" << f->getRegistryString() << "},\n";
  }
  if(functions.size() > 0) {
    reg.seekp(static_cast<long>(reg.tellp()) - 2);
    reg << "\n";
  }
  reg << "  });\n";

  //Go through all the bases to find the ones that are actually scriptable
  for(auto b : raw_bases) {
    //get rid of inheritance access specifiers
    if(b.find("public") != std::string::npos)
      b.replace(b.find("public"), 6, "");
    if(b.find("protected") != std::string::npos)
      b.replace(b.find("protected"), 9, "");
    if(b.find("private") != std::string::npos)
      b.replace(b.find("private"), 7, "");

    //Extract namespace(s)
    auto ns = b.substr(0, b.find_last_of("::")-1);
    //Extract class
    auto cs = b.substr(b.find_last_of("::")+1, b.size() - b.find_last_of("::"));

    //Check to see if this base class has been registered
    //if it hasn't, we don't pay attention to it
    if(hasObjectBeenRegistered(cs, ns))
    {
      reg << "  module->add(chaiscript::base_class<" << (ns != "" ? (ns + "::") : "" ) << cs << ", " << (getNamespace() != "" ? (getNamespace() + "::") : "" ) << getName() << ">());\n";
    }
  }

  reg << "  return module;\n";
  reg << "}\n";

  return reg.str();
}
