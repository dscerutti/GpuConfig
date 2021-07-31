// -*-c++-*-
#ifndef OMNI_FILE_LISTING_H
#define OMNI_FILE_LISTING_H

#include <vector>
#include <string>

namespace omni {
namespace parse {

/// \brief Enumerate the possible types of directory search (this is equivalent to whether there is
///        a -r modifier after ls).
enum class SearchStyle {
  RECURSIVE, NONRECURSIVE
};

/// \brief Enumerate whether a path can be determined to be a file, directory, or, barring any
///        other explanation, a regular expression.
enum class DrivePathType {
  FILE, DIRECTORY, REGEXP
};

/// \brief An enumerator to make note of the operating systems
enum class OperatingSystem {
  LINUX, UNIX, WINDOWS, MAC_OS
};

/// Pre-processor directives determine the separators to use in path and file concatenation
/// \{
#if defined(__linux__)
constexpr OperatingSystem detected_os = OperatingSystem::LINUX;
#elif defined(unix) || defined(__unix) || defined(__unix__)
constexpr OperatingSystem detected_os = OperatingSystem::UNIX;
#elif defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
constexpr OperatingSystem detected_os = OperatingSystem::WINDOWS;
#elif defined(__APPLE__) || defined(__MACH__)
constexpr OperatingSystem detected_os = OperatingSystem::MAC_OS;
#endif
/// \}

std::string osSeparator();

DrivePathType getDrivePathType(const std::string &path);

std::vector<std::string> parseRegExp(const std::string &regexp_path, SearchStyle r_option);
  
std::vector<std::string> listDirectory(const std::string &path,
				       SearchStyle r_option = SearchStyle::NONRECURSIVE);

} // namespace parse
} // namespace omni

#endif
