/*************************************************************************
Name:	    Isabell Jansson, Jonathan Bosson, Ronja Grosz
SPH: 	    SPH.h

SPH is responsible for orginization of a group of smooth particles.
*************************************************************************/

#ifndef SPH_H
#define SPH_H

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#ifdef __linux__
#include "../glm/glm/glm.hpp"
#include "../glm/glm/gtc/type_ptr.hpp"
#elif __APPLE__
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"	
#endif

#include "boost/timer.hpp"
#include <vector>
#include "../particle/Particle.h"
#include "CellList.h"

using namespace boost;


class SPH
{
	protected:
		int				particleCount;	
		timer			*frameTimer;	//this tracks the amount of time between frames for reasonable rendering
		double			timeLastFrame;	//the time that the last frame was rendered at
		double			dT;

		vector <Particle*> 	*water;	//this is my vector full of particles
		GLuint vao, vbo[2];		// handles to vao and abo
		//GLfloat *vertices[particleCount][3];	// pointer to all vertices

		// particle properties
		double 	particleRadius;
		double 	particleMass;
		double 	H;
		double 	particleViscosity;
		double  maxTimestep;
		double  iterations;
		double  constantAcceleration;
		std::string sceneName;

		double getRadius();
		double getMass();
		double getViscosity();

		void setRadius(double);
		void setMass(double);
		void setViscosity(double);

		void loadJson(std::string);
		void createVAO(int particles);
		void calculateNonPressureForces();
		void predictVelocities();
		bool isSolid(double,double,double,int);
		void adaptTimestep(double timeDiff);
		void calculateDensity();	
		void calculateAlpha();
		void simulate(double timeDiff);	//gets neighboring particels and calls their getForceAtPoint, applyForce...
		
	public:
		SPH();
		SPH(const SPH&);
		~SPH();

		void display();
		void setTimer(timer*);

};

#endif
