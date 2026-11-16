#pragma once

#include <map>
#include <string>

class ExpectedOutputRegistry
{
public:
  static void set(const std::string& id, const std::string& data)
  {
    registryInstance()[id] = data;
  }

  static std::string get(const std::string& id)
  {
    return registryInstance().at(id);
  }

private:
  typedef std::map<std::string, std::string> ImplType;

  static ImplType& registryInstance()
  {
    static ImplType m;
    return m;
  }

  ExpectedOutputRegistry();
  static const ExpectedOutputRegistry initializer;
};


const ExpectedOutputRegistry ExpectedOutputRegistry::initializer;
