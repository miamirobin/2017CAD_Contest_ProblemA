/****************************************************************************
  FileName     [ myString.h ]
  PackageName  [ util ]
  Synopsis     [ Functions about strings ]
  Author       [ music960633 ]
****************************************************************************/

#ifndef UTIL_MYSTRING_H_
#define UTIL_MYSTRING_H_

#include <vector>
#include <string>

std::vector<std::string> mySplit(const std::string& str) {
  std::vector<std::string> ret;
  std::string tmp = "";
  for (unsigned i = 0, n = str.size(); i < n; ++i) {
    if (str[i] == ' ' || str[i] == '\t') {
      if (tmp.size() > 0)
        ret.push_back(tmp);
      tmp = "";
    } else {
      tmp.push_back(str[i]);
    }
  }
  if (tmp.size() > 0)
    ret.push_back(tmp);
  return ret;
}

#endif  // UTIL_MYSTRING_H_
