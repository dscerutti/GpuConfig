#ifndef OMNI_SEARCH_DOX_H
#define OMNI_SEARCH_DOX_H

namespace omni {
namespace docs {

/// \brief 
  
/// \brief Stores data on instances of an object (i.e. a function or struct found in a given file.
struct ObjectIdentifier {
  std::string filename;
  std::string return_type;
  int n_instances;
};

/// \brief Enumerator to describe how much information to deliver about an object.
enum class ObjectReportType {
  FULL,             ///< Print both annotation and usage statistics
  ANNOTATION_ONLY,  ///< Print annotation on members or function arguments
  STATS_ONLY        ///< Print usage statistics on the instances of this object across all files
};

/// \brief Enumerator for pre-processor lines of interest
enum class PreProcessorScopeModifier {
  NONE,  ///< The pre-processor line does not modify the scope, i.e. a #define statement
  IF,    ///< The line is a pre-processor if statement
  ELIF,  ///< The line is a pre-processor elif (else-if) statement
  ELSE,  ///< The line is a pre-processor else statement
  ENDIF, ///< The line is a pre-processor endif statement
};
  
PreProcessorScopeModifier testScopeModifier(const char* line, int nchar);

bool lineIsPreProcessor(const char* line, int nchar);

std::vector<int3> findPreProcessorScopes(const TextFile &tf);

ObjectIdentifier searchFileForObject(const std::string &object_name, const std::string &filename);

void searchFileForObject(const std::string &object_name,
                         const std::string &member_name = std::string,
                         ObjectReportType report_format = ObjectReportType::FULL);

} // namespace docs
} // namespace omni

#endif
