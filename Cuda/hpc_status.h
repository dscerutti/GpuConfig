// -*-c++-*-
#ifndef OMNI_HPC_STATUS
#define OMNI_HPC_STATUS

#include <vector>
#include "Constants/scaling.h"

namespace omni {
namespace cuda {

/// \brief Detect the amount of GPU memory occupied by a given process to determine whether it is
///        a significant process.  Occupying a megabyte of GPU RAM is one way for a process to
///        qualify as significant.
/// \{
constexpr long long int significant_gpu_memory = mega;
/// \}
  
/// \brief Unguarded struct describing pertinent aspects of one particular GPU.  Condensing the
///        data for each GPU in this manner helps to ensure that one cache line will obtain all
///        statistics for a single GPU 
struct GpuDetails {
  bool available;            ///< Flag to indicate whether a GPU is available for the program's use
  bool supported;            ///< Flag to indicate whether OMNI supports this GPU
  int arch_major;            ///< Major architecture numbers for each GPU
  int arch_minor;            ///< Minor architecture numbers for each GPU
  int smp_count;             ///< Number of streaming multiprocessors in each GPU
  int card_ram;              ///< The amount of RAM available on each GPU
  int max_threads_per_block; ///< The maximum number of threads per thread block
  int max_threads_per_smp;   ///< Number of threads one streaming multiprocessor (SMP) can handle
  int max_blocks_per_smp;    ///< Maximum number of blocks permissible on one SMP
  int max_shared_per_block;  ///< Maximum shared memory available per block (bytes)
  int max_shared_per_smp;    ///< Maximum shared memory available per SMP (bytes)
  int registers_per_smp;     ///< Size of the register file on each SMP
  std::string card_name;     ///< Name of the card according to the server
};

struct HpcStatus {

  // Constructor will detect all available GPUs if an HPC language is compiled
  HpcStatus();
  
  // Getter member functions
  int getOverallGpuCount() const;
  int getAvailableGpuCount() const;
  int getSupportedGpuCount() const;
  const GpuDetails getGpuInfo(int gpu_index) const;

  // Define the array index operator to call the appropriate getter
  operator [] (...);
  
private:
  int overall_gpu_count;            ///< The physical number of GPUs detected in the server
  int available_gpu_count;          ///< The number of available GPUs
  int supported_gpu_count;          ///< The number of supported GPUs
  std::vector<GpuDetails> gpu_list; ///< Details an availability of each GPU in the system
};

} // namespace cuda
} // namespace omni
#endif
