#ifndef CHAI_FUNCTION_TEMPLATE_H
#define CHAI_FUNCTION_TEMPLATE_H
#include <vector>
#include "chai_function.h"

enum class SubstitutionLocation {
  BEFORE, AFTER, MUTATOR
};

class ChaiFunctionTemplate : public ChaiFunction {
private:
  SubstitutionLocation sub_location;
  std::vector<std::pair<std::string, std::vector<std::string>>> template_names_to_types;
public:
  ChaiFunctionTemplate() = delete;
  ChaiFunctionTemplate(const std::string& name, const std::string& _namespace = "", const FunctionType& type = FunctionType::GLOBAL_TEMPLATE, const std::string& return_type = "void", const bool is_static = false);
  ChaiFunctionTemplate(ChaiFunctionTemplate&& func) = default;

  void addTemplateVariable(const std::string& name) noexcept;
  std::vector<std::string> getTemplateVariables() const noexcept;
  unsigned int getNumTemplateVariables() const noexcept;
  void addSubstitutionType(const std::string& name, const std::string& type) noexcept;
  void addSubstitutionTypes(const std::string& name, const std::vector<std::string>& types) noexcept;
  std::vector<std::string> getSubstitutionTypes(const std::string& name) const;
  void setSubstitutionLocation(const SubstitutionLocation& s) noexcept;
  SubstitutionLocation getSubstitutionLocation() const noexcept;

  virtual std::string toString() const noexcept override;
  virtual std::string getRegistryString() override;
};

#endif
