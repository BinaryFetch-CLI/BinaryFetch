#pragma once
#include <iostream>
#include <windows.h>
#include <VersionHelpers.h>
using namespace std;
class CompactOS {
public:
    std::string getOSName();
    std::string getOSBuild();
    std::string getArchitecture();
    std::string getUptime();
};