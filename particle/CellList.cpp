/*************************************************************************
Name:       Isabell Jansson, Jonathan Bosson, Ronja Grosz
File name:  CellList.cpp

This class contains the data structure for a cell list to make accessing
neighbouring particles faster.
*************************************************************************/

#include "CellList.h"

CellList::CellList(glm::dvec3 lowestPoint, glm::dvec3 highestPoint, const double H, vector<Particle*> *water) : lowerLeft(lowestPoint), radius(H)
{
    // Amount of cubes in every direction
    glm::ivec3 noOfCubes = glm::ceil((highestPoint - lowerLeft) / radius) + 1.0;

    // Set up the cell list.
    cellList.resize(noOfCubes.x);
    for (int i = 0; i < noOfCubes.x; ++i)
    {
        cellList[i].resize(noOfCubes.y);
        for (int j = 0; j < noOfCubes.y; ++j)
        {
            cellList[i][j].resize(noOfCubes.z);
        }
    }

    for (int i = 0; i < water->size(); i++)
    {
        // Get which cell the particle belongs to
        glm::ivec3 cell = getCellPos(water->at(i)->getPosition());

        // Add particles index to cell
        cellList[cell.x][cell.y][cell.z].push_back(i);

        // Add paticles place in cellList to particle to allow for easy access?
    }
}

vector<int>* CellList::findNeighbours(int pInd, vector<Particle*> *water)
{
    vector<int>* neighbourList = new vector<int>;
    glm::ivec3 cell = getCellPos(water->at(pInd)->getPosition());

    for (int x = -1; x < 2; x++)
    {
        for (int y = -1; y < 2; y++)
        {
            for (int z = -1; z < 2; z++)
            {
                // Get neighbouring cells position
                glm::ivec3 pos = glm::ivec3(cell.x + x, cell.y + y, cell.z +z);

                if (!validCellPos(pos))
                    continue;

                // Iterate through potential neighbours
                for(vector<int>::iterator it = cellList[pos.x][pos.y][pos.z].begin(); it != cellList[pos.x][pos.y][pos.z].end(); ++it)
                {
                    if (pInd != *it && glm::length(water->at(pInd)->getPosition() - water->at(*it)->getPosition()) < H)
                        neighbourList->push_back(*it);
                }

            }
        }
    }
    return neighbourList; 
}

glm::ivec3 CellList::getCellPos(glm::dvec3 pos)
{
    // Get which cell the particle belongs to
    glm::ivec3 cell = glm::ceil(pos - lowerLeft) / radius;

    // Validate that the cell position is correct
    if (cell.x < 0 || cell.y < 0 || cell.z < 0)
        cout << "Particle is out of bounds in cell list, move the bounds of the cell list to encapsulate the whole scene\n";
    return cell;
}

bool CellList::validCellPos(glm::ivec3 pos)
{
    // Return true if position is inside the boundaries
    return pos.x > 0 && 
           pos.y > 0 && 
           pos.z > 0 && 
           pos.x < cellList.size() && 
           pos.y < cellList[0].size() && 
           pos.z < cellList[0][0].size();
}
