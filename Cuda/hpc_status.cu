// -*-c++-*-
#include <cuda.h>
#include <stdio.h>
#include <stdlib.h>
#include <nvml.h>
#include <curand_kernel.h>
#include <vector>

#include "Reporting/error_format.h"
#include "hpc_status.h"

namespace omni {
namespace cuda {

extern std::vector<GpuDetails> queryGpuStats() {

  // Test that there is a GPU in the system.  Initialize
  // a vector of specs to store all detected GPUs.
  int n_gpus;
  if (cudaGetDeviceCount(&n_gpus) != cudaSuccess) {
    if (n_gpus == 0) {
      rt_err("Error.  No CUDA-capable devices were found.", "queryGpuStats");
    }
  }
  std::vector<GpuDetails> device_catalog;
  device_catalog.resize(n_gpus);
  
  // Activate zero-copy
  if (cudaSetDeviceFlags(cudaDeviceMapHost) != cudaSuccess) {
    rt_err("Unable to establish cudaDeviceMapHost with cudaSetDeviceFlags().", "queryGpuStats");
  }

  // Initialize the NVIDIA Management Library
  nvmlReturn_t nvml_init_result = nvmlInit_v2();
  if (nvml_init_result == NVML_ERROR_DRIVER_NOT_LOADED) {
    rt_err("nvmlInit_v2() failed with NVML_ERROR_DRIVER_NOT_LOADED.", "queryGpuStats");
  }
  else if (nvml_init_result == NVML_ERROR_NO_PERMISSION) {
    rt_err("nvmlInit_v2() failed with NVML_ERROR_NO_PERMISSION.", "queryGpuStats");
  }
  else if (nvml_init_result == NVML_ERROR_UNKNOWN) {
    rt_err("nvmlInit_v2() failed with NVML_ERROR_UNKNOWN.", "queryGpuStats");
  }

  // Get device properties
  for (int i = 0; i < n_gpus; i++) {
    std::vector<nvmlProcessInfo_t> nvml_info(32);
    nvmlDevice_t nt_device;
    cudaDeviceProp device_properties;
    if (cudaGetDeviceProperties(&device_properties, i) != cudaSuccess) {
      printf("queryGpuStats :: Warning.  Unable to query properties for GPU %d\n", i);
      continue;
    }

    // Transcribe information about this GPU
    device_catalog[i].arch_major            = device_properties.major;
    device_catalog[i].arch_minor            = device_properties.minor;
    device_catalog[i].smp_count             = device_properties.multiProcessorCount;
    device_catalog[i].card_ram              = device_properties.totalGlobalMem;
    device_catalog[i].max_threads_per_block = device_properties.maxThreadsPerBlock;
    device_catalog[i].max_threads_per_smp   = device_properties.maxThreadsPerMultiProcessor;
    device_catalog[i].max_blocks_per_smp    = ;
    device_catalog[i].max_shared_per_block  = ;
    device_catalog[i].max_shared_per_smp    = ;
    device_catalog[i].registers_per_smp     = ;
    device_catalog[i].card_name             = std::string(device_properties.name);

    // Determine whether the GPU is available
    if (device_properties.major < 3) {
      device_catalog[i].available = false;
      device_catalog[i].supported = false;
    }
    else if (device_properties.major >= 3) {
      device_catalog[i].supported = true;
      if (nvmlDeviceGetHandleByIndex_v2(i, &nt_device) != NVML_SUCCESS) {
        printf("queryGpuStats :: Warning.  Unable to get device handle for GPU %d\n", i);
        continue;
      }
      unsigned int nvml_item_count = 0;
      nvmlReturn_t nv_status = nvmlDeviceGetComputeRunningProcesses(nt_device, &nvml_item_count,
								    nvml_info.data());
      if (nv_status != NVML_SUCCESS && nv_status != NVML_ERROR_INSUFFICIENT_SIZE) {
        printf("queryGpuStats :: Warning.  Unable to monitor activity on GPU %d [error %u]\n", i,
	       nv_status);
      }
      unsigned long long int mem_occ = 0;
      for (int j = 0; j < nvml_item_count; j++) {
        mem_occ += nvml_info[j].usedGpuMemory;
      }
      device_catalog[i].available = (mem_occ < significant_gpu_activity);
    }
  }

  // Shut down the NVIDIA Management Library
  if (nvmlShutdown() != NVML_SUCCESS) {
    rt_err("Error executing nvmlShutdown().", "queryGpuStats");
  }

  return device_catalog;
}

/// \brief Return the indices, specs, and one or more
void HpcStatus::getGpuDevice(int requested_count) {

  // Make a list of supported and available GPUs by their device indices
  std::vector<int> selections;
  for (int i = 0; i < gpu_count; i++) {
    if (gpu_list[i].available) {
      selections.push_back[i];
    }
  }
  
  // Select a device from the list
  if (cudaSetValidDevices(selections.data(), ndev) != cudaSuccess) {
    cudaDeviceReset();
    rt_err("Error searching for compatible GPU.", "getGpuDevice");
  }

  // Establish the CUDA context
  if (cudaFree(0) != cudaSuccess) {
    cudaDeviceReset();
    rt_err("Error initializing the CUDA context with cudaFree(0).", "getGpuDevice");
  }

  // Get the device (this is a sanity check to ensure that the device can still be seen)
  int selected_device;
  if (cudaGetDevice(&selected_device) != cudaSuccess) {
    cudaDeviceReset();
    rt_err("Error selecting GPU.", "getGpuDevice");
  }

  // Set the device so that it will be used in all future calculations launched by this thread
  if (cudaSetDevice(&selected_device) != cudaSuccess) {
    cudaDeviceReset();
    rt_err("Error setting GPU.", "getGpuDevice");
  }  
}
  
} // namespace cuda
} // namespace omni
