#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <functional>
using namespace std;

struct storage_data {
    string drive_letter;
    string used_space;
    string total_space;
    string used_percentage;
    string file_system;
    bool is_external;
    string storage_type;
    string serial_number;
    string read_speed;
    string write_speed;
    string predicted_read_speed;
    string predicted_write_speed;
};

class StorageInfo {
public:
    vector<storage_data> get_all_storage_info();

    // NEW: Process disks one-by-one with callback
    void process_storage_info(std::function<void(const storage_data&)> callback);

private:
    string get_storage_type(const string& drive_letter, const string& root_path, bool is_external);
};