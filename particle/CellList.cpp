/*************************************************************************
Name:       Isabell Jansson, Jonathan Bosson, Ronja Grosz
File name:  CellList.cpp

This class contains the data structure for a cell list to make accessing
neighbouring particles faster.
*************************************************************************/

#include "CellList.h"

CellList::CellList(dvec3 lowestPoint, dvec3 highestPoint, double H) : lowerLeft(lowestPoint), radius(H) {
    // Amount of cubes in every direction
    ivec3 noOfCubes = ceil((highestPoint - lowerLeft) / radius) + 1.0;
    if (noOfCubes.x < 0 || noOfCubes.y < 0 || noOfCubes.z < 0) {
        cout << "Radius or dimension of cellList is wrong";
    }

    // Set up the cell list.
    cellList.resize(noOfCubes.x);
    for (int i = 0; i < noOfCubes.x; ++i) {
        cellList[i].resize(noOfCubes.y);
        for (int j = 0; j < noOfCubes.y; ++j) {
            cellList[i][j].resize(noOfCubes.z);
        }
    }
}

// Add a particle to the cellList structure
void CellList::addParticle(Particle* particle, int pIndex) {
    // Get which cell the particle belongs to
    ivec3 cell = getCellPos(particle->getPosition());

    // Validate new cell position
    if (!validCellPos(cell))
        cout << "Particle is out of bounds in cell list, move the bounds of the cell list to encapsulate the whole scene\n";

    // Add particles index to cell
    cellList[cell.x][cell.y][cell.z].push_back(pIndex);

    // Add cell index to particle
    particle->setCellIndex(ivec4(cell, cellList[cell.x][cell.y][cell.z].size() - 1));
}

// Move the particle from one cell to another if it has moved
void CellList::moveParticle(Particle* particle, int pIndex) {
    // Validate that particles position is ok
    if (std::isnan(particle->getPosition().x) ||
        std::isnan(particle->getPosition().y) ||
        std::isnan(particle->getPosition().z)) {
        cout << "Position for particle is NaN\n";
        return;
    }

    // Get new and old cell for particle
    ivec3 newCell = getCellPos(particle->getPosition());
    ivec4 oldCell = particle->getCellIndex();
    vector< int >::iterator it;

    // Move particle if it has left the old cell and the new cell is valid
    if ((newCell.x != oldCell.x || newCell.y != oldCell.y || newCell.z != oldCell.z) && validCellPos(newCell)) {
        // Find particle in old cell
        // Check if the old cell w coord is larger than the amount of particles in cell
        if (oldCell.w > cellList[oldCell.x][oldCell.y][oldCell.z].size() - 1) {
            it = cellList[oldCell.x][oldCell.y][oldCell.z].end();
            it--;
        } else { // Set iterator at the predicted cell index
             it = cellList[oldCell.x][oldCell.y][oldCell.z].begin() + oldCell.w;
        }
        // Iterate to the position where the index actually is
        for (; it != cellList[oldCell.x][oldCell.y][oldCell.z].begin(); --it) {
            if (*it == pIndex)
                break;
        }
        // Validate that the iterator actually is at the particle
        if (*it != pIndex)
            cout << "Didn't find particle in cell in moveParticle\n";

        // Insert into new cell
        cellList[newCell.x][newCell.y][newCell.z].push_back(pIndex);

        // Remove old cell
        cellList[oldCell.x][oldCell.y][oldCell.z].erase(it);

        // Update particles cell index
        particle->setCellIndex(ivec4(newCell, cellList[newCell.x][newCell.y][newCell.z].size() - 1));
    } 
}

// Find the particles actual neighbours in the neighbouring cells
vector<int>* CellList::findNeighbours(vector<Particle*> *water, int pIndex) {
    vector< int >* neighbourList = new vector< int >;
    if (std::isnan(water->at(pIndex)->getPosition().x) ||
        std::isnan(water->at(pIndex)->getPosition().y) ||
        std::isnan(water->at(pIndex)->getPosition().z)) {
        cout << "Position for particle is NaN\n";
    }
    ivec3 cell = water->at(pIndex)->getCellIndex();

    for (int x = -1; x < 2; x++) { 
        for (int y = -1; y < 2; y++) {
            for (int z = -1; z < 2; z++) {
                // Get neighbouring cells position
                ivec3 pos = ivec3(cell.x + x, cell.y + y, cell.z + z);

                if (!validCellPos(pos))
                    continue;

                // Iterate through potential neighbours
                for(vector<int>::iterator it = cellList[pos.x][pos.y][pos.z].begin(); it != cellList[pos.x][pos.y][pos.z].end(); ++it) {
                    if (length(water->at(pIndex)->getPosition() 
                        - water->at(*it)->getPosition()) <= radius)
                        neighbourList->push_back(*it);
                }
            }
        }
    }
    return neighbourList; 
}

// Get which cell the particle belongs to
ivec3 CellList::getCellPos(dvec3 pos) {
    // Return which cell the particle belongs to
    return ceil(pos - lowerLeft) / radius;
}

// Validate that the cell position is in bounds
bool CellList::validCellPos(ivec3 pos) {
    // Return true if position is inside the boundaries
    return  !std::isnan(pos.x) &&
            !std::isnan(pos.y) &&
            !std::isnan(pos.z) &&
            pos.x >= 0 && 
            pos.y >= 0 && 
            pos.z >= 0 && 
            pos.x < cellList.size() && 
            pos.y < cellList[0].size() && 
            pos.z < cellList[0][0].size();
}
