// -*-c++-*-
#ifndef OMNI_FILE_LISTING_H
#define OMNI_FILE_LISTING_H

namespace omni {
namespace parse {

enum class SearchStyle {
  RECURSIVE, NONRECURSIVE
};

std::string osSeparator();
  
std::vector<std::string> listDirectory(const std::string &path,
				       SearchStyle r_option = SearchStyle::NONRECURSIVE);

} // namespace parse
} // namespace omni

#endif
