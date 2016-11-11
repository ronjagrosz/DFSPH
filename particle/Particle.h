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

struct BindingPoint
{
	vector <double> points;
//	vector <SmoothedParticle*> bound;

};


class Particle
{
	protected:
		
		vector<int>	*neighbours;	

		dvec3 	position;
		dvec3 	velocity;
		dvec3   force;
		vec3	color;	//the color of the smooth particle
        glm::ivec3  cellIndex;
		
		// properties
		double 	density;
		double  alpha; // for kernelfunction (ai)
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
        void setCellIndex(glm::ivec3 cell);
		void setDensity(double);
		void setAlpha(double);
		

		//getters
		dvec3 getPosition();
		dvec3 getVelocity();
		dvec3 getForce();
		vec3 getColor();
        glm::ivec3 getCellIndex();
        vector<int>* getNeighbours();
		double getDensity();
		double getAlpha();
		double getStiffness();
		
		
		//void correctDensityError();
		void updatePosition(double elapsedTime);	// apply the velocity to the position
		void updateNeighbours(vector<int>* neighbourList);
		//void correctDivergenceError();
		//void updateVelocity(); // is this one needed?


		// Can be removed after clean up
		virtual	double kernel(dvec3, double);
		virtual	dvec3 gradientKernel(dvec3, double);
		
		//virtual void calculateDensity(Particle*);		//used to calculate the pressure force
		
		inline void printDensity(){cout << "density = " << density << " " << std::isnan(density) << endl;};

		void clearNAN()	//a very kludgey solution to a nan problem I was having in the density calculation.
		{			//finding the root cause of this is on my list of things to do.
			if(std::isnan(density))
                glm::ivec3 getCellIndex();
				density = 0;
		};
};

#endif













