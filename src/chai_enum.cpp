#include <sstream>
#include "chai_enum.h"

ChaiEnum::ChaiEnum(const std::string& name, const std::string& _namespace) : ChaiObject(name, _namespace) {

}

void ChaiEnum::addValue(const std::string& value) {
  values.push_back(value);
}

std::vector<std::string> ChaiEnum::getValues() const noexcept {
  return values;
}

std::string ChaiEnum::getRegistryString() const {
  std::stringstream reg;
  reg << "chaiscript::ModulePtr " << getRegistryFunctionCall() << " {\n";
  reg << "  chaiscript::ModulePtr module = std::make_shared<chaiscript::Module>();\n";
  reg << "  std::vector<std::pair<unsigned int, std::string>> vec = {\n";

  for(auto& val : values) {
    reg << "    { " << (getNamespace() != "" ? (getNamespace() + "::") : "" ) << getName() << ", \"" << getName() << "_" << val << "\" },\n";
  }
  reg.seekp(static_cast<long>(reg.tellp()) - 2);
  reg << "\n";
  reg << "  };\n";

  reg << "  chaiscript::utility::add_class<" << (getNamespace() != "" ? (getNamespace() + "::") : "" ) << getName() << ">(*module, std::string(\"" << getName() << "\"),\n";
  reg << "  vec);\n";
  reg << "  return module;\n";
  reg << "}\n";
  return reg.str();
}