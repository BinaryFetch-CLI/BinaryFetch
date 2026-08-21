#pragma once
#include <iostream>
using namespace std;
class CompactOS {
public:
    string getOSName();
    string getOSBuild();
    string getArchitecture();
    string getUptime();
};