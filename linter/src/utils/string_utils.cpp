#include "utils/string_utils.h"

#include <Surelog/Common/FileSystem.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>

#include <string>

auto Trim(const std::string& str) -> std::string {
  auto start = str.find_first_not_of(" \t\n\r");
  if (start == std::string::npos) {
    return "";
  }
  auto end = str.find_last_not_of(" \t\n\r");
  return str.substr(start, end - start + 1);
}