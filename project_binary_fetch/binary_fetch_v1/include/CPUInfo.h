#pragma once
#include <string>
using namespace std;

class CPUInfo {
public:
	// cpu brand and model
	string get_cpu_info();

	// utilization percentage (like task manager)
	float get_cpu_utilization();

	// cpu speeds
	string get_cpu_base_speed();        // base speed in GHz
	string get_cpu_speed();             // current speed in GHz

	// cpu topology
	int get_cpu_sockets();              // number of sockets
	int get_cpu_cores();                // physical cores
	int get_cpu_logical_processors();   // logical processors (threads)

	// virtualization status
	string get_cpu_virtualization();

	// cache information
	string get_cpu_l1_cache();          // L1 cache size
	string get_cpu_l2_cache();          // L2 cache size
	string get_cpu_l3_cache();          // L3 cache size

	// system statistics
	string get_system_uptime();         // system uptime
	int get_process_count();            // number of processes
	int get_thread_count();             // number of threads
	int get_handle_count();             // number of handles
};