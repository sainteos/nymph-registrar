#include <sstream>
#include <iostream>
#include <cctype>
#include "chai_module.h"
#include "chai_function_template.h"

ChaiFunctionTemplate::ChaiFunctionTemplate(const std::string& name, const std::string& _namespace, const FunctionType& type, const std::string& return_type, const bool is_static)
  : ChaiFunction(name, _namespace, type, return_type, false, is_static), sub_location(SubstitutionLocation::BEFORE) {

}

void ChaiFunctionTemplate::addTemplateVariable(const std::string& name) noexcept {
  template_names_to_types.push_back(std::pair<std::string, std::vector<std::string>>(name, std::vector<std::string>()));
}

std::vector<std::string> ChaiFunctionTemplate::getTemplateVariables() const noexcept {
  std::vector<std::string> out;
  for(auto t : template_names_to_types) {
    out.push_back(t.first);
  }
  return out;
}

unsigned int ChaiFunctionTemplate::getNumTemplateVariables() const noexcept {
  return template_names_to_types.size();
}

void ChaiFunctionTemplate::setSubstitutionLocation(const SubstitutionLocation& s) noexcept {
  sub_location = s;
}

SubstitutionLocation ChaiFunctionTemplate::getSubstitutionLocation() const noexcept {
  return sub_location;
}

void ChaiFunctionTemplate::addSubstitutionType(const std::string& name, const std::string& type) noexcept {
  auto pred = [&](const std::pair<std::string, std::vector<std::string>>& p) {
    return name == p.first;
  };

  if(std::find_if(template_names_to_types.begin(), template_names_to_types.end(), pred) != template_names_to_types.end()) {
    std::find_if(template_names_to_types.begin(), template_names_to_types.end(), pred)->second.push_back(type);
  }
}

void ChaiFunctionTemplate::addSubstitutionTypes(const std::string& name, const std::vector<std::string>& types) noexcept {
  auto pred = [&](const std::pair<std::string, std::vector<std::string>>& p) {
    return name == p.first;
  };

  auto find_result = std::find_if(template_names_to_types.begin(), template_names_to_types.end(), pred);
  if(find_result != template_names_to_types.end()) {
    find_result->second.insert(find_result->second.cbegin(), find_result->second.cend(), types.cbegin());
  }
}

std::vector<std::string> ChaiFunctionTemplate::getSubstitutionTypes(const std::string& name) const {
  auto pred = [&](const std::pair<std::string, std::vector<std::string>>& p) {
    return name == p.first;
  };

  auto find_result = std::find_if(template_names_to_types.begin(), template_names_to_types.end(), pred);
  return find_result->second;
}

std::string ChaiFunctionTemplate::toString() const noexcept {
  std::stringstream str;
  str << "template<";
  for(auto s : template_names_to_types) {
    str << s.first << ", ";
  }

  str.seekp(static_cast<long>(str.tellp()) - 2);

  str << ">";
  str <<  (this->module.use_count() > 0 ? this->module.lock()->getName() + "::" : "") + getName();
  return str.str();
}

namespace detail {
  struct CombinationOutput {
    std::vector<std::string> chai_names;
    std::vector<std::vector<std::string>> template_args;
  };

  std::vector<std::string> combine_str(const std::vector<std::string> v1, const std::vector<std::string> v2) {
    std::vector<std::string> out;
    for(auto type1 : v1) {
      for(auto type2 : v2) {
        out.push_back(type1 + type2);
      }
    }
    return out;
  }

  std::vector<std::vector<std::string>> expand_to_vectors(const std::vector<std::string> strs) {
    std::vector<std::vector<std::string>> out;
    for(auto str : strs) {
      std::vector<std::string> expansion;
      expansion.push_back(str);
      out.push_back(expansion);
    }
    return out;
  }

  std::vector<std::vector<std::string>> combine_arg(const std::vector<std::string> v1, const std::vector<std::string> v2) {
    std::vector<std::vector<std::string>> out;

    for(auto type1 : v1) {
      for(auto type2 : v2) {
        std::vector<std::string> args;
        args.push_back(type1);
        args.push_back(type2);
        out.push_back(args);
      }
    }

    return out;
  }

  std::vector<std::vector<std::string>> combine_arg(const std::vector<std::vector<std::string>> v1, const std::vector<std::string> v2) {
    std::vector<std::vector<std::string>> start = v1;
    std::vector<std::vector<std::string>> out;
    for(auto type_list : start) {
      for(auto type : v2) {
        auto new_list = type_list;
        new_list.push_back(type);
        out.push_back(new_list);
      }
    }
    return out;
  }

  CombinationOutput combine_all(const std::vector<std::pair<std::string, std::vector<std::string>>> tt) {
    if(tt.size() == 0)
      return CombinationOutput{std::vector<std::string>(), std::vector<std::vector<std::string>>()};
    else if(tt.size() == 1)
      return CombinationOutput{tt.begin()->second, expand_to_vectors(tt.begin()->second)};
    else {
      std::vector<std::string> str_output;
      str_output = combine_str(tt[0].second, tt[1].second);
      for(int i = 2; i < tt.size(); i++) {
        str_output = combine_str(str_output, tt[i].second);
      }

      std::vector<std::vector<std::string>> arg_output;
      arg_output = combine_arg(tt[0].second, tt[1].second);
      for(int i = 2; i < tt.size(); i++) {
        arg_output = combine_arg(arg_output, tt[i].second);
      }

      return CombinationOutput{str_output, arg_output};
    }
  }
}

std::string ChaiFunctionTemplate::getRegistryString() {
  std::stringstream reg;

  auto possible_types = detail::combine_all(template_names_to_types);
  if(possible_types.chai_names.size() != possible_types.template_args.size()) {
    throw std::runtime_error("Mismatch of chai names to template args when trying to generate function template registry.");
  }

  int size = possible_types.chai_names.size();

  for(int i = 0; i < size; i ++) {
    reg << "{ chaiscript::fun(";
    reg << "&" << this->module.lock()->getName() << "::" << getName() << "<";
    for(auto arg : possible_types.template_args[i]) {
      reg << arg << ", ";
    }
    reg.seekp(static_cast<long>(reg.tellp()) - 2);
    reg << ">), \"";
    if(is_static) {
      reg << this->module.lock()->getName() << "_";
    }

    auto strip_scope = [&](const std::string& s) -> std::string {
      if(s.find_last_of("::") != std::string::npos) {
        return s.substr(s.find_last_of("::") + 1, s.size() - s.find_last_of("::") - 1);
      }
      else {
        return s;
      }
    };

    switch(sub_location) {
      case SubstitutionLocation::BEFORE: {
        auto cap_name = getName();
        cap_name[0] = std::toupper(cap_name[0]);

        auto template_type = strip_scope(possible_types.chai_names[i]);
        //For cases like unsigned int
        while(template_type.find(" ") != std::string::npos) {
          template_type[template_type.find(" ") + 1] = std::toupper(template_type[template_type.find(" ") + 1]);
          template_type.erase(template_type.find(" "), 1);
        }

        reg << template_type << cap_name;
        break;
      }
      case SubstitutionLocation::AFTER: {
        auto template_type = strip_scope(possible_types.chai_names[i]);
        template_type[0] = std::toupper(template_type[0]);
        //For cases like unsigned int
        while(template_type.find(" ") != std::string::npos) {
          template_type[template_type.find(" ") + 1] = std::toupper(template_type[template_type.find(" ") + 1]);
          template_type.erase(template_type.find(" "), 1);
        }
        reg << getName() << template_type;
        break;
      }
      case SubstitutionLocation::MUTATOR: {
        auto name = getName();
        auto cap_loc = std::find_if(name.begin(), name.end(), [&](const char s) { return s == std::toupper(s); });
        auto template_type = strip_scope(possible_types.chai_names[i]);
        template_type[0] = std::toupper(template_type[0]);
        //For cases like unsigned int
        while(template_type.find(" ") != std::string::npos) {
          template_type[template_type.find(" ") + 1] = std::toupper(template_type[template_type.find(" ") + 1]);
          template_type.erase(template_type.find(" "), 1);
        }
        if(cap_loc == name.end()) {
          reg << name << template_type;
        }
        else {
          name.insert(cap_loc, template_type.begin(), template_type.end());
          reg << name;
        }
        break;
      }
    }
    reg << "\" },\n";
  }
  return reg.str();
}
