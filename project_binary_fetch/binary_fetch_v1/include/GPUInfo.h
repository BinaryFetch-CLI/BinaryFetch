#ifndef GPUINFO_H
#define GPUINFO_H

#include <string>
#include <vector>

// ----------------------------------------------------
// GPU data structure
struct gpu_data
{
    std::string gpu_name;
    std::string gpu_memory;
    std::string gpu_driver_version;
    std::string gpu_vendor;
    float gpu_usage;
    float gpu_temperature;
    int gpu_core_count;
    float gpu_frequency;  // Add this line (in MHz)
};


// ----------------------------------------------------
// GPUInfo class
class GPUInfo
{
public:
    // Get all GPU information
    static std::vector<gpu_data> get_all_gpu_info();

    // Get GPU usage percentage
    static float get_gpu_usage();

    // Get GPU temperature in Celsius
    static float get_gpu_temperature();

    // Get GPU core count (estimated)
    static int get_gpu_core_count();

	
};

#endif // GPUINFO_H