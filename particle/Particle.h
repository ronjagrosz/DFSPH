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
#elif __APPLE__
#include "glm/glm.hpp"
#endif

#define WATER 0

using namespace std;
using namespace boost;
using namespace glm;


const double H = .005; // Cutoff radius


struct BindingPoint
{
	vector <double> points;
//	vector <SmoothedParticle*> bound;

};


class Particle
{
	protected:
		
		vector<int>	*neighbors;	

		dvec3 	position;
		dvec3 	velocity;
		dvec3   force;
		vec3	color;	//the color of the smooth particle
        glm::ivec4  cellIndex;
		
		// properties
		double 	density;
		double  A; // for kernelfunction (ai)
		double  stiffness; // k variable in report

	public:
		Particle();
		//Particle(const Particle&);
		~Particle();
		
		//setters
		void setPosition(float,float,float);
		void setVelocity(dvec3);
		void setForce(double, double, double);
		void setColor(vec3 newColor);
        void setCellIndex(glm::ivec4 cell);
		void setDensity(double);
		

		//getters
		dvec3 getPosition();
		dvec3 getVelocity();
		dvec3 getForce();
		vec3 getColor();
        glm::ivec4 getCellIndex();
		double getDensity();
		double getStiffness();
		
		
		//void correctDensityError();
		void updatePosition(double elapsedTime);	// apply the velocity to the position
		//void updateNeighborhoods();
		//void calculateDensityA();
		//void correctDivergenceError();
		//void updateVelocity(); // is this one needed?


		// Can be removed after clean up
		dvec3* pressureKernel(dvec3);	//smoothing kernel functions used in the getForceAtPoint function
		dvec3* viscosityKernel(dvec3);
		double densityKernel(dvec3);
		
		void calculateDensity(Particle*);		//used to calculate the pressure force
		
		inline void printDensity(){cout << "density = " << density << " " << std::isnan(density) << endl;};

		void clearNAN()	//a very kludgey solution to a nan problem I was having in the density calculation.
		{			//finding the root cause of this is on my list of things to do.
			if(std::isnan(density))
                glm::ivec4 getCellIndex();
				density = 0;
		};
};

#endif













