#ifndef CHAI_FUNCTION_H
#define CHAI_FUNCTION_H
#include <string>
#include <vector>
#include <map>
#include "chai_object.h"

class ChaiModule;

class ChaiFunction : public ChaiObject {
private:
  std::string return_type;
  std::weak_ptr<ChaiModule> module;
  std::multimap<std::string, std::string> arguments;
  bool is_overloaded;
  bool is_static;
public:
  ChaiFunction() = delete;
  ChaiFunction(const std::string& name, const std::string& _namespace = "", const std::string& return_type = "void", const bool is_overloaded = false, const bool is_static = false);

  void setReturnType(const std::string& return_type) noexcept;
  std::string getReturnType() const noexcept;
  void setOverloaded(const bool overloaded);
  bool isOverloaded() const noexcept;
  void setStatic(const bool is_static);
  bool isStatic() const noexcept;

  void addArgument(const std::string& type, const std::string& arg);
  std::multimap<std::string, std::string> getArguments() const noexcept;

  void isMethodOf(std::weak_ptr<ChaiModule> module);

  virtual std::string toString() const noexcept override;
  virtual std::string getRegistryString() const override;
};

#endif
