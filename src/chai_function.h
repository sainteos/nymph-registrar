#ifndef CHAI_FUNCTION_H
#define CHAI_FUNCTION_H
#include <string>
#include <vector>
#include <map>
#include "chai_object.h"

class ChaiModule;

enum class FunctionType { GLOBAL, GLOBAL_TEMPLATE, MEMBER, MEMBER_TEMPLATE };

class ChaiFunction : public ChaiObject {
protected:
  FunctionType type;
  std::string return_type;
  std::weak_ptr<ChaiModule> module;
  std::vector<std::pair<std::string, std::string>> arguments;
  bool is_constructor;
  bool is_overloaded;
  bool is_static;
public:
  ChaiFunction() = delete;
  ChaiFunction(const std::string& name, const std::string& _namespace = "", const FunctionType& type = FunctionType::GLOBAL, const std::string& return_type = "void", const bool is_overloaded = false, const bool is_static = false);
  ChaiFunction(ChaiFunction&& func) = default;
  void setReturnType(const std::string& return_type) noexcept;
  std::string getReturnType() const noexcept;
  void setConstructor(const bool constructor);
  bool isConstructor() const noexcept;
  void setOverloaded(const bool overloaded);
  bool isOverloaded() const noexcept;
  void setStatic(const bool is_static);
  bool isStatic() const noexcept;

  FunctionType getFunctionType() const noexcept;

  void addArgument(const std::string& type, const std::string& arg);
  std::vector<std::pair<std::string, std::string>> getArguments() const noexcept;

  void isMethodOf(std::weak_ptr<ChaiModule> module);

  virtual std::string toString() const noexcept override;
  virtual std::string getRegistryString() override;
};

#endif
