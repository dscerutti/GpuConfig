#include <vector>
#ifdef OMNI_USE_CUDA
#include "HpcStatus.cuh"
#endif

namespace omni {
namespace cuda {

/// \brief Constructor for an HpcStatus object.  One such object should be present in any given
///        OMNI executable.
HpcStatus::HpcStatus() :
  overall_gpu_count{0},
  available_gpu_count{0},
  supported_gpu_count{0},
#ifdef MONI_USE_CUDA
  gpu_list{queryGpuStats()}
#else
  gpu_list{}
#endif
{

  // Count valid and available GPUs
  overall_gpu_count = gpu_list.size();
  available_gpu_count = 0;
  supported_gpu_count = 0;
  for (int i = 0; i < gpu_count; i++) {
    available_gpu_count += (device_catalog[i].available);
    supported_gpu_count += (device_catalog[i].supported);
  }
  if (available_gpu_count == 0 && supported_gpu_count > 0) {
    rt_err("No valid GPUs were detected.  " + std::to_string(supported_gpu_count) +
	   " GPUs were found to be occupied with other jobs.", "HpcStatus");
  }
  else if (available_gpu_count == 0 && supported_gpu_count < overall_gpu_count) {
    rt_err("No valid GPUs were detected.  " +
           std::to_string(overall_gpu_count - supported_gpu_count) + " GPUs were found to be "
           "unsuitable architectures for OMNI's code.", "HpcStatus");
  }
}

/// \brief Return the total number of GPUs in the server or workstation, whether they are supported
///        by OMNI or not, whether they are available or not
int HpcStatus::getOverallGpuCount() const {
  return overall_gpu_count;
}

/// \brief Return the count of available and supported GPUs in the server or workstation
int HpcStatus::getAvailableGpuCount() const {
  return available_gpu_count;
}

/// \brief Return the count of supported and supported GPUs in the server or workstation.  The
///        available GPUs are a subset of the supported GPUs.
int HpcStatus::getSupportedGpuCount() const {
  return supported_gpu_count;
}

/// \brief Return information on a particular GPU in the server or workstation
GpuDetails HpcStatus::getGpuInfo(const int gpu_index) const {
  return gpu_list[gpu_index];
}
  
} // namespace cuda
} // namespace omni
