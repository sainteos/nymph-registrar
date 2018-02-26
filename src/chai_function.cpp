#include <sstream>
#include <iostream>
#include "chai_function.h"
#include "chai_module.h"

ChaiFunction::ChaiFunction(const std::string& name, const std::string& _namespace, const FunctionType& type, const std::string& return_type,  const bool is_overloaded, const bool is_static)
: ChaiObject(name, _namespace), type(type), return_type(return_type), is_constructor(false), is_overloaded(is_overloaded), is_static(is_static) {

}

void ChaiFunction::setReturnType(const std::string& return_type) noexcept {
  this->return_type = return_type;
}

std::string ChaiFunction::getReturnType() const noexcept {
  return return_type;
}

void ChaiFunction::setConstructor(const bool constructor) {
  type = FunctionType::MEMBER;
  is_constructor = constructor;
}

bool ChaiFunction::isConstructor() const noexcept {
  return is_constructor;
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
  arguments.push_back(std::pair<std::string, std::string>(type, arg));
}

std::vector<std::pair<std::string, std::string>> ChaiFunction::getArguments() const noexcept {
  return arguments;
}

void ChaiFunction::isMethodOf(std::weak_ptr<ChaiModule> module) {
  if(type == FunctionType::GLOBAL){
    type = FunctionType::MEMBER;
  }
  else if(type == FunctionType::GLOBAL_TEMPLATE) {
    type = FunctionType::MEMBER_TEMPLATE;
  }
  this->module = module;
}

FunctionType ChaiFunction::getFunctionType() const noexcept {
  return type;
}

std::string ChaiFunction::toString() const noexcept {
  return (this->module.use_count() > 0 ? this->module.lock()->getName() + "::" : "") + getName();
}

std::string ChaiFunction::getRegistryString() {
  std::stringstream reg;
  auto collapse_args = [&]() {
    std::string args;
    for(auto& arg : arguments) {
      args += arg.first + " " + arg.second + ", ";
    }
    if(!args.empty())
      args.pop_back();
    if(!args.empty())
      args.pop_back();
    return args;
  };

  auto collapse_arg_names = [&]() {
    std::string args;
    for(auto& arg : arguments) {
      args += arg.second + ", ";
    }
    if(!args.empty())
      args.pop_back();
    if(!args.empty())
      args.pop_back();
    return args;
  };
  reg << "{";
  if(is_constructor) {
    reg << "chaiscript::constructor<" << getName() << "(" << collapse_args() << ")>()";
  }
  else if(!is_constructor && is_overloaded) {
    reg << "chaiscript::fun(";
    reg << "[](" << this->module.lock()->getName() << "& c";
    reg << (arguments.size() > 0 ? ", " + collapse_args() : "") << ") ";
    reg << "{ return c." << getName() << "(" << (arguments.size() > 0 ? collapse_arg_names() : "") << "); }), \"" << getName() << "\"";
  }
  else {
    reg << "chaiscript::fun(";
    reg << "&" << this->module.lock()->getName() << "::" << getName() << "), \"";
    if(is_static) {
      reg << this->module.lock()->getName() << "_";
    }
    auto chai_name = getName();
    if(chai_name.find("operator") != std::string::npos) {
      chai_name.replace(0, 8, "");
    }
    reg << chai_name << "\"";
  }
  reg << "},\n";

  return reg.str();
}
