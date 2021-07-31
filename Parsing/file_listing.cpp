#include <dirent.h>
#include <sys/stat.h>
#include "file_listing.h"
//#include "Reporting/error-format.h"

// CHECK
#include <stdlib.h>
// END CHECK

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
    return "\\";
  }
  __builtin_unreachable();
}

/// \brief Test whether a path is a file, directory, or must instead be a regular expression.
DrivePathType getDrivePathType(const std::string &path) {

  // Try getting the status.  If this fails, the path is probably a regular expression.
  struct stat path_stat;
  if (stat(path.c_str(), &path_stat) == 0) {
    if (S_ISREG(path_stat.st_mode) == 1) {
      return DrivePathType::FILE;
    }
    else if (S_ISDIR(path_stat.st_mode) == 1) {
      return DrivePathType::DIRECTORY;
    }
  }
  
  // In order to confirm that the path is a regular expression, look for * or []
  bool star_found = false;
  bool left_bracket_found = false;
  bool pair_bracket_found = false;
  for (int i = 0; i < path.size(); i++) {
    star_found = (star_found || path[i] == '*');
    left_bracket_found = (left_bracket_found || path[i] == '[');
    pair_bracket_found = (pair_bracket_found || (left_bracket_found && path[i] == ']'));
  }
  if (star_found || pair_bracket_found) {
    return DrivePathType::REGEXP;
  }

  // Throw an error
#if 0
  rt_err("Path " + path + " is not a file or directory, and could not be interpreted as a regular "
	 "expression.", "getDrivePathType");
#endif
  __builtin_unreachable();
}
  
std::vector<std::string> parseRegExp(const std::string &regexp_path, SearchStyle r_option) {
  std::vector<std::string> ls_result;
  return ls_result;
}
  
std::vector<std::string> listDirectory(const std::string &dir_path, SearchStyle r_option) {
  
  // Attempt to open the directory
  std::vector<std::string> ls_result;
  DIR *dir;
  if ((dir = opendir(dir_path.c_str())) == NULL) {
    return ls_result;
  }
  struct dirent *ent;
  while ((ent = readdir(dir)) != NULL) {
    if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
      continue;
    }
    
    // Test whether the result at hand is, itself, a directory.
    struct stat ent_stat;
    std::string nested_dir_path = dir_path + osSeparator() + std::string(ent->d_name);    
    if (stat(nested_dir_path.c_str(), &ent_stat) == 0 && S_ISDIR(ent_stat.st_mode) == 1) {
      switch (r_option) {
      case SearchStyle::RECURSIVE:
	{      
	  std::vector<std::string> nested_result = listDirectory(nested_dir_path, r_option);
          ls_result.insert(ls_result.end(), nested_result.begin(), nested_result.end());
	  break;
        }
      case SearchStyle::NONRECURSIVE:
	break;
      }
    }
    else {
      std::string new_name(ent->d_name);
      ls_result.push_back(dir_path + osSeparator() + new_name);
    }
  }
  closedir(dir);

  return ls_result;
}

} // namespace parse
} // namespace omni

int main () {

  using omni::parse::listDirectory;
  using omni::parse::SearchStyle;
  
  std::vector<std::string> T1 = listDirectory("/Users/davidcerutti/amber20_src",
					      SearchStyle::NONRECURSIVE);
  printf("T1 = [\n");
  for (int i = 0; i < T1.size(); i++) {
    printf("%s\n", T1[i].c_str());
  }
  printf("];\n\n");
  std::vector<std::string> T2 = listDirectory("/Users/davidcerutti/amber20_src",
					      SearchStyle::RECURSIVE);
  printf("T2 = [\n");
  for (int i = 0; i < T2.size(); i++) {
    printf("%s\n", T2[i].c_str());
  }
  printf("];\n\n");

  return 0;
}
