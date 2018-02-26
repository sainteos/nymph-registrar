#ifndef CHAI_MODULE_H
#define CHAI_MODULE_H
#include <string>
#include <vector>
#include <deque>
#include <memory>
#include <map>
#include <list>
#include "chai_object.h"
#include "chai_function.h"

class ChaiModule : public ChaiObject {
private:
  std::deque<std::shared_ptr<ChaiFunction>> functions;
  std::deque<std::unique_ptr<ChaiFunction>> constructors;
  std::vector<std::string> raw_bases;

protected:
  std::vector<std::shared_ptr<ChaiModule>> getBases(const std::string& ns, const std::string& cs) const;

public:
  ChaiModule() = delete;
  ChaiModule(const std::string& _class, const std::string& _namespace);
  ChaiModule(ChaiModule&& m) = default;

  void addFunction(std::shared_ptr<ChaiFunction> function);
  bool hasFunction(const std::string& name) const;
  std::shared_ptr<ChaiFunction> getFunction(const std::string& name) const;
  void addConstructor(std::unique_ptr<ChaiFunction> constructor);

  void addRawBase(const std::string& name);
  void addRawBases(const std::vector<std::string>& bases);
  std::vector<std::string> getRawBases() const noexcept;

  std::list<std::shared_ptr<ChaiModule>> getAllBases() const;

  virtual bool operator<(ChaiObject& other) const override;

  virtual std::string getRegistryString() override;
};

#endif
