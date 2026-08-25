#pragma once
#include <string>
#include <vector>
using namespace std;

struct GPUData {
	int index;
	string name;
	float frequency_ghz;
	float vram_gb;
};

class DetailedGPUInfo {
public:
	DetailedGPUInfo();
	~DetailedGPUInfo();

	//returns info for all gpus
	vector<GPUData> get_all_gpus();

	//returns info for only the primary gpu (gpu 0)
	GPUData primary_gpu_info();
};