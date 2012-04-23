#include <resourceTools.h>
#include <iostream>

std::string getResourceAsString(const std::string &name) {
  std::map<std::string, const char*>::iterator it = resourceMap.find(name);
  if (it != resourceMap.end()) {
    return std::string(resourceMap[name]);
  }
  return "";
}
