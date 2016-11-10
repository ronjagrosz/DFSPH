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
//#include "../render/TriangleSoup.h"

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
		double 	particleViscosity;
		double  maxTimestep;
		double  iterations;
		double  constantAcceleration;
		std::string sceneName;

		//TriangleSoup* scene;

		


		virtual double getRadius();
		virtual double getMass();
		virtual double getViscosity();

		virtual void setRadius(double);
		virtual void setMass(double);
		virtual void setViscosity(double);

		virtual void loadJson(std::string);
		virtual void createVAO(int particles);
		virtual void predictVelocities();
		virtual bool isSolid(double,double,double,int);
		virtual void adaptTimestep(double timeDiff);
		virtual void calculateDensity();	//this runs through material finds neighboring particles and calls their calculateDensity()
		virtual void simulate(double timeDiff);	//gets neighboring particels and calls their getForceAtPoint, applyForce...
		
	public:
		SPH();
		SPH(const SPH&);
		~SPH();

		virtual void display();
		virtual void setTimer(timer*);

};

#endif
