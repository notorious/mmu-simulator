//
// Created by Vi on 3/29/2023.
//

#ifndef MMUWITHTLB_TLB_H
#define MMUWITHTLB_TLB_H

#include <unordered_map>
#include <cstdint>

struct TLB {
    int TLBCacheSize;
    int addressCounter;
    uint32_t vpnBitMasking;
    std::unordered_map<unsigned int, unsigned int> TLBMap; // Maps VPN -> PFN
    std::unordered_map<unsigned int, int> MRAMap; // Maps VPN -> Most Recently Access Time (addressCounter)
    TLB(int TLBCache, int totalVPN);
    bool lookUp(unsigned int VPN);
    void insert(unsigned int vpn, unsigned int pfn);
    unsigned int VAtoVPN(unsigned VA);
};

#endif //MMUWITHTLB_TLB_H
