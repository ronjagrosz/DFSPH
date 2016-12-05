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
#include "../glm/glm/ext.hpp"
#elif __APPLE__
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/ext.hpp"	
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
		double			dT;
		double 			restDensity;
		double			avgDensityAdv;
		double 			avgDerivedDensity;

		vector <Particle*> 	*water;	//this is my vector full of particles
        CellList* cellList;
		GLuint vbo[2];		// handles to vao and abo
		
		// particle properties
		double 	particleRadius;
		double 	particleMass;
		double 	H;
		double  maxTimestep;
		double  iterations;
		double  constantAcceleration;
		double  maxError;
		double  maxErrorV;
		double  frameError;
		double  maxVelocity;

		dvec3 	gravity;

		std::string sceneName;
		vec4 geometry;

		void loadJson(std::string);

		void adaptTimestep();
		void predictVelocities();
		void boundaryCondition(int);
		dvec3 alongBoundary(dvec4);
		void calculateDensityChange();
		void correctDensityError();
		void calculateDensityAndAlpha();	
		void correctDivergenceError();
		void simulate();	//gets neighboring particels and calls their getForceAtPoint, applyForce...	
		
	public:
		SPH();
		~SPH();

		void display(float, float, GLuint);

};

#endif
