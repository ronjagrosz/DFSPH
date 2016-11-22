/*************************************************************************
Name:	    Isabell Jansson, Jonathan Bosson, Ronja Grosz
File name: 	Particle.h

The class Particle represents a single smooth particle.  It is responsible for
all calculations regarding that particle (eg. getting the force from this 
particle at a certain point.
*************************************************************************/

#ifndef SP_H
#define SP_H

#include <iostream>
#include <vector>
#include <stack>
#include "boost/timer.hpp"

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#ifdef __linux__
#include "../glm/glm/glm.hpp"
#include "../glm/glm/ext.hpp"	
#elif __APPLE__
#include "glm/glm.hpp"
#include "glm/ext.hpp"	
#endif

#define WATER 0

using namespace std;
using namespace boost;
using namespace glm;

struct BindingPoint {
	vector <double> points;
//	vector <SmoothedParticle*> bound;

};


class Particle {
	protected:
		
		vector<int>	*neighbours;	

		dvec3 	position;
		dvec3 	velocity;
        ivec4  cellIndex;
		
		// Properties
		double 	density;
		double  dDensity;
		double  alpha; // for kernelfunction (ai)

	public:
		Particle();
		//Particle(const Particle&);
		~Particle();
		
		// Setters
		void setPosition(float,float,float);
		void setVelocity(dvec3);
        void setCellIndex(ivec4 cell);
		void setDensity(double);
		void setdDensity(double);
		void setAlpha(double);

		// Getters
		dvec3 getPosition();
		dvec3 getVelocity();
        ivec4 getCellIndex();
        vector<int>* getNeighbours();
		double getDensity();
		double getdDensity();
		double getAlpha();
		
		void updatePosition(double elapsedTime);	// Apply the velocity to the position
		void updateNeighbours(vector<int>* neighbourList);
		//void updateVelocity(); // is this one needed?


		// Can be removed after clean up
		virtual	double kernel(dvec3, double);
		virtual	dvec3 gradientKernel(dvec3, double);
		
};

#endif
