#include <string>
#include <dirent.h>
#include "Reporting/error-format.h"

namespace omni {
namespace parse {

/// \brief Produce the correct file path separator.
std::string osSeparator() {
  switch (detected_os) {
  case (OperatingSystem::LINUX):
  case (OperatingSystem::UNIX):
  case (OperatingSystem::MAC_OS):
    return "/";
  case (OperatingSystem::WINDOWS):
    return "\";                                                                                     
  }
  __builtin_unreachable();
}

std::vector<std::string> listDirectory(const std::string &path, SearchStyle r_option) {

  // Verify that the path is a directory
  struct stat path_stat;
  if (stat(path.c_str(), &path_stat) != 0 || S_ISDIR(path_stat.st_mode) != 1) {
    rt_err("Path " + path + " is not a directory.", "listDirectory");
  }

  // Search the directory
  
}

} // namespace parse
} // namespace omni
