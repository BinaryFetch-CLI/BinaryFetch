/*
===============================================================
  Project: BinaryFetch — System Information & Hardware Insights Tool
  Author:  Maruf Hasan  |  Founder of BinaryOxide
  Language: C++17 (Windows API)
  File: StorageInfo.cpp (FULL SNAPSHOT READ/WRITE VERSION) - FIXED
  --------------------------------------------------------------
  Overview:
    Handles all disk-related functionalities — identification,
    total/used/free space, snapshot read/write speeds, and predicted speeds.
    Fix: read-test will create a temporary file first if missing so reads are valid.
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

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "cfgmgr32.lib")

using namespace std;

// Property ID definitions
#ifndef StorageDeviceSeekPenaltyProperty
#define StorageDeviceSeekPenaltyProperty ((STORAGE_PROPERTY_ID)7)
#endif

#ifndef StorageDeviceTrimProperty  
#define StorageDeviceTrimProperty ((STORAGE_PROPERTY_ID)8)
#endif

// ============================================================
//  Function: get_storage_type()
string StorageInfo::get_storage_type(const string&, const string& root_path, bool) {
    string type = "Unknown";
    char letter = toupper(root_path[0]);
    string volumePath = "\\\\.\\" + string(1, letter) + ":";

    HANDLE hVol = CreateFileA(volumePath.c_str(), GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hVol == INVALID_HANDLE_VALUE) return type;

    BYTE buf[512]{};
    DWORD returned = 0;
    if (!DeviceIoControl(hVol, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, nullptr, 0,
        buf, sizeof(buf), &returned, nullptr))
    {
        CloseHandle(hVol);
        return type;
    }

    auto* ext = reinterpret_cast<VOLUME_DISK_EXTENTS*>(buf);
    if (ext->NumberOfDiskExtents == 0) { CloseHandle(hVol); return type; }
    DWORD diskNumber = ext->Extents[0].DiskNumber;
    CloseHandle(hVol);

    string physPath = "\\\\.\\PhysicalDrive" + to_string(diskNumber);
    HANDLE hDisk = CreateFileA(physPath.c_str(), GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hDisk == INVALID_HANDLE_VALUE) return type;

    STORAGE_PROPERTY_QUERY q{};
    q.PropertyId = StorageDeviceProperty;
    q.QueryType = PropertyStandardQuery;

    STORAGE_DESCRIPTOR_HEADER hdr{};
    if (!DeviceIoControl(hDisk, IOCTL_STORAGE_QUERY_PROPERTY, &q, sizeof(q),
        &hdr, sizeof(hdr), &returned, nullptr))
    {
        CloseHandle(hDisk);
        return type;
    }

    vector<BYTE> dbuf(hdr.Size);
    if (!DeviceIoControl(hDisk, IOCTL_STORAGE_QUERY_PROPERTY, &q, sizeof(q),
        dbuf.data(), (DWORD)hdr.Size, &returned, nullptr))
    {
        CloseHandle(hDisk);
        return type;
    }

    auto* desc = reinterpret_cast<STORAGE_DEVICE_DESCRIPTOR*>(dbuf.data());

    if (desc->BusType == BusTypeUsb || desc->RemovableMedia) { type = "USB"; CloseHandle(hDisk); return type; }
    if (desc->BusType == BusTypeNvme) { type = "SSD"; CloseHandle(hDisk); return type; }

    if (desc->BusType == BusTypeSata || desc->BusType == BusTypeScsi ||
        desc->BusType == BusTypeSas || desc->BusType == BusTypeAta ||
        desc->BusType == BusTypeAtapi || desc->BusType == BusTypeRAID)
    {
        STORAGE_PROPERTY_QUERY seekQuery{};
        seekQuery.PropertyId = StorageDeviceSeekPenaltyProperty;
        seekQuery.QueryType = PropertyStandardQuery;
        BYTE seekBuffer[512] = { 0 };
        DWORD bytesReturned = 0;

        BOOL seekResult = DeviceIoControl(hDisk, IOCTL_STORAGE_QUERY_PROPERTY,
            &seekQuery, sizeof(seekQuery), &seekBuffer, sizeof(seekBuffer),
            &bytesReturned, nullptr);

        if (seekResult && bytesReturned >= 9) {
            BYTE incursSeekPenalty = seekBuffer[8];
            type = (incursSeekPenalty == 0) ? "SSD" : "HDD";
            CloseHandle(hDisk);
            return type;
        }

        STORAGE_PROPERTY_QUERY trimQuery{};
        trimQuery.PropertyId = StorageDeviceTrimProperty;
        trimQuery.QueryType = PropertyStandardQuery;
        BYTE trimBuffer[512] = { 0 };
        bytesReturned = 0;

        BOOL trimResult = DeviceIoControl(hDisk, IOCTL_STORAGE_QUERY_PROPERTY,
            &trimQuery, sizeof(trimQuery), &trimBuffer, sizeof(trimBuffer),
            &bytesReturned, nullptr);

        if (trimResult && bytesReturned >= 9) {
            BYTE trimEnabled = trimBuffer[8];
            type = (trimEnabled == 1) ? "SSD" : "HDD";
            CloseHandle(hDisk);
            return type;
        }

        type = "Unknown";
    }

    CloseHandle(hDisk);
    return type;
}

// ============================================================
//  Helper: get_sector_size()
static DWORD get_sector_size(const string& root_path) {
    DWORD sectorsPerCluster = 0, bytesPerSector = 0, numFreeClusters = 0, totalClusters = 0;
    if (GetDiskFreeSpaceA(root_path.c_str(), &sectorsPerCluster, &bytesPerSector,
        &numFreeClusters, &totalClusters))
    {
        return (bytesPerSector > 0) ? bytesPerSector : 4096;
    }
    return 4096;
}

// ============================================================
//  Helper: measure snapshot read/write speed (fixed)
//    - Ensures read test creates temp file first if needed
//    - Uses 32 MiB buffer like your working code
// ============================================================
static double measure_disk_speed(const string& root_path, bool writeTest) {
    const size_t BUF_SIZE = 32 * 1024 * 1024; // 32 MB
    vector<char> buffer(BUF_SIZE, 0xAA);

    // test file path: C:\speed_test_tmp.bin
    string testFile = root_path + "speed_test_tmp.bin";

    DWORD flags = FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD bytesDone = 0;
    BOOL ok = FALSE;

    // If writeTest: create/overwrite file and write buffer once
    if (writeTest) {
        hFile = CreateFileA(
            testFile.c_str(),
            GENERIC_WRITE,
            0,
            nullptr,
            CREATE_ALWAYS,
            flags,
            nullptr
        );

        if (hFile == INVALID_HANDLE_VALUE) return 0.0;

        auto start = chrono::high_resolution_clock::now();
        ok = WriteFile(hFile, buffer.data(), (DWORD)buffer.size(), &bytesDone, nullptr);
        FlushFileBuffers(hFile);
        auto end = chrono::high_resolution_clock::now();

        CloseHandle(hFile);
        DeleteFileA(testFile.c_str());

        if (!ok || bytesDone == 0) return 0.0;
        double seconds = chrono::duration<double>(end - start).count();
        return (bytesDone / (1024.0 * 1024.0)) / seconds;
    }

    // If readTest: ensure test file exists first (create it if needed),
    // then open for reading and measure.
    // Try to open existing file for read
    hFile = CreateFileA(
        testFile.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        flags,
        nullptr
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        // file doesn't exist — create it first (write once) so read test has content
        HANDLE hCreate = CreateFileA(
            testFile.c_str(),
            GENERIC_WRITE,
            0,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
            nullptr
        );
        if (hCreate == INVALID_HANDLE_VALUE) {
            // cannot create test file -> can't measure read
            return 0.0;
        }

        DWORD written = 0;
        // write a single buffer (no NO_BUFFERING here to maximize compatibility)
        if (!WriteFile(hCreate, buffer.data(), (DWORD)buffer.size(), &written, nullptr) || written == 0) {
            CloseHandle(hCreate);
            DeleteFileA(testFile.c_str());
            return 0.0;
        }
        FlushFileBuffers(hCreate);
        CloseHandle(hCreate);

        // now open for read with original flags
        hFile = CreateFileA(
            testFile.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_EXISTING,
            flags,
            nullptr
        );

        if (hFile == INVALID_HANDLE_VALUE) {
            DeleteFileA(testFile.c_str());
            return 0.0;
        }
    }

    // perform the timed read
    auto start = chrono::high_resolution_clock::now();
    ok = ReadFile(hFile, buffer.data(), (DWORD)buffer.size(), &bytesDone, nullptr);
    auto end = chrono::high_resolution_clock::now();

    CloseHandle(hFile);
    DeleteFileA(testFile.c_str());

    if (!ok || bytesDone == 0) return 0.0;
    double seconds = chrono::duration<double>(end - start).count();
    return (bytesDone / (1024.0 * 1024.0)) / seconds;
}

// ============================================================
//  Function: StorageInfo::get_all_storage_info()
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
                double used_percent = (total_gib > 0) ? (used_gib / total_gib) * 100.0 : 0.0;

                char fs_name[MAX_PATH] = { 0 };
                GetVolumeInformationA(root_path.c_str(), nullptr, 0, nullptr, nullptr, nullptr, fs_name, sizeof(fs_name));
                string formatted_fs = string(fs_name);
                if (formatted_fs == "NTFS") formatted_fs = "NTFS ";

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
                disk.file_system = formatted_fs;
                disk.is_external = is_external;

                // Storage type
                disk.storage_type = get_storage_type(disk.drive_letter, root_path, is_external);

                // If the drive type cannot be identified → skip it (example: Virtual CD drives)
                if (disk.storage_type == "Unknown") {
                    drive_mask >>= 1;
                    drive_letter++;
                    continue;   // do NOT add to all_disks
                }

                // Snapshot read/write speeds (ONLY for valid drives)
                double w = measure_disk_speed(root_path, true);
                double r = measure_disk_speed(root_path, false);


                ostringstream ss;
                ss << fixed << setprecision(2) << r;
                disk.read_speed = ss.str();
                ss.str("");
                ss.clear();
                ss << fixed << setprecision(2) << w;
                disk.write_speed = ss.str();

                // Serial number placeholder
                disk.serial_number = "SN-" + to_string(1000 + disk_index);

                // Predicted speeds based on type
                if (disk.storage_type == "USB") {
                    disk.predicted_read_speed = "100";
                    disk.predicted_write_speed = "80";
                }
                else if (disk.storage_type == "SSD") {
                    disk.predicted_read_speed = "500";
                    disk.predicted_write_speed = "450";
                }
                else if (disk.storage_type == "HDD") {
                    disk.predicted_read_speed = "140";
                    disk.predicted_write_speed = "120";
                }
                else {
                    disk.predicted_read_speed = "---";
                    disk.predicted_write_speed = "---";
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

// ============================================================
//  Function: StorageInfo::process_storage_info()
//  Processes each disk and calls callback immediately
// ============================================================
void StorageInfo::process_storage_info(std::function<void(const storage_data&)> callback) {
    DWORD drive_mask = GetLogicalDrives();
    if (drive_mask == 0) return;

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
                double used_percent = (total_gib > 0) ? (used_gib / total_gib) * 100.0 : 0.0;

                char fs_name[MAX_PATH] = { 0 };
                GetVolumeInformationA(root_path.c_str(), nullptr, 0, nullptr, nullptr, nullptr, fs_name, sizeof(fs_name));
                string formatted_fs = string(fs_name);
                if (formatted_fs == "NTFS") formatted_fs = "NTFS ";

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
                disk.file_system = formatted_fs;
                disk.is_external = is_external;

                // Storage type
                disk.storage_type = get_storage_type(disk.drive_letter, root_path, is_external);

                // Skip unknown drives
                if (disk.storage_type == "Unknown") {
                    drive_mask >>= 1;
                    drive_letter++;
                    continue;
                }

                // Snapshot read/write speeds
                double w = measure_disk_speed(root_path, true);
                double r = measure_disk_speed(root_path, false);

                ostringstream ss;
                ss << fixed << setprecision(2) << r;
                disk.read_speed = ss.str();
                ss.str("");
                ss.clear();
                ss << fixed << setprecision(2) << w;
                disk.write_speed = ss.str();

                // Serial number placeholder
                disk.serial_number = "SN-" + to_string(1000 + disk_index);

                // Predicted speeds based on type
                if (disk.storage_type == "USB") {
                    disk.predicted_read_speed = "100";
                    disk.predicted_write_speed = "80";
                }
                else if (disk.storage_type == "SSD") {
                    disk.predicted_read_speed = "500";
                    disk.predicted_write_speed = "450";
                }
                else if (disk.storage_type == "HDD") {
                    disk.predicted_read_speed = "140";
                    disk.predicted_write_speed = "120";
                }
                else {
                    disk.predicted_read_speed = "---";
                    disk.predicted_write_speed = "---";
                }

                // CALL CALLBACK IMMEDIATELY for this disk
                callback(disk);

                disk_index++;
            }
        }
        drive_letter++;
        drive_mask >>= 1;
    }
}

/*
===============================================================
  End of File - Snapshot Read/Write Fully Integrated (BUG FIXED)
===============================================================
*/
