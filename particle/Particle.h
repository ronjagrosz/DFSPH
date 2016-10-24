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
		
			
		//variables that are needed for basic functioning
		//physical properties
		vec3 	position;
		stack	 <int>		*neighbors;

		glm::vec4 	*velocity;
		double 	radius;
		double 	mass;
		double 	viscosity;
		int 	materialID;	//this will later be used to tell different fluids apart (eg. oil and water)
		timer	*frameTimer;
		double 	timeLastFrame;
		double 	forceConstant;
		double 	density;	

		double threshold;	//thresHold distance between attractive and repulsive
		double stretchR;	//stretctes the attractive force curve
		double stretchA;	//stretches the repulsive force curve
		double offsetR;		//the x-axis translation for the repulsive curve
		double offsetA;		//the x-axis translation for the attractive curve
		double maxR;		//Maximum scalar value for the repulsive force (x^2)
		double maxA;		//maximum scalar value for the attractive force (x^2)


		GLuint	DL;		//this is the pointer to the Particle's display list

		//The color and shape
		vec3	color;	//the color of the smooth particle
		float	pressureScale;	//the rate at which the particle turns color
//		GLuint	sphereDL;

		//These variables are used for ray tracing
//		double alpha;
//		double indexOfRefraction;
//		double reflectivity;

		//Thes variables are used for Crystalization
//		vector <BindingPoints> 	boundParticles;
//		vector <double		angularVelocity;
//		glm::vec4	orientation;
//		bool	isBound;


	public:
		Particle();
		//Particle(const Particle&);
		~Particle();
		
		//setters
		virtual void setPosition(float,float,float);
		virtual void setVelocity(double,double,double);
		virtual void setRadius(double);
		virtual void setMass(double);
		virtual void setMaterialID(double);
		virtual void setColor(vec3 newColor);
		virtual void setPressureScale(float);
		virtual void setDL(GLuint);
		virtual void setTimer(timer *currentTime);

		//getters
		virtual vec3 getPosition();
		virtual vec4* getVelocity();
		virtual double getRadius();
		virtual double getMass();
		virtual double getMaterialID();
		virtual vec3 getColor();
		virtual float getPressurescale();
		virtual GLuint getDL();
		
		virtual void display(double);
		virtual vec4* calculateForces(Particle*);	//this is the biggest deal in this program
		virtual void predictVelocity(glm::vec4 &, double);		//apply the forces to the velocity
		virtual void updatePosition(double elapsedTime);	//apply the velocity to the position
		
		virtual vector <double>* pressureKernel(vector <double>*);	//smoothing kernel functions used in the getForceAtPoint function
		virtual vector <double>* viscosityKernel(vector <double>*);
		virtual	double densityKernel(vec3);
		
		virtual void calculateDensity(Particle*);		//used to calculate the pressure force
		

		virtual inline void zeroDensity(){density = mass/(radius*radius*M_PI);};	//this is used after the frame is over and the current density is no longer needed
		virtual inline void printDensity(){cout << "density = " << density << " " << std::isnan(density) << endl;};

		virtual void clearNAN()	//a very kludgey solution to a nan problem I was having in the density calculation.
		{			//finding the root cause of this is on my list of things to do.
			if(std::isnan(density))
				density = 0;
		};
};

#endif













