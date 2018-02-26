#ifndef CHAI_ENUM_H
#define CHAI_ENUM_H
#include <string>
#include <vector>
#include "chai_object.h"

class ChaiEnum : public ChaiObject {
private:
  std::vector<std::string> values;
  std::string in_class;
public:
  ChaiEnum() = delete;
  ChaiEnum(const std::string& name, const std::string& _namespace);

  void addValue(const std::string& value);
  std::vector<std::string> getValues() const noexcept;

  void inClass(const std::string& class_name);
  std::string getClassWithin() const noexcept;

  virtual std::string getRegistryString() override;
};

#endif
