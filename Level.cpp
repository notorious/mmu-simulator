//
// Created by Vi on 3/22/2023.
//

#include <iostream>
#include "Level.h"

Level::Level(int depth, bool isLeaf, int size) {
    this->depth = depth;
    this->isLeaf = isLeaf;
    // If is a leave level then allocate the map array
    if (this->isLeaf) {
        this->mapArr = new Map*[size];
        for (int i = 0; i < size; i++) {
            mapArr[i] = nullptr;
        }
    } else { // Else allocate the level array
        this->nextLevelPtr = new Level*[size];
        for (int i = 0; i < size; i++) {
            nextLevelPtr[i] = nullptr;
        }
    }
}
