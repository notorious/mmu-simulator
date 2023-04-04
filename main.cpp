#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <vector>
#include "vaddr_tracereader.h"
#include "PageTable.h"
#include "print_helpers.h"
#include <cctype>
#include <algorithm>
#include <cmath>
#include "TLB.h"

// Definitions
#define SUCCESS_EXIT 0
#define BAD_EXIT 1

using namespace std;
bool isNum(char *str);

int main(int argc, char ** argv) {
    // Optional Arguments
    int option;
    int numOfAddressToProcess = 0; // if this is 0, read all addresses
    int cacheCapacityTLB = 0; // 0 as default
    int printMode = 0; // 0 - summary, 1 - levelbitmasks, 2 - va2pa, 3 - va2pa_tlb_ptwalk, 4 - vpn2pfn, - 5 offset
    string printModeString;

    // Handles optional arguments
    while ((option = getopt(argc, argv, "n:c:p:")) != -1) {
        switch (option) {
            case 'c':
                // cacheCapacityTLB
                if (isNum(optarg) == 1 && atoi(optarg) > 0 ) {
                    cacheCapacityTLB = atoi(optarg);
                } else {
                    cerr << "Cache capacity must be a number, greater than or equal to 0" << endl;
                    exit(BAD_EXIT);
                }
                break;
            case 'n':
                // numOfAddressToProcess
                if (isNum(optarg) == 1 && atoi(optarg) > 0) {
                    numOfAddressToProcess = atoi(optarg);
                } else {
                    cerr << "Number of memory accesses must be a number, greater than or equal to 0" << endl;
                    exit(BAD_EXIT);
                }
                break;
            case 'p':
                // printModeString
                printModeString = optarg;
                break;
            default:
                cerr << "Invalid option: " << option << endl;
                exit(BAD_EXIT);
        }
    }

    // Print Mode String -> Converter
    // Transform it into lower case string
    transform(printModeString.begin(), printModeString.end(), printModeString.begin(), ::tolower);

    if (printModeString == "levelbitmasks") {
        printMode = 1;
    } else if (printModeString == "va2pa") {
        printMode = 2;
    } else if(printModeString == "va2pa_tlb_ptwalk") {
        printMode = 3;
    } else if (printModeString == "vpn2pfn") {
        printMode = 4;
    } else if (printModeString == "offset") {
        printMode = 5;
    } else if (printModeString.empty() || printModeString == "summary") {
        printMode = 0;
    } else {
        cout << "Invalid printing method" << endl;
        exit(BAD_EXIT);
    }

    // TraceFile validator checker
    int idx = optind;
    const char *traceFileName = argv[idx];
    FILE* traceFile;
    traceFile = fopen(traceFileName, "r");
    // Invalid Trace File Exit
    if (traceFile == nullptr) {
        exit(BAD_EXIT);
    }

    // Page level arguments
    vector<int> bitsPerLevel;
    idx++;

    // No levels arguments, exit with a status of one
    if (argc - idx < 1) {
        exit(BAD_EXIT);
    }

    int levelCount = 0;
    int totalBits = 0;

    // Processing bitsPerLevel
    for (int i = idx; i < argc; i++) {
        if (isNum(argv[i]) && atoi(argv[i]) >= 1) {
            bitsPerLevel.push_back(atoi(argv[i]));
        } else {
            cerr << "Level " << levelCount << " page table must be atleast 1 bit" << endl;
            exit(BAD_EXIT);
        }
        // If total bits exceeds 28, exit
        totalBits += atoi(argv[i]);
        levelCount += 1;
    }

    if (totalBits > 28) {
        cerr << "Too many bits used in page tables" << endl;
        exit(BAD_EXIT);
    }

    // Populate PageTable & Setup
    int entryCount[levelCount];
    int shiftArray[levelCount];

    // Populate entryCount
    for (int i = 0; i < bitsPerLevel.size(); i++) {
        entryCount[i] = 1 << bitsPerLevel[i];
    }

    // Populate shiftArray
    int shiftArrayAccumulator = 0;
    for (int i = 0; i < bitsPerLevel.size(); i++) {
        shiftArray[i] = 32 - bitsPerLevel[i] - shiftArrayAccumulator;
        shiftArrayAccumulator += bitsPerLevel[i];
    }

    // Populate BitMasking
    int bitCounter = 32;
    uint32_t bitMaskingArray[bitsPerLevel.size()];

    // Iterate through each levels
    for (int i = 0; i < bitsPerLevel.size(); i++) {
        uint32_t bitMask = 0;
        int numOfMaskedBits = bitsPerLevel[i];

        // Add 1 based on how many binary digits needs masking
        for (int j = 0; j < numOfMaskedBits; j++) {
            bitMask = bitMask << 1;
            bitMask = bitMask | 1;
        }

        // subtract from original 32 to keep track of position
        bitCounter -= bitsPerLevel[i];

        // shift the bits to mask at the location wanted
        bitMask = bitMask << bitCounter;

        // place it in the array
        bitMaskingArray[i] = bitMask;
    }

    // Calculate offset
    unsigned offsetBits = 32 - totalBits;
    unsigned offset = 0;

    for (int i = 0; i < offsetBits; i++) {
        offset = offset << 1;
        offset = offset | 1;
    }

    // Page table initialization
    PageTable *pg = new PageTable(levelCount, bitMaskingArray, shiftArray, entryCount, offset);

    // 0 - summary, 1 - levelbitmasks, 2 - va2pa, 3 - va2pa_tlb_ptwalk, 4 - vpn2pfn, - 5 offset
    int counter = 0;
    p2AddrTr mtrace;

    // Cache Hits, Page Hits
    int cacheHits = 0;
    int pageHits = 0;
    int addressProcessed = 0;

    // Misses (Both TLB and Lookup)
    int misses = 0;

    // Page Size
    int pageSize = pow(2, offsetBits);

    // Initialize TLB
    TLB * newTLB = new TLB(cacheCapacityTLB, totalBits);

    while (NextAddress(traceFile, &mtrace)) {
        unsigned int virtualAddress = mtrace.addr;

        // Summary -> this will collect data for the summary print option
        if(printMode == 0) {

            addressProcessed++;

            // There no caching
            if (cacheCapacityTLB == 0) {
                // If lookup is failed then increment misses else increment pageHits.
                if (lookup_vpn2pfn(pg, virtualAddress) == nullptr) {
                    misses++;
                } else {
                    pageHits++;
                }
            } else { // with caching and TLB
                if(newTLB->lookUp(newTLB->VAtoVPN(virtualAddress))) {
                    cacheHits++;
                } else {
                    if (lookup_vpn2pfn(pg,virtualAddress) != nullptr) {
                        pageHits++;
                        uint32_t PFN = lookup_vpn2pfn(pg, virtualAddress)->frameNumber;
                        newTLB->insert(newTLB->VAtoVPN(virtualAddress), PFN);
                    } else {
                        misses++;
                        insert_vpn2pfn(pg,virtualAddress);
                        uint32_t PFN = lookup_vpn2pfn(pg, virtualAddress)->frameNumber;
                        newTLB->insert(newTLB->VAtoVPN(virtualAddress), PFN);
                    }
                }
            }

            insert_vpn2pfn(pg, virtualAddress);
        }

        // report_levelbitmasks * WORKS
        if (printMode == 1) {
            report_levelbitmasks(levelCount, pg->bitMaskArray);
            break;
        }

        // report_virtualAddr2physicalAddr * WORKS
        if (printMode == 2) {
            insert_vpn2pfn(pg, virtualAddress);
            uint32_t PFN = lookup_vpn2pfn(pg, virtualAddress)->frameNumber;
            PFN = (PFN << (32-totalBits)) | (virtualAddress & pg->offset);
            report_virtualAddr2physicalAddr(virtualAddress, PFN);
        }

        // report_va2pa_TLB_PTwalk
        if(printMode == 3) {
            // Lookup in the TLB first
            if(newTLB->lookUp(newTLB->VAtoVPN(virtualAddress))) {
                uint32_t PFN = newTLB->TLBMap[newTLB->VAtoVPN(virtualAddress)];
                PFN = (PFN << (32-totalBits)) | (virtualAddress & pg->offset);
                report_va2pa_TLB_PTwalk(virtualAddress,PFN, true, true);
            } else {
                // If TLB Miss, then do Page Table Walk
                if (lookup_vpn2pfn(pg,virtualAddress) != nullptr) {
                    uint32_t PFN = lookup_vpn2pfn(pg, virtualAddress)->frameNumber;
                    PFN = (PFN << (32-totalBits)) | (virtualAddress & pg->offset);
                    report_va2pa_TLB_PTwalk(virtualAddress,PFN, false, true);
                } else { // If both TLB and Page table misses, then insert
                    insert_vpn2pfn(pg,virtualAddress);
                    uint32_t PFN = lookup_vpn2pfn(pg, virtualAddress)->frameNumber;
                    newTLB->insert(newTLB->VAtoVPN(virtualAddress), PFN);
                    PFN = (PFN << (32-totalBits)) | (virtualAddress & pg->offset);
                    report_va2pa_TLB_PTwalk(virtualAddress,PFN, false, false);
                }
            }
        }

        // vpn2pfn * WORKS
        if (printMode == 4) {
            insert_vpn2pfn(pg, virtualAddress);
            uint32_t vpnArr[pg->levelCount];
            for (int k = 0; k < pg->levelCount; k++) {
                vpnArr[k] = virtualAddressToVPN(virtualAddress, pg->bitMaskArray[k], pg->shiftArray[k]);
            }
            unsigned int frame = lookup_vpn2pfn(pg, virtualAddress)->frameNumber;
            report_pagetable_map(pg->levelCount, vpnArr, frame);
        }

        // offset * WORKS
        if (printMode == 5) {
            hexnum((virtualAddress & pg->offset));
        }

        // this mean there is a limit, if there isn't read all num
        // default number is 0 to read all
        if (numOfAddressToProcess > 0) {
            if (counter + 1 >= numOfAddressToProcess) {
                break;
            }
        }

        counter++;
    }

    // For summary byte used
    getByteTotal(pg);

    // Print this out if print mode is summary
    if (printMode == 0) {
        report_summary(pageSize, cacheHits, pageHits, addressProcessed, pg->frameCount, pg->totalBytesUsed);
    }

    return SUCCESS_EXIT;
}

// A function that takes in a string
// See if it is a number
bool isNum(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) {
            return false;
        }
    }
    return true;
}
