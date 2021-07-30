#ifndef OMNI_SEARCH_DOX_H
#define OMNI_SEARCH_DOX_H

namespace omni {
namespace docs {

enum class OperatingSystem {
  LINUX, UNIX, WINDOWS, MAC_OS
};

/// Pre-processor directives determine the separators to use in path and file concatenation
/// \{
#if defined(__linux__)
constexpr OperatingSystem detected_os = LINUX;
#elif defined(unix) || defined(__unix) || defined(__unix__)
constexpr OperatingSystem detected_os = UNIX;
#elif defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
constexpr OperatingSystem detected_os = WINDOWS;
#elif defined(__APPLE__) || defined(__MACH__)
constexpr OperatingSystem detected_os = MAC_OS;
#endif
/// \}

void search_struct(std::string struct_name, std::string member_name);

} // namespace docs
} // namespace omni

#endif
