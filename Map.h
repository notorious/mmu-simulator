//
// Created by Vi on 3/22/2023.
//

#ifndef A3_MAP_H
#define A3_MAP_H
#include <cstdint>

struct Map{
    // Mapping for the leaf level, VPN -> PFN
    unsigned int frameNumber;
    bool validFlag;
    Map(bool valid, unsigned int frame);
};

#endif //A3_MAP_H
