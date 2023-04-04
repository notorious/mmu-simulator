//
// Created by Vi on 3/20/2023.
//

#include "PageTable.h"
#define NOT_EXIST nullptr

using namespace std;

PageTable::PageTable(int level, const uint32_t *bitMaskingArr, const int *shiftArr, const int *entryArr, unsigned int offset) {
    this->levelCount = level;
    this->bitMaskArray = new uint32_t[level];
    this->shiftArray = new int[level];
    this->entryCount = new int[level];

    // This is used to keep track of the next frame to allocate
    this->frameCount = 0;

    // Total byte used in all levels and page table
    this->totalBytesUsed = 0;

    // offset bit-masking
    this->offset = offset;

    // Populating the shift array, bitmasking, and entry count at each level
    // Signified by the index
    for (int i = 0; i < level; i++) {
        this->shiftArray[i] = shiftArr[i];
        this->bitMaskArray[i] = bitMaskingArr[i];
        this->entryCount[i] = entryArr[i];
    }
    if (levelCount == 1) { // If level == 1, meaning there is only one large singular level
        this->rootLevelPtr = new Level(0, true, this->entryCount[0]);
    } else { // Else there are multiple level, hence first level isn't a leaf level
        this->rootLevelPtr = new Level(0, false, this->entryCount[0]);
    }
}

// VA -> VPN, depend on bit masking & shift
unsigned int virtualAddressToVPN(unsigned int virtualAddress, unsigned int mask, unsigned int shift) {
    return (virtualAddress & mask) >> shift;
}

// Page table walk -> lookup method for page table
Map * lookup_vpn2pfn(PageTable *pageTable, unsigned int virtualAddress) {
    Level * currLevel = pageTable->rootLevelPtr;
    unsigned int virtualPageNumber;
    for (int level = 0; level < pageTable->levelCount; level++) {
        // Get VPN based on the current level
        virtualPageNumber = virtualAddressToVPN(virtualAddress, pageTable->bitMaskArray[level], pageTable->shiftArray[level]);
        // If current level is a leaf level
        if (currLevel->isLeaf) {
            if (currLevel->mapArr[virtualPageNumber] != NOT_EXIST) {
                // Page Table Walk is successful, return the mapping
                return currLevel->mapArr[virtualPageNumber];
            }
            // if the next level isn't valid, return NOT_EXIST -> nullptr
            return NOT_EXIST;
        }
        // if the next level isn't valid, return NOT_EXIST -> nullptr
        if (currLevel->nextLevelPtr[virtualPageNumber] == NOT_EXIST) {
            return NOT_EXIST;
        }
        // Set currLevel to the next level
        currLevel = currLevel->nextLevelPtr[virtualPageNumber];
    }
    // if the next level isn't valid, return NOT_EXIST -> nullptr
    return NOT_EXIST;
}

// Insert method for PageTable
void insert_vpn2pfn(PageTable *pageTable, unsigned int virtualAddress) {
    Level * currLevel = pageTable->rootLevelPtr;
    unsigned int virtualPageNumber;
    // First look up to see if it exists, if it is just return.
    if (lookup_vpn2pfn(pageTable, virtualAddress) != NOT_EXIST) {
        return;
    }
    // Insert VPN -> PFN
    for (int level = 0; level < pageTable->levelCount; level++) {
        virtualPageNumber = virtualAddressToVPN(virtualAddress, pageTable->bitMaskArray[level], pageTable->shiftArray[level]);
        // If it is a leaf level, then create a mapping between VPN -> PFN
        if (currLevel->isLeaf) {
            Map * newMap = new Map(true, pageTable->frameCount);
            pageTable->frameCount++;
            currLevel->mapArr[virtualPageNumber] = newMap;
            return;
        }
        // If it's not a leaf, and doesn't exist
        // Create an inner level node
        if (currLevel->nextLevelPtr[virtualPageNumber] == NOT_EXIST) {
            auto * newLevel = new Level(level + 1, (level + 1 == pageTable->levelCount - 1), pageTable->entryCount[level+1]);
            currLevel->nextLevelPtr[virtualPageNumber] = newLevel;
        }
        // Traverse to the next level
        currLevel = currLevel->nextLevelPtr[virtualPageNumber];
    }
}

// Recursion helper to add up all the bytes used at all levels
void getByteTotalHelper(Level * level, PageTable *pageTable) {
    if (level->isLeaf) {
        // If its leaf
        for (int i = 0; i < pageTable->entryCount[level->depth]; i++) {
            pageTable->totalBytesUsed += sizeof(Map);
        }
    } else {
        // Iterate through the sizes of each level
        for (int i = 0; i < pageTable->entryCount[level->depth]; i++) {
            // If it's not a nullptr, then there another level object at the index
            if(level->nextLevelPtr[i] != nullptr) {
                // Go to that Level object, and recurse this function
                getByteTotalHelper(level->nextLevelPtr[i], pageTable);
            }
        }
        // Account for current level bytes
        pageTable->totalBytesUsed += sizeof(Level);
    }
}

// Recursion starter to calculate all the byte used at all levels
void getByteTotal(PageTable *pageTable) {
    pageTable->totalBytesUsed = sizeof(PageTable);
    getByteTotalHelper(pageTable->rootLevelPtr, pageTable);
}
