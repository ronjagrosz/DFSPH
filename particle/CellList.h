/*************************************************************************
Name:       Isabell Jansson, Jonathan Bosson, Ronja Grosz
File name:  CellList.h

This class contains the data structure for a cell list to make accessing
neighbouring particles faster.
*************************************************************************/

#include <iostream>
#include <vector>

#ifdef __linux__
#include "../glm/glm/glm.hpp"
#include "../glm/glm/gtc/type_ptr.hpp"
#elif __APPLE__
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#endif

#include "Particle.h"

using namespace std;
using namespace glm;

// g is the coordinate of the loweLeft corner in the scene.
// b is the coordinate of the "highestPoint" in the scene.
// Every point in the scene has to be inside the cube.
//                  y
//                  ^
//                  |              
//                  e-------f     
//                 /|      /|     
//                / |     / |     
//               a--|----b  |     
//     (0, 0, 0) |  g----|--h ---> x
//               | /     | /     
//               c-------d      
//               /              
//              z

class CellList
{
    public:
        CellList(glm::dvec3 lowestPoint, glm::dvec3 highestPoint, double H);
        ~CellList();

        void addParticle(Particle* particle, int pIndex);
        void moveParticle(Particle* particle, int pIndex);
        vector<int>* findNeighbours(vector<Particle*> *water, int pIndex);

    protected:
        vector<vector<vector<vector<int>>>> cellList;
        glm::dvec3 lowerLeft;
        const double radius;

        glm::ivec3 getCellPos(glm::dvec3 pos);
        bool validCellPos(glm::ivec3 pos);
};
