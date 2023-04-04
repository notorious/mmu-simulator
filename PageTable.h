//
// Created by Vi on 3/20/2023.
//

#ifndef A3_PAGETABLE_H
#define A3_PAGETABLE_H

#include <cstdint>
#include <iostream>
#include "Map.h"
#include "Level.h"

struct PageTable {
    // For more details, check out the PageTable.cpp
    int levelCount;
    unsigned int offset;
    int totalBytesUsed;
    uint32_t * bitMaskArray;
    int * shiftArray;
    int * entryCount;
    Level * rootLevelPtr;
    int frameCount;
    PageTable(int level, const uint32_t * bm, const int * shift, const int * entry, unsigned int offset);
};

// Functions that operate on the PageTable
unsigned int virtualAddressToVPN(unsigned int virtualAddress, unsigned int mask, unsigned int shift);
void insert_vpn2pfn(PageTable *pageTable, unsigned int virtualAddress);
Map * lookup_vpn2pfn(PageTable *pageTable, unsigned int virtualAddress);
void getByteTotal(PageTable *pageTable);

#endif //A3_PAGETABLE_H

