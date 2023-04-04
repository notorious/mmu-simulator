//
// Created by Vi on 3/22/2023.
//

#ifndef A3_LEVEL_H
#define A3_LEVEL_H
#include "Map.h"

struct Level {
    // If level is leaf then it will populate the map array
    // Else populate an array of levels
    int depth;
    bool isLeaf;
    Map ** mapArr;
    Level ** nextLevelPtr;
    Level(int depth, bool isLeaf, int size);
};

#endif //A3_LEVEL_H
