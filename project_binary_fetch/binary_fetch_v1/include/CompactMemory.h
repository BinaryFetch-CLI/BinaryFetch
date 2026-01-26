#ifndef COMPACT_MEMORY_H
#define COMPACT_MEMORY_H

#include <string>
using namespace std;

class CompactMemory {
public:
    double get_total_memory();
    double get_free_memory();
    double get_used_memory_percent();
    int memory_slot_used();
    int memory_slot_available();
};

#endif
