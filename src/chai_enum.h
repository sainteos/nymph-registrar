#ifndef CHAI_ENUM_H
#define CHAI_ENUM_H
#include <string>
#include <vector>
#include "chai_object.h"

class ChaiEnum : public ChaiObject {
private:
  std::vector<std::string> values;
public:
  ChaiEnum() = delete;
  ChaiEnum(const std::string& name, const std::string& _namespace);

  void addValue(const std::string& value);
  std::vector<std::string> getValues() const noexcept;

  virtual std::string getRegistryString() const override;
};

#endif
