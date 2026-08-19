#pragma once
#include <iostream>
#include <windows.h>
#include <VersionHelpers.h>
using namespace std;
class CompactOS {
public:
    string getOSName();
    string getOSBuild();
    string getArchitecture();
    string getUptime();
};