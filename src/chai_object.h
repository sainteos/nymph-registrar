#ifndef CHAI_OBJECT_H
#define CHAI_OBJECT_H
#include <string>
#include <vector>
#include <memory>

class ChaiObject : public std::enable_shared_from_this<ChaiObject> {
private:
  std::string _namespace;
  std::string name;
protected:
    static std::vector<std::weak_ptr<ChaiObject>> object_registry;
    ChaiObject(const std::string& name, const std::string& _namespace = "");
public:
  ChaiObject() = delete;
  virtual ~ChaiObject();

  std::string getNamespace() const noexcept;
  std::string getName() const noexcept;

  virtual std::string toString() const noexcept;
  virtual bool operator<(ChaiObject& other) const;

  static bool hasObjectBeenRegistered(const std::string& name, const std::string& _namespace);
  static std::shared_ptr<ChaiObject> getRegisteredObject(const std::string& name, const std::string& _namespace);

  template<class T>
  static std::shared_ptr<T> create(const std::string& name, const std::string& _namespace);

  virtual std::string getRegistryString() const = 0;
  virtual std::string getRegistryFunctionCall() const;
};

template<class T>
std::shared_ptr<T> ChaiObject::create(const std::string& name, const std::string& _namespace) {
  auto obj = std::make_shared<T>(name, _namespace);
  object_registry.push_back(obj);
  return obj;
}

#endif
