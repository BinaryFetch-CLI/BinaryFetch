/*
===============================================================
  Project: BinaryFetch — System Information & Hardware Insights Tool
  Author:  Maruf Hasan  |  Founder of BinaryOxide
  Language: C++17 (Windows API)
  File: StorageInfo.cpp
  --------------------------------------------------------------
  Overview:
    Handles all disk-related functionalities — identification,
    total/used/free space, read/write speeds, and predicted speeds.
===============================================================
*/

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0603  // Windows 8.1 or newer
#endif

#include "StorageInfo.h"
#include <Windows.h>
#include <sstream>
#include <iomanip>
#include <vector>
#include <iostream>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <malloc.h>
#include <winioctl.h>
#include <setupapi.h>
#include <devguid.h>
#include <cfgmgr32.h>

// Add this definition near the top of your file, after includes:
#ifndef StorageDeviceRotationRateProperty
#define StorageDeviceRotationRateProperty ((STORAGE_PROPERTY_ID)7)
#endif

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "cfgmgr32.lib")

using namespace std;

// ============================================================
//  Function: get_storage_type()
// ============================================================
string StorageInfo::get_storage_type(const string& drive_letter, const string& root_path, bool is_external) {
    string storage_type = "---";

    if (is_external) {
        // External drives: try USB/SD heuristics
        string letter = drive_letter.substr(5, 1); // 'C' from "Disk (C:)"
        storage_type = "USB";

        // Try bus type first
        string volume_path = "\\\\.\\" + letter + ":";
        HANDLE hDevice = CreateFileA(volume_path.c_str(),
            0, FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, OPEN_EXISTING, 0, NULL);

        if (hDevice != INVALID_HANDLE_VALUE) {
            STORAGE_PROPERTY_QUERY query{};
            query.PropertyId = StorageDeviceProperty;
            query.QueryType = PropertyStandardQuery;

            STORAGE_DESCRIPTOR_HEADER header{};
            DWORD bytesReturned = 0;

            if (DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
                &query, sizeof(query),
                &header, sizeof(header),
                &bytesReturned, NULL)) {
                DWORD bufferSize = header.Size;
                BYTE* pBuffer = new BYTE[bufferSize];
                if (DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
                    &query, sizeof(query),
                    pBuffer, bufferSize,
                    &bytesReturned, NULL)) {
                    STORAGE_DEVICE_DESCRIPTOR* pDescriptor = (STORAGE_DEVICE_DESCRIPTOR*)pBuffer;
                    if (pDescriptor->BusType == BusTypeUsb) storage_type = "USB";
                    else if (pDescriptor->BusType == BusTypeSd) storage_type = "SD Card";
                }
                delete[] pBuffer;
            }
            CloseHandle(hDevice);
        }

        return storage_type;
    }

    // Internal drives: SSD/HDD detection
    string device_path = "\\\\.\\PhysicalDrive";
    for (int driveIndex = 0; driveIndex < 16; driveIndex++) {
        string current_path = device_path + to_string(driveIndex);
        HANDLE hDevice = CreateFileA(current_path.c_str(), 0,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, OPEN_EXISTING, 0, NULL);

        if (hDevice == INVALID_HANDLE_VALUE) continue;

        STORAGE_PROPERTY_QUERY query{};
        query.PropertyId = StorageDeviceProperty;
        query.QueryType = PropertyStandardQuery;

        STORAGE_DESCRIPTOR_HEADER header{};
        DWORD bytesReturned = 0;

        if (DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
            &query, sizeof(query),
            &header, sizeof(header),
            &bytesReturned, NULL)) {

            DWORD bufferSize = header.Size;
            BYTE* pBuffer = new BYTE[bufferSize];
            if (DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
                &query, sizeof(query),
                pBuffer, bufferSize,
                &bytesReturned, NULL)) {

                STORAGE_DEVICE_DESCRIPTOR* pDescriptor = (STORAGE_DEVICE_DESCRIPTOR*)pBuffer;

                // Check bus type for NVMe or removable media
                if (pDescriptor->BusType == BusTypeNvme) storage_type = "SSD";
                else if (pDescriptor->RemovableMedia) storage_type = "USB";
                else {
                    // Check rotation rate
                    STORAGE_PROPERTY_QUERY rotateQuery{};
                    rotateQuery.PropertyId = StorageDeviceRotationRateProperty;
                    rotateQuery.QueryType = PropertyStandardQuery;

                    DWORD rotateRate = 0;
                    DWORD rotateBytesReturned = 0;
                    if (DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
                        &rotateQuery, sizeof(rotateQuery),
                        &rotateRate, sizeof(rotateRate),
                        &rotateBytesReturned, NULL)) {
                        storage_type = (rotateRate == 0 || rotateRate == 1) ? "SSD" : "HDD";
                    }
                }
            }
            delete[] pBuffer;
        }
        CloseHandle(hDevice);

        if (storage_type != "---") break;
    }

    return storage_type;
}

// ============================================================
//  Helper: get_sector_size()
// ============================================================
static DWORD get_sector_size(const string& root_path) {
    DWORD sectorsPerCluster = 0;
    DWORD bytesPerSector = 0;
    DWORD numberOfFreeClusters = 0;
    DWORD totalNumberOfClusters = 0;
    if (GetDiskFreeSpaceA(root_path.c_str(), &sectorsPerCluster, &bytesPerSector,
        &numberOfFreeClusters, &totalNumberOfClusters)) {
        return (bytesPerSector > 0) ? bytesPerSector : 4096;
    }
    return 4096;
}

// ============================================================
//  Helper: round_up_to()
// ============================================================
static size_t round_up_to(size_t value, size_t align) {
    return ((value + align - 1) / align) * align;
}

// ============================================================
//  Helper: create_test_file_winapi()
// ============================================================
static bool create_test_file_winapi(const string& path, size_t requested_size,
    size_t sector_size, size_t buffer_size = 4 * 1024 * 1024) {
    const string fname = path + "speed_test.tmp";
    size_t aligned_file_size = round_up_to(requested_size, sector_size);
    size_t desired_buffer = round_up_to(buffer_size, sector_size);

    HANDLE h = CreateFileA(fname.c_str(), GENERIC_WRITE, 0, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_WRITE_THROUGH, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;

    vector<char> buf(desired_buffer, 'x');
    size_t remaining = aligned_file_size;
    DWORD written = 0;

    while (remaining > 0) {
        DWORD toWrite = (DWORD)min<size_t>(buf.size(), remaining);
        if (!WriteFile(h, buf.data(), toWrite, &written, nullptr) || written == 0) {
            CloseHandle(h);
            DeleteFileA(fname.c_str());
            return false;
        }
        remaining -= written;
    }

    FlushFileBuffers(h);
    CloseHandle(h);
    return true;
}

// ============================================================
//  Function: measure_write_speed()
// ============================================================
double measure_write_speed(const string& path) {
    const string test_file = path + "speed_test.tmp";
    const size_t file_size = 50ull * 1024 * 1024;
    size_t sector_size = get_sector_size(path);
    size_t buffer_size = round_up_to(4ull * 1024 * 1024, sector_size);

    void* aligned_buf = _aligned_malloc(buffer_size, sector_size);
    if (!aligned_buf) return 0.0;
    memset(aligned_buf, 'x', buffer_size);

    HANDLE h = CreateFileA(test_file.c_str(), GENERIC_WRITE, 0, nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH,
        nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        _aligned_free(aligned_buf);
        DeleteFileA(test_file.c_str());
        return 0.0;
    }

    size_t remaining = file_size;
    DWORD bytesWritten = 0;
    auto start = chrono::high_resolution_clock::now();

    while (remaining > 0) {
        DWORD toWrite = (DWORD)min<size_t>(buffer_size, remaining);
        if (!WriteFile(h, aligned_buf, toWrite, &bytesWritten, nullptr) || bytesWritten == 0) break;
        remaining -= bytesWritten;
    }

    FlushFileBuffers(h);
    CloseHandle(h);

    auto end = chrono::high_resolution_clock::now();
    _aligned_free(aligned_buf);
    DeleteFileA(test_file.c_str());

    double seconds = chrono::duration<double>(end - start).count();
    return (seconds > 0.0) ? (double)file_size / (1024.0 * 1024.0) / seconds : 0.0;
}

// ============================================================
//  Function: measure_read_speed()
// ============================================================
double measure_read_speed(const string& path) {
    const string test_file = path + "speed_test.tmp";
    const size_t file_size = 50ull * 1024 * 1024;
    size_t sector_size = get_sector_size(path);
    size_t buffer_size = round_up_to(4ull * 1024 * 1024, sector_size);

    if (!create_test_file_winapi(path, file_size, sector_size, buffer_size)) {
        DeleteFileA(test_file.c_str());
        return 0.0;
    }

    void* aligned_buf = _aligned_malloc(buffer_size, sector_size);
    if (!aligned_buf) { DeleteFileA(test_file.c_str()); return 0.0; }

    HANDLE h = CreateFileA(test_file.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_NO_BUFFERING,
        nullptr);
    if (h == INVALID_HANDLE_VALUE) { _aligned_free(aligned_buf); DeleteFileA(test_file.c_str()); return 0.0; }

    size_t totalRead = 0;
    DWORD bytesRead = 0;
    auto start = chrono::high_resolution_clock::now();

    while (true) {
        if (!ReadFile(h, aligned_buf, (DWORD)buffer_size, &bytesRead, nullptr) || bytesRead == 0) break;
        totalRead += bytesRead;
        if (totalRead >= file_size) break;
    }

    auto end = chrono::high_resolution_clock::now();
    CloseHandle(h);
    _aligned_free(aligned_buf);
    DeleteFileA(test_file.c_str());

    double seconds = chrono::duration<double>(end - start).count();
    return (seconds > 0.0 && totalRead > 0) ? (double)totalRead / (1024.0 * 1024.0) / seconds : 0.0;
}

// ============================================================
//  Function: StorageInfo::get_all_storage_info()
// ============================================================
vector<storage_data> StorageInfo::get_all_storage_info() {
    vector<storage_data> all_disks;
    DWORD drive_mask = GetLogicalDrives();
    if (drive_mask == 0) return all_disks;

    char drive_letter = 'A';
    int disk_index = 0;

    while (drive_mask) {
        if (drive_mask & 1) {
            string root_path = string(1, drive_letter) + ":\\";
            ULARGE_INTEGER free_bytes, total_bytes, free_bytes_available;

            if (GetDiskFreeSpaceExA(root_path.c_str(), &free_bytes_available, &total_bytes, &free_bytes)) {
                double total_gib = total_bytes.QuadPart / (1024.0 * 1024.0 * 1024.0);
                double free_gib = free_bytes.QuadPart / (1024.0 * 1024.0 * 1024.0);
                double used_gib = total_gib - free_gib;
                double used_percent = (used_gib / total_gib) * 100.0;

                char fs_name[MAX_PATH] = { 0 };
                GetVolumeInformationA(root_path.c_str(), nullptr, 0, nullptr, nullptr, nullptr, fs_name, sizeof(fs_name));
                UINT drive_type = GetDriveTypeA(root_path.c_str());
                bool is_external = (drive_type == DRIVE_REMOVABLE);

                ostringstream used_str, total_str, percent_str;
                used_str << fixed << setprecision(2) << used_gib;
                total_str << fixed << setprecision(2) << total_gib;
                percent_str << "(" << (int)used_percent << "%)";

                storage_data disk;
                disk.drive_letter = "Disk (" + string(1, drive_letter) + ":)";
                disk.used_space = used_str.str();
                disk.total_space = total_str.str();
                disk.used_percentage = percent_str.str();
                disk.file_system = fs_name;
                disk.is_external = is_external;

                disk.storage_type = get_storage_type(disk.drive_letter, root_path, is_external);

                // Only measure speeds if unknown
                if (disk.storage_type == "---") {
                    double r = measure_read_speed(root_path);
                    double w = measure_write_speed(root_path);
                    if (r > 200.0 && w > 150.0) disk.storage_type = "SSD";
                    else if (r > 80.0 && w > 60.0) disk.storage_type = (total_gib >= 500) ? "HDD" : "SSD";
                    else disk.storage_type = "HDD";
                }

                disk.serial_number = "SN-" + to_string(1000 + disk_index);

                double r = measure_read_speed(root_path);
                double w = measure_write_speed(root_path);

                ostringstream ss;
                ss << fixed << setprecision(2) << r; disk.read_speed = ss.str();
                ss.str(""); ss.clear();
                ss << fixed << setprecision(2) << w; disk.write_speed = ss.str();

                // Predicted speeds
                if (is_external) {
                    disk.predicted_read_speed = "100";
                    disk.predicted_write_speed = "80";
                }
                else {
                    if (total_gib >= 500) {
                        disk.predicted_read_speed = "140";
                        disk.predicted_write_speed = "120";
                    }
                    else {
                        disk.predicted_read_speed = "450";
                        disk.predicted_write_speed = "400";
                    }
                }

                all_disks.push_back(disk);
                disk_index++;
            }
        }
        drive_letter++;
        drive_mask >>= 1;
    }

    return all_disks;
}

/*
===============================================================
  End of File
===============================================================
*/
