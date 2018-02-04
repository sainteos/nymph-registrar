#ifndef CHAI_MODULE_H
#define CHAI_MODULE_H
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <set>
#include "chai_object.h"
#include "chai_function.h"

class ChaiModule : public ChaiObject {
private:
  std::vector<std::shared_ptr<ChaiFunction>> functions;
  std::vector<std::shared_ptr<ChaiFunction>> constructors;
  std::vector<std::string> raw_bases;

  std::map<std::string, std::shared_ptr<ChaiFunction>> function_names;

public:
  ChaiModule() = delete;
  ChaiModule(const std::string& _class, const std::string& _namespace);

  void addFunction(std::shared_ptr<ChaiFunction> function);
  std::vector<std::shared_ptr<ChaiFunction>>& getFunctions() noexcept;
  void addConstructor(std::shared_ptr<ChaiFunction> constructor);
  std::vector<std::shared_ptr<ChaiFunction>>& getConstructors() noexcept;

  void addRawBase(const std::string& name);
  void addRawBases(std::vector<std::string> bases);
  std::vector<std::string> getRawBases() const noexcept;

  std::set<std::shared_ptr<ChaiModule>> getAllBases() const;

  virtual bool operator<(const ChaiObject& other) const override;

  virtual std::string getRegistryString() const override;
};

#endif
