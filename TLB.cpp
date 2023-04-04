//
// Created by Vi on 3/29/2023.
//

#include "TLB.h"

#include <cstdint>
#include <limits>


TLB::TLB(int TLBCache, int totalVPN) {
    this->TLBCacheSize = TLBCache;
    this->addressCounter = 0;

    uint32_t bitMask = 0;
    for (int j = 0; j < totalVPN; j++) {
        bitMask = bitMask << 1;
        bitMask = bitMask | 1;
    }

    bitMask = bitMask << (32-totalVPN);
    this->vpnBitMasking = bitMask;
}

bool TLB::lookUp(unsigned int VPN) {
    if (this->TLBMap.count(VPN) != 0) {
        // VPN Exist in TLB
        // Update the VPN Access Counter in MRAMap
        this->MRAMap[VPN] = this->addressCounter;
        this->addressCounter++;
        return true;
    }

    // VPN is not in the TLB
    return false;
}

void TLB::insert(unsigned int VPN, unsigned int PFN) {
    int MAX_MRU_SIZE = std::min(this->TLBCacheSize, 8);

    // TLB is full, and must evict a VPN using the LRU cache algorithm
    if (this->TLBMap.size() == this->TLBCacheSize) {
        unsigned int vpnToRemove = 0;
        int LRU = std::numeric_limits<int>::max();

        // Iterate through the Most Recently Access, using LRU caching replacement algorithm
        // Find the one with the lowest time, and remove it
        for (auto const& p : this->MRAMap) {
            auto const& key = p.first;
            auto const& val = p.second;
            if (val < LRU) {
                LRU = val;
                vpnToRemove = key;
            }
        }

        // Remove off both the TLB and MRA
        TLBMap.erase(vpnToRemove);
        MRAMap.erase(vpnToRemove);
    }

    if (this->MRAMap.size() == MAX_MRU_SIZE) {
        // If the MRU is full and the accessed VPN is not in the MRU, evict the LRU VPN from MRU only
        unsigned int vpnToRemove = 0;
        int LRU = std::numeric_limits<int>::max();;

        // Iterate through the Most Recently Access, using LRU caching replacement algorithm
        // Find the one with the lowest time, and remove it from MRU only
        for (auto const& p : this->MRAMap) {
            auto const& key = p.first;
            auto const& val = p.second;
            if (val < LRU) {
                LRU = val;
                vpnToRemove = key;
            }
        }

        MRAMap.erase(vpnToRemove);
    }

    // If the TLB is not full
    // Just insert normally
    TLBMap[VPN] = PFN;
    MRAMap[VPN] = this->addressCounter;

    // Increment it
    this->addressCounter++;
}

unsigned int TLB::VAtoVPN(unsigned int VA) {
    return (VA & this->vpnBitMasking);
}

