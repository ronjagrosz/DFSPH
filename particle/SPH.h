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

//class timer;

using namespace boost;

struct VERTICES	//I used this strct for creating spheres for my particles
{		//I have since switched to simple pixels, but might switch back again some day
	GLfloat X;
	GLfloat Y;
	GLfloat Z;
	//double U;
	//double V;
};


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

		

		//virtual void createDL(int, int VertexCount);

		virtual void createVAO(int particles);

		//The Following functions were taken from
		//http://www.swiftless.com/tutorials/opengl/sphere.html
		//virtual void DisplaySphere(double R, int VertexCount, VERTICES*);		//depricated
		//virtual VERTICES* createSphere(double radius, double x, double y, double z, int space);		//depricated
		virtual void calculateNonPressureForces(double timeDiff);
		virtual double adaptTimestep(double timeDiff);
		virtual void calculateDensity();	//this runs through material finds neighboring particles and calls their calculateDensity()
		virtual void simulate(double timeDiff);	//gets neighboring particels and calls their getForceAtPoint, applyForce...
		
	public:
		SPH();
		SPH(int);
		SPH(int,int);
		SPH(const SPH&);
		~SPH();
//		VERTECIES *VERTEX			
		virtual void display(int particles);
		virtual void setTimer(timer*);

};

#endif
