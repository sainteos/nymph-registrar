#include <memory>
#include <iostream>
#include <sstream>
#include "chai_object.h"

std::vector<std::weak_ptr<ChaiObject>> ChaiObject::object_registry = std::vector<std::weak_ptr<ChaiObject>>();

ChaiObject::ChaiObject(const std::string& name, const std::string& _namespace) : _namespace(_namespace), name(name) {
}

ChaiObject::~ChaiObject() {

}

std::string ChaiObject::getNamespace() const noexcept {
  return _namespace;
}

std::string ChaiObject::getName() const noexcept {
  return name;
}

std::string ChaiObject::toString() const noexcept {
  return _namespace + "::" + name;
}

bool ChaiObject::operator<(const ChaiObject& other) const {
  return false;
}

bool ChaiObject::hasObjectBeenRegistered(const std::string& name, const std::string& _namespace) {
  return std::find_if(std::begin(object_registry), std::end(object_registry), [&](std::weak_ptr<ChaiObject> obj) {
    return obj.lock()->getName() == name && obj.lock()->getNamespace() == _namespace;
  }) != object_registry.end();
}

std::string ChaiObject::getRegistryFunctionCall() const {
  std::stringstream str;
  str << "get" << getNamespace() << getName() << "Module()";
  return str.str();
}
