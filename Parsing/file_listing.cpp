#include <dirent.h>
#include <sys/stat.h>
#include <regex>
#include "file_listing.h"
//#include "Reporting/error-format.h"

// CHECK
#include <stdlib.h>
// END CHECK

namespace omni {
namespace parse {

/// \brief Produce the correct file path separator.
char osSeparator() {
  switch (detected_os) {
  case (OperatingSystem::LINUX):
  case (OperatingSystem::UNIX):
  case (OperatingSystem::MAC_OS):
    return '/';
  case (OperatingSystem::WINDOWS):
    return '\\';
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

  // Interpret anything else as a regular expression
  return DrivePathType::REGEXP;
}

/// \brief Given a path that has been established to be a directory, list all files within it.
///        Recursively descend into subdirectories, if requested.
///
/// \param dir_path  The path to search
/// \param r_option  Option to use recursion or not
std::vector<std::string> listDirectory(const std::string &dir_path, const SearchStyle r_option,
				       const DrivePathType entity_kind) {
  
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
    DrivePathType item_type = getDrivePathType(nested_dir_path);
    if (item_type == DrivePathType::DIRECTORY) {
      switch (r_option) {
      case SearchStyle::RECURSIVE:
	{
	  std::vector<std::string> nested_result = listDirectory(nested_dir_path, r_option,
                                                                 entity_kind);
          ls_result.insert(ls_result.end(), nested_result.begin(), nested_result.end());
	  break;
        }
      case SearchStyle::NONRECURSIVE:
        if (entity_kind == DrivePathType::DIRECTORY) {
          ls_result.push_back(nested_dir_path);
        }
	break;
      }
    }
    else if (item_type == DrivePathType::FILE) {
      if (entity_kind == DrivePathType::FILE) {
        std::string new_name(ent->d_name);
        ls_result.push_back(dir_path + osSeparator() + new_name);
      }
    }
  }
  closedir(dir);

  return ls_result;
}

/// \brief Get the normative path by removing any trailing slashes.
///
/// \param path  The path to normalize
std::string getNormPath(const std::string &path) {
  const char sep_char = osSeparator();
  int i = path.size() - 1;
  while (i >= 0 && path[i] == sep_char) {
    i--;
  }
  return path.substr(0, i + 1);
}

/// \brief Get the base name of a path by taking everything after the last OS-specific separator.
///
/// \param path  The path from which to extract a base name
std::string getBaseName(const std::string &path) {
  const char sep_char = osSeparator();
  const int plength = path.size();
  int last_separator = 0;
  for (int i = 0; i < plength; i++) {
    if (path[i] == sep_char) {
      last_separator = i + 1;
    }
  }
  return path.substr(last_separator, plength - last_separator);
}

/// \brief Given a path that has been established to be a directory, list all files it could
///        describe, recursively descend into subdirectories if requested.  The recursion only
///        kicks in once the regular expression has been interpreted into a path that is, itself,
///        a directory.  Otherwise, "A/*/B/[a-z]*" will find all subdirectories of A/ such that
///        they contain their own subdirectory B/ and within each such B/ all files beginning with
///        a lowercase letter or, if recursion is activated, descend recursively into any and all
///        subdirectories of B/ whose name begins with a lowercase letter listing all files.
///        The search string "A/*/B/[a-z]*" would not find a file A/foo.txt, but it would find
///        A/bar/B/foo.txt, and if recursion were activated it would find A/bar/B/C/D/E/foo.txt.
///
/// \param regexp_path  The regular expression to evaluate
/// \param r_option     Option to use recursion or not
std::vector<std::string> listRegExp(const std::string &regexp_path, SearchStyle r_option) {
  std::vector<std::string> ls_result;

  // Detect a leading slash to see if the regular expression contains an absolute path
  const char sep_char = osSeparator();
  bool absolute_path = (regexp_path[0] == sep_char);
  
  // Break the regular expression into a series of directory levels
  std::vector<std::string> levels;
  int i = 0;
  const int rp_length = regexp_path.size();
  while (i < rp_length) {

    // Take in characters until the next separator
    std::string new_level;
    while (i < rp_length) {
      char t_char = regexp_path[i];
      i++;
      if (t_char != sep_char) {
        new_level += t_char;
      }
      else {
	break;
      }
    }
    if (new_level.size() > 0) {
      levels.push_back(new_level);
    }
  }

  // For each level, look at the appropriate directory and search for matching subdirectories
  const int n_levels = levels.size();  
  std::string partial_path = (absolute_path) ? "" : ".";
  for (i = 0; i < n_levels; i++) {
    std::string test_path = partial_path + sep_char + levels[i];
    DrivePathType pd = getDrivePathType(test_path);
    switch (pd) {
    case DrivePathType::FILE:
      if (i == n_levels - 1) {
	ls_result.push_back(test_path);
      }
      else {
#if 0
	rt_err("A file " + test_path + " was encountered while parsing regular expression " +
	       regexp_path + ", which should reach deeper into the directory tree.", "listRegExp");
#endif
	// CHECK
	// END CHECK
      }
      break;
    case DrivePathType::DIRECTORY:
      if (i == n_levels - 1) {
        std::vector<std::string> nested_result = listDirectory(test_path, r_option);
        ls_result.insert(ls_result.end(), nested_result.begin(), nested_result.end());
      }
      else {
	partial_path = test_path;
      }
      break;
    case DrivePathType::REGEXP:
      {
	std::regex level_expr(levels[i]);
        std::vector<std::string> test_paths = listDirectory(partial_path,
							    SearchStyle::NONRECURSIVE,
							    DrivePathType::FILE);
        std::vector<std::string> additional_paths = listDirectory(partial_path,
							          SearchStyle::NONRECURSIVE,
							          DrivePathType::DIRECTORY);
	test_paths.insert(test_paths.end(), additional_paths.begin(), additional_paths.end());
        const int n_paths = test_paths.size();
        for (int j = 0 ; j < n_paths; j++) {
	  
          // Test that the deepest level of this path conforms to the regular expression at hand.
          if (regex_match(getBaseName(getNormPath(test_paths[j])), level_expr)) {

            // This path is valid.  If there are more levels to explore, continue adding to the
            // partial path.  Otherwise, if the path is a file, add it to the list, or if it is
            // a directory, list its files, using recursion as requested.
            std::string refined_path = test_paths[j];
            for (int k = i + 1; k < n_levels; k++) {
              refined_path += sep_char + levels[k];
            }
            std::vector<std::string> nested_result = listRegExp(refined_path, r_option);
            ls_result.insert(ls_result.end(), nested_result.begin(), nested_result.end());
          }
	}
      }
      break;
    }
  }
  
  return ls_result;
}

} // namespace parse
} // namespace omni

int main () {

  using omni::parse::listDirectory;
  using omni::parse::SearchStyle;
#if 0  
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
#endif
  std::vector<std::string> T3 = listRegExp("/Users/davidcerutti/TestFiles/a_[abc].*",
					   SearchStyle::RECURSIVE);
  printf("T3 = [\n");
  for (int i = 0; i < T3.size(); i++) {
    printf("%s\n", T3[i].c_str());
  }
  printf("];\n\n");

  
  return 0;
}
