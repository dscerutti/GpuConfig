#include <vector>
#include <string>
#include "Parsing/parse.h"

namespace omni {
namespace docs {

void search_struct(std::string struct_name, std::string member_name) {

  // Get the OMNI home directory
  std::string omni_home = std::getenv("OMNI_HOME");

  // Fold in the source directories
  std::string omni_src = omni_home + os_separator() + "src" + os_separator();

  // Search for additional directories in ${OMNI_HOME}/src/.  Make an array of code file names.
  
}

} // namespace docs
} // namespace omni
