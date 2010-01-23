#include "splitString.h"

std::vector<std::string> split(const std::string & str,
                                      const std::string & delims=" *-")
{
  string::size_type lastPos = str.find_first_not_of(delims, 0);
  string::size_type pos = str.find_first_of(delims, lastPos);
  vector<string> tokens;
 
  while (string::npos != pos || string::npos != lastPos)
    {
      tokens.push_back(str.substr(lastPos, pos - lastPos));
      lastPos = str.find_first_not_of(delims, pos);
      pos     = str.find_first_of(delims, lastPos);
    }
 
  return tokens;
}

std::vector<std::string> split(const char* lhs, const char* rhs){
  const std::string m1 (lhs), m2 (rhs);
  return split(m1, m2);
}

std::vector<std::string> split(const char* lhs, const std::string& rhs){
  return split(lhs, rhs.c_str());
}

std::vector<std::string> split(const std::string& lhs, const char* rhs){
  return split(lhs.c_str(), rhs);
}
