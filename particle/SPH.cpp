/*************************************************************************
Name:       Isabell Jansson, Jonathan Bosson, Ronja Grosz	
File name: 	SPH.cpp

SPH is responsible for orginization of a group of smooth particles.
*************************************************************************/

#include <string.h>
#include <cstdlib>
#include <stdio.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>

#include "../json/picojson.h"
#include "../particle/SPH.h"

#define EPSILON 0.0000000001f

using namespace glm;

bool compareZ(Particle* left, Particle* right)
{
	return (left->getPosition().z < right->getPosition().z);
}

SPH::SPH()
{
	frameTimer = new timer;

	// load json parameters
	loadJson("json/scene_parameters.json");
	/*
	particleCount = 200000;
	maxTimestep = 0.002;
	iterations = 15;
	constantAcceleration = -9.81;

	particleRadius = 1;
	particleMass = 5;
	particleViscosity = 2.034;
	*/

	GLfloat randX, randY, randZ;
	vec3 newColor = vec3(1.0f,1.0f,1.0f);
	double randI, randJ, randK; //velocity vector values
	
	srand(time(0));

	water = new vector<Particle*>(particleCount);

	for(int i = 0; i < particleCount; ++i) {

		// random position
		randX = ((float)rand()/(float)RAND_MAX) * 1.0 - 0.5;
		randY = ((float)rand()/(float)RAND_MAX) * 1.0 - 0.5 + 1.6;
		randZ = ((float)rand()/(float)RAND_MAX) * 1.0 - 0.5;
		

		// random velocity
		randI = (((double)rand()/(double)RAND_MAX) * 0.2) - 0.1;
		randJ = (((double)rand()/(double)RAND_MAX) * 0.2) - 0.1;
		randK = (((double)rand()/(double)RAND_MAX) * 0.2) - 0.1;
		
		water->at(i) = new Particle();
		water->at(i)->setPosition(randX, randY, randZ);
		water->at(i)->setColor(newColor);
		water->at(i)->setVelocity(dvec3(randI, randJ, randK));

		// find neighborhood
		// compute densities
		// compute ai

	}

	createVAO(particleCount);
	timeLastFrame = frameTimer->elapsed();
}


SPH::~SPH()
{
	for(int i = 0; i < particleCount; i++)
	{
		delete water->at(i);
	}
	delete water;
	delete frameTimer;

}

void SPH::setRadius(double rad) {
	particleRadius = rad;
}
void SPH::setMass(double mass) {
	particleMass = mass;
}
void SPH::setViscosity(double vis) {
	particleViscosity = vis;
}
double SPH::getRadius(){return particleRadius;}
double SPH::getMass(){return particleMass;}
double SPH::getViscosity(){return particleViscosity;}


void SPH::loadJson(string fileName)
{
	picojson::value params;
    ifstream paramStream (fileName);
    paramStream >> params;
    particleCount = (int)(params.get<picojson::object>()["particlesCount"].get<double>());
    maxTimestep = params.get<picojson::object>()["maxTimestep"].get<double>();
    iterations = (int)(params.get<picojson::object>()["iterations"].get<double>());
    constantAcceleration = params.get<picojson::object>()["constantAcceleration"].get<double>();

    particleRadius = params.get<picojson::object>()["particleRadius"].get<double>();
    particleMass = params.get<picojson::object>()["particleMass"].get<double>();
    particleViscosity = params.get<picojson::object>()["particleViscosity"].get<double>();
}


// Update positions with a small timestep
void SPH::simulate(double timeDiff)
{
	calculateNonPressureForces(timeDiff); // if only gravity timeDiff is not needed

	// adapt timestep according to CFL condition
	dT = adaptTimestep(timeDiff);

	// predict velocities
	for (int i = 0; i < particleCount; ++i) 
	{
		water->at(i)->predictVelocity(dT);
	}
	// correctDensityError

	// update positions
	for (int i = 0; i < particleCount; ++i)
	{
		water->at(i)->updatePosition(dT);
	}

	// update neighborhoods

	// compute densities and ai factors

	// correctDivergenceError

	// update velocities
	
}

// Compute non-pressure forces - advect, body forces (gravity) and viscosity
void SPH::calculateNonPressureForces(double timeDiff)
{
	
	//dvec3 *particleVel;
	//dvec3 *neighborVel;
	//dvec3 particlePos;
	//dvec3 neighborPos;
	double steps = 6;
	
	for(int i = 0; i < particleCount; ++i)
	{
		dvec3 currentPos = water->at(i)->getPosition();
		dvec3 currentVel = water->at(i)->getVelocity();

		// Gravity
		water->at(i)->setForce(0.0, particleMass * constantAcceleration, 0.0);

		// Self-Advection - Need velocity at other position
		for(int step = 0; step < steps; ++step) {
			// Get direction.
			// we are tracing back in time, hence -=, scale dt to number of steps.
			currentPos -= currentVel * timeDiff / steps / particleRadius;

			// get vel at other position
			currentVel = water->at(i)->getVelocity();
		}
		water->at(i)->setVelocity(currentVel);

		/*
		// Surface tension (according to [3])
        float correctionFactor = 2.f * _restDensity / (density_i + density_j);
        forceCohesion += correctionFactor * (r / rn) * _kernel.surfaceTension(rn);
        forceCurvature += correctionFactor * (n_i - n_j);
		*/

		// Viscosity (later)
		/*
		for(iterator of neighbor)
		{
			neighborPos = water->at(neighbors.at(j))->getPosition();
			neighborVel = water->at(neighbors.at(j))->getVelocity();

			if (density_j > 0.0001f) {
                viscosity -= (currentVel - neighborVel) * (_kernel.viscosityLaplace(rn) / density_j);
            }
		}
		*/
	}
}

double SPH::adaptTimestep(double timeDiff)
{
	dvec3 val;
	double mag, dT, vMax = 0.0;

	for (int i = 0; i < particleCount; ++i)
	{
		val = water->at(i)->getForce();
		mag = dot(val, val);

		if (vMax < mag)
			vMax = mag;
	}
	dT = particleRadius * 0.8 / vMax - EPSILON;

	if (timeDiff < dT)
		dT = timeDiff;

	return dT;
}

//calculate density function
void SPH::calculateDensity()
{
	double distance = 0;
	
	glm::vec4 *particleVel;
	glm::vec4 *neighborVel;

	

	for(int i = 0; i < particleCount; i++)
	{
		dvec3 particlePos = water->at(i)->getPosition();
		for(int j = 0; j < particleCount; j++)
		{
			dvec3 neighborPos = water->at(j)->getPosition();
			//if(primaryPositionVector && secondaryPositionVector)
			//{
				distance = dot(particlePos - neighborPos, particlePos - neighborPos);
			//}
				
			if(distance <= H*H)
			{
				//if(primaryPositionVector && secondaryPositionVector)
				//{
					water->at(i)->calculateDensity(water->at(j));			
				//}
	
				//delete secondaryPositionVector;
				delete particleVel;
				delete neighborVel;
			} else 
			{
				//delete secondaryPositionVector;
	
				break;
			}
				
		}
		//delete primaryPositionVector;
	}
	for(int i = 0; i<particleCount; i++)
	{
		water->at(i)->clearNAN();
//		water->at(i)->printDensity();
	}

}


void SPH::display()	
{	

	GLfloat vertices[particleCount][3];
	GLfloat colors[particleCount][3];

	//this is used to log the elapsed time since the last frame
	double timeDiff = frameTimer->elapsed() - timeLastFrame;
	if (timeDiff - EPSILON > 0.0) {
		double t = 0.0;
		int iter = 0;
		// Call simulate - how many timesteps per frame?
		while (t < maxTimestep && iter < iterations) {
			simulate(timeDiff);
			t += dT;
			iter++;
		}

		cout << "Iterations: " << iter << "\n";
		// Render stuff
		for(int i = 0; i < particleCount; ++i) {
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
	    glBufferData(GL_ARRAY_BUFFER, 3 * particleCount * sizeof(GLfloat), vertices,/* 9 * sizeof(GLfloat), diamond, */ GL_STATIC_DRAW);

	    // Specify that our coordinate data is going into attribute index 0, and contains three floats per vertex 
	    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	    // Enable attribute index 0 as being used 
	    glEnableVertexAttribArray(0);

	    // Bind the second VBO as being the active buffer and storing vertex attributes (colors)
	    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);

	    glBufferData(GL_ARRAY_BUFFER, 3 * particleCount * sizeof(GLfloat), colors, GL_STATIC_DRAW);

	    // Specify that our color data is going into attribute index 1, and contains three floats per vertex 
	    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	    // Enable attribute index 1 as being used
	    glEnableVertexAttribArray(1);
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

	glDrawArrays(GL_POINTS, 0, particleCount);
	timeLastFrame = frameTimer->elapsed();
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
