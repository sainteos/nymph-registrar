#include <sstream>
#include <iostream>
#include "chai_function.h"
#include "chai_module.h"

ChaiFunction::ChaiFunction(const std::string& name, const std::string& _namespace, const std::string& return_type,  const bool is_overloaded, const bool is_static)
: ChaiObject(name, _namespace), return_type(return_type), is_overloaded(is_overloaded), is_static(is_static) {

}

void ChaiFunction::setReturnType(const std::string& return_type) noexcept {
  this->return_type = return_type;
}

std::string ChaiFunction::getReturnType() const noexcept {
  return return_type;
}

void ChaiFunction::setOverloaded(const bool overloaded) {
  is_overloaded = overloaded;
}

bool ChaiFunction::isOverloaded() const noexcept {
  return is_overloaded;
}

void ChaiFunction::setStatic(const bool is_static) {
  this->is_static = is_static;
}
bool ChaiFunction::isStatic() const noexcept {
  return is_static;
}

void ChaiFunction::addArgument(const std::string& type, const std::string& arg) {
  arguments.insert(std::pair<std::string, std::string>(type, arg));
}

std::multimap<std::string, std::string> ChaiFunction::getArguments() const noexcept {
  return arguments;
}

void ChaiFunction::isMethodOf(std::weak_ptr<ChaiModule> module) {
  this->module = module;
}

std::string ChaiFunction::toString() const noexcept {
  return getNamespace() + "::" + (this->module.use_count() > 0 ? this->module.lock()->getName() + "::" : "") + getName();
}

std::string ChaiFunction::getRegistryString() const {
  std::stringstream reg;
  auto collapse_args = [&]() {
    std::string args;
    for(auto& arg : arguments) {
      args += arg.first + " " + arg.second + ", ";
    }
    args.pop_back();
    args.pop_back();
    return args;
  };

  reg << "chaiscript::fun(";
  if(is_overloaded) {
    reg << "[](" << (getNamespace() != "" ? (getNamespace() + "::") : "" ) << this->module.lock()->getName() << "& c";
    reg << (arguments.size() > 0 ? ", " + collapse_args() : "") << ") ";
    reg << "{ c." << getName() << "(" << (arguments.size() > 0 ? collapse_args() : "") << "); }), \"" << getName() << "\"";
  }
  else {
    reg << "&" << (getNamespace() != "" ? (getNamespace() + "::") : "" ) << this->module.lock()->getName() << "::" << getName() << "), \"";
    if(is_static) {
      reg << this->module.lock()->getName() << "_";
    }
    auto chai_name = getName();
    if(chai_name.find("operator") != std::string::npos) {
      chai_name.replace(0, 8, "");
    }
    reg << chai_name << "\"";
  }

  return reg.str();
}
