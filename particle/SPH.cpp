/*************************************************************************
Name:       Isabell Jansson, Jonathan Bosson, Ronja Grosz	
File name: 	SPH.cpp

SPH is responsible for orginization of a group of smooth particles.
*************************************************************************/

#include <string.h>
#include <cstdlib>
#include <stdio.h>

#include <iostream>
#include <vector>
#include <stack>
#include <algorithm>
#include <ctime>

#include "../particle/SPH.h"

#define GRAVITY 9.81

using namespace glm;

//#define VISIBLE_TEST  //this tells the program to only make 5 particles in a horizontal line.


bool compareZ(Particle* left, Particle* right)
{
	return (left->getPosition().z < right->getPosition().z);
}

SPH::SPH()
{
	frameTimer = new timer;
	timeLastFrame = frameTimer->elapsed();
}

SPH::SPH(int particles)
{
	frameTimer = new timer;

	particleCount = particles;
	GLfloat randX, randY, randZ;
	vec3 newColor = vec3(1.0f,1.0f,1.0f);
	double randI, randJ, randK; //velocity vector values
	
	srand(time(0));

	water = new vector<Particle*>(particleCount);

	for(int i = 0; i < particleCount; i++) {

		// random position
		randX = ((float)rand()/(float)RAND_MAX) * 1.0 - 0.5;
		randY = ((float)rand()/(float)RAND_MAX) * 1.0 - 0.5;
		randZ = ((float)rand()/(float)RAND_MAX) * 1.0 - 0.5;
		

		// random velocity
		randI = (((double)rand()/(double)RAND_MAX) * 0.2) - 0.1;
		randJ = (((double)rand()/(double)RAND_MAX) * 0.2) - 0.1;
		randK = (((double)rand()/(double)RAND_MAX) * 0.2) - 0.1;
		
		water->at(i) = new Particle();
		water->at(i)->setPosition(randX, randY, randZ);
		water->at(i)->setColor(newColor);
		water->at(i)->setVelocity(randI, randJ, randK);
		water->at(i)->setMass(5);

		// find neighborhood

		// compute densities
		calculateDensity();

		// compute ai

	}

	createVAO(particleCount);
	frameTimer = new timer;
	timeLastFrame = frameTimer->elapsed();
}


SPH::~SPH()
{
	for(int i = 0; i < particleCount; i++)
	{
		delete water->at(i);
	}
	delete water;
	delete dls;
	delete frameTimer;

}

//this is one of the most important functions in the program
void SPH::simulate(double timeDiff)
{

	// compute non pressure forces - advect, body forces (gravity), (and viscosity)
	/*
	dvec3 *particleVel;
	dvec3 *neighborVel;
	dvec3 particlePos;
	dvec3 neighborPos;

	for(int i = 0; i < particleCount; ++i)
	{
		//primary is the particle we will be comparing the rest to
		particlePos = water->at(i)->getPosition();
		for(iterator of neighbor)
		{
			//secondary is a particle down the line	
			neighborPos = water->at(neighbors.at(j))->getPosition();
			
			//get the forces that the two particles enact on each other
			particleVel = water->at(i)->calculateForces(water->at(neighbors.at(j)));
			neighborVel = water->at(neighbors.at(j))->calculateForces(water->at(i));
		
			delete particleVel;
			delete neighborVel;
		}
		
		//delete primaryPositionVector;
	}
	*/

	// adapt timestep according to CFL condition
	// deltaT <= 0.4*2*radius/(Vmax)

	// predict velocities
	for (int i = 0; i < particleCount; ++i) 
	{
		water->at(i)->predictVelocity(timeDiff);
	}
	// correctDensityError

	// update positions
	for (int i = 0; i < particleCount; ++i)
	{
		water->at(i)->updatePosition(timeDiff);
	}

	// update neighborhoods

	// compute densities (since positions have been changed)
	calculateDensity();

	// compute ai factors (need to be updated since densities have been changed)
	// maybe change so it's not a function in SPH, just call calculateAlpha for a particle...
	//calculateAlpha();
	
	// correctDivergenceError

	// update velocities
	
}

//this is effectively the same algorith as above, but instead of 
//getting and applying force it just calls the particle's 
//calculate density function.

void SPH::calculateDensity()
{	
	// for all particles
	for(int i = 0; i < particleCount; i++) {
		// set density for particle i to zero? reuse this function in simulate?
		water->at(i)->setDensity(0.0);
		/*
		//calculate density in the area around particle i
		for(all neightbours)
			water->at(i)->calculateDensity(neightbour);				
	
		*/
	}
	/*for(int i = 0; i<particleCount; i++)
	{
		water->at(i)->clearNAN();
//		water->at(i)->printDensity();
	}*/

}

/*void SPH::calculateAlpha()
{
	for(int i = 0; i < particleCount; i++) {
		//calc alpha
	}	
}*/


void SPH::display(int particles)	
{	

	GLfloat vertices[particles][3];
	GLfloat colors[particles][3];

	//this is used to log the elapsed time since the last frame
	double currentTime = frameTimer->elapsed();

	if ((currentTime - timeLastFrame) > 0) {

		// Call simulate - how many timesteps per frame?
		

		// Render stuff
		for(int i = 0; i < particles; i++) {
			vertices[i][0] = water->at(i)->getPosition().x;
			vertices[i][1] = water->at(i)->getPosition().y;
			vertices[i][2] = water->at(i)->getPosition().z;
			
			//tmp = water->at(i)->getColor();
			colors[i][0] = 0.7 - water->at(i)->getPosition().x;
			colors[i][1] = 0.7 - water->at(i)->getPosition().y;
			colors[i][2] = 0.7 - water->at(i)->getPosition().z;
			
		}


		// Bind the first VBO as being the active buffer and storing vertex attributes (coordinates)
	    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);

		// Copy the vertex data from diamond to our buffer 
	    // 8 * sizeof(GLfloat) is the size of the diamond array, since it contains 8 GLfloat values 
	    glBufferData(GL_ARRAY_BUFFER, 3 * particles * sizeof(GLfloat), vertices,/* 9 * sizeof(GLfloat), diamond, */ GL_STATIC_DRAW);

	    // Specify that our coordinate data is going into attribute index 0, and contains three floats per vertex 
	    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	    // Enable attribute index 0 as being used 
	    glEnableVertexAttribArray(0);

	    // Bind the second VBO as being the active buffer and storing vertex attributes (colors)
	    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);

	    glBufferData(GL_ARRAY_BUFFER, 3 * particles * sizeof(GLfloat), colors, GL_STATIC_DRAW);

	    // Specify that our color data is going into attribute index 1, and contains three floats per vertex 
	    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	    // Enable attribute index 1 as being used
	    glEnableVertexAttribArray(1);


		//update the time.
		timeLastFrame = frameTimer->elapsed();
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // make background black
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(vao);

	glEnable(GL_PROGRAM_POINT_SIZE); //enable gl_PointSize in vertex shader
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

	glDrawArrays(GL_POINTS, 0, particles);
}

void SPH::createVAO ( int particles ) {
	// A VAO (Vertex Array Object) stores information of a complete rendered object.
	// It contains all VBOs (Vertex Buffer Objects)
	// A VBO stores information about the vertices. 
	// Now we're using two VBOs, one for coordinates and one for colors
	

	// Allocate and bind Vertex Array Object to the handle vao
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Generate (one) new Vertex Buffer Object and get the associated id
	glGenBuffers(2, vbo);
}

void SPH::setTimer(timer *newTimer)
{
	frameTimer = newTimer;
}
