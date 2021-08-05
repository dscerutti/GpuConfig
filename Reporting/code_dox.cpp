#include <vector>
#include <string>
#include "Parsing/parse.h"
#include "Parsing/file_listing.h"
#include "DataTypes/vector_types.h"

namespace omni {
namespace docs {

using parse::TextFile;
using parse::osSeparator;
using parse::DrivePathType;
using parse::SearchStyle;
using ;

/// \brief Determine whether a line is a pre-processor directive, based on whether the line begins
///        with a hash (#) after any amount of whitespace.
bool lineIsPreProcessor(const char* line, const int nchar) {
  int i;
  while (i < nchar) {
    if (line[i] == '#') {
      return true;
    }
    else if (line[i] != ' ') {
      return false;
    }
    i++;
  }
  return false;
}

/// \brief Test a pre-processor line to determine whether it is part of a scope-modifying
///        if/then/else statement.
PreProcessorScopeModifier testScopeModifier(const char* line, const int nchar) {
  int i;
  while (i < nchar) {
    if (i < nchar - 1 && (line[i] == 'i' || line[i] == 'I') &&
        (line[i + 1] == 'f' || line[i + 1] == 'F')) {
      return PreProcessorScopeModifier::IF;
    }
    else if (i < nchar - 3 && (line[i] == 'e' || line[i] == 'E') &&
	     (line[i + 1] == 'l' || line[i + 1] == 'L') &&
	     (line[i + 2] == 'i' || line[i + 2] == 'I') &&
	     (line[i + 3] == 'f' || line[i + 3] == 'F')) {
      return PreProcessorScopeModifier::ELIF;
    }
    else if (i < nchar - 3 && (line[i] == 'e' || line[i] == 'E') &&
	     (line[i + 1] == 'l' || line[i + 1] == 'L') &&
	     (line[i + 2] == 's' || line[i + 2] == 'S') &&
	     (line[i + 3] == 'e' || line[i + 3] == 'E')) {
      return PreProcessorScopeModifier::ELSE;
    }
    else if (i < nchar - 4 && (line[i] == 'e' || line[i] == 'E') &&
	     (line[i + 1] == 'n' || line[i + 1] == 'N') &&
	     (line[i + 2] == 'd' || line[i + 2] == 'D') &&
	     (line[i + 3] == 'i' || line[i + 3] == 'I') &&
	     (line[i + 4] == 'f' || line[i + 4] == 'F')) {
      return PreProcessorScopeModifier::ENDIF;
    }
    i++;
  }
  return PreProcessorScopeModifier::NONE;
}

/// \brief Find all #if / #elif / #else / #endif pre-processor scopes within a given file.
///
/// \param tf  Text of the file, read into RAM
std::vector<int3> findPreProcessorScopes(const TextFile::Reader &tfr) {

  // Make a list of lines with pre-processor directives
  std::vector<int> pp_lines;
  for (int i = 0; i < tfr.line_count; i++) {
    const int nchar = tfr.line_limits[i + 1] - tfr.line_limits[i];
    if (lineIsPreProcessor(&tfr.text[tfr.line_limits[i]], nchar)) {
      pp_lines.push_back(i);
    }
  }
  const int n_pp_line = pp_lines.size();
  std::vector<PreProccessorScopeModifier> sc_type(n_pp_line, PreProcessorScopeModifier::NONE);
  for (int i = 0; i < n_pp_line; i++) {
    const int nchar = tfr.line_limits[pp_lines[i] + 1] - tfr.line_limits[pp_lines[i]];
    sc_type[i] = testScopeModifier(&tfr.text[tfr.line_limits[pp_lines[i]]], nchar);
  }

  // Sort the lines with pre-processor directives into a list of scopes
  std::vector<int3> scopes;
  bool do_search = (n_pp_line > 0);
  int search_depth = 1;
  while (do_search) {
    int current_depth = 0;
    const int n_found = scopes.size();
    int3 tsm;
    for (int i = 0; i < n_pp_line; i++) {
      if (sc_type[i] == PreProcessorScopeModifier::IF) {
	current_depth++;
	if (current_depth == search_depth) {
	  tsm.x = search_depth;
	  tsm.y = i + 1;
	}
      }
      else if ((sc_type[i] == PreProcessorScopeModifier::ELIF ||
		sc_type[i] == PreProcessorScopeModifier::ELSE) && current_depth == search_depth) {
	tsm.z = i;
	scopes.push_back(tsm);
	n_found++;
	tsm.x = search_depth;
	tsm.y = i + 1;
      }
      else if (sc_type[i] == PreProcessorScopeModifier::ENDIF) {
        if (current_depth == search_depth) {
          tsm.z = i;
          scopes.push_back(tsm);
        }
        current_depth--;
      }
    }
    do_search = (scopes.size() > n_found);
  }

  // Return the result as a vector of int3 objects, x = level, y = first line of scope, and
  // z = last line of scope.  The list will have to be searched exhaustively to determine what
  // scope any particular line of code belongs to.  Some scopes will be 'linked' in that only
  // one of a chain of scopes can be part of the code at once.  This will be detectable by
  // the first line of one scope being only one more than the last line of the previous scope.
  return scopes;
}
  
/// \brief Find all C++ scopes within { } braces in a given file.
std::vector<CppScope> findCppScopes(const TextFile::Reader &tfr) {
  bool in_starred_comment = false;
  for (int i = 0; i < tfr.line_count; i++) {
    const int l_start = tfr.line_limits[i];
    const int l_end = tfr.line_limits[i + 1];
    int j = l_start;
    while (j < l_end) {
      if (in_starred_comment) {
        if (j < l_end - 1 && tfr.text[j] == '*' && tfr.text[j + 1] == '/') {
	  j += 2;
	  in_starred_comment = false;
	}
      }
      else {

        // Double slash comments out the rest of the line
        if (j < l_end - 1 && tfr.text[j] == '/' && tfr.text[j + 1] == '/') {
          j = l_end;
          continue;
        }
	
	if (j < l_end - 1 && tfr.text[j] == '/' && tfr.text[j + 1] == '*') {
	  j += 2;
	  in_starred_comment = true;
	}
      }
    }
  }
}
  
/// \brief Search a particular file for instances of a given object.
///
/// \param object_name  Name of the object (a function or struct) to search for
/// \param filename     File to search
ObjectIdentifier searchFileForObject(const std::string &object_name, const std::string &filename) {
  const TextFile tf(filename);
  const TextFile::Reader tfr = tf.data();
  for (int i = 0; i < 
}

/// \brief Search for an object in the entire OMNI source code tree.
///
/// \param object_name    Name of the object (a function or struct) to search for
/// \param member_name    Name of a struct's member variable or a function's argument
/// \param report_format  Format of the report to write
void searchObject(const std::string &object_name, const std::string &member_name,
                  const ObjectReportType report_format) {

  // Get the OMNI home directory
  const std::string omni_home = std::getenv("OMNI_HOME");

  // Get a list of directories in OMNI's src/ directory, then make lists of .cpp and .h files
  const std::vector<std::string> src_dirs = listDirectory(omni_home + osSeparator() + "src",
                                                          SearchStyle::RECURSIVE,
                                                          DrivePathType::DIRECTORY);
  std::vector<std::string> omni_cpp;
  std::vector<std::string> omni_hdr;
  const int n_src_dir = src_dirs.size();
  for (int i = 0; i < n_src_dir; i++) {
    std::vector<std::string> tmp_cpp = listFilesInPath(src_dirs[i] + osSeparator() + ".*.cpp");
    omni_cpp.insert(omni_cpp.end(), tmp_cpp.begin(), tmp_cpp.end());
    std::vector<std::string> tmp_hdr = listFilesInPath(src_dirs[i] + osSeparator() + ".*.h");
    omni_hdr.insert(omni_hdr.end(), tmp_hdr.begin(), tmp_hdr.end());
  }

  // Go through all files, looking for the object.
  const int n_cpp = omni_cpp.size();
  const int n_hdr = omni_hdr.size();
  std::vector<ObjectIdentifier> obj_docs;
  for (int i = 0; i < n_cpp; i++) {
    ObjectIdentifier tmp_id = searchFileForObject(object_name, omni_cpp[i]);
    obj_docs.push_back(tmp_id);
  }
}

} // namespace docs
} // namespace omni
