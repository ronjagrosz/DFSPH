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
		
		vector<int>	*neighbors;	

		dvec3 	position;
		dvec3 	velocity;
		dvec3   force;
		vec3	color;	//the color of the smooth particle
		
		// properties
		double 	density;
		double  alpha; // for kernelfunction (ai)
		double  stiffness; // k variable in report

	public:
		Particle();
		//Particle(const Particle&);
		~Particle();
		
		//setters
		virtual void setPosition(float,float,float);
		virtual void setVelocity(dvec3);
		virtual void setForce(double, double, double);
		virtual void setColor(vec3 newColor);
		virtual void setDensity(double);
		virtual void setAlpha(double);
		

		//getters
		virtual dvec3 getPosition();
		virtual dvec3 getVelocity();
		virtual dvec3 getForce();
		virtual vec3 getColor();
		virtual double getDensity();
		virtual double getAlpha();
		virtual double getStiffness();
		
		
		//virtual void correctDensityError();
		virtual void updatePosition(double elapsedTime);	// apply the velocity to the position
		//virtual void updateNeighborhoods();
		//virtual void correctDivergenceError();
		//virtual void updateVelocity(); // is this one needed?


		// Can be removed after clean up
		virtual	double kernel(dvec3, double);
		virtual	dvec3 gradientKernel(dvec3, double);
		
		//virtual void calculateDensity(Particle*);		//used to calculate the pressure force
		
		virtual inline void printDensity(){cout << "density = " << density << " " << std::isnan(density) << endl;};

		virtual void clearNAN()	//a very kludgey solution to a nan problem I was having in the density calculation.
		{			//finding the root cause of this is on my list of things to do.
			if(std::isnan(density))
				density = 0;
		};
};

#endif













