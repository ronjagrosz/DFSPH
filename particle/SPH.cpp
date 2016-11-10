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

#define EPSILON 0.000000000000001f

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

	//scene.readOBJ(sceneName.c_str());
	//scene.printInfo();

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


	}

	for(int i = 0; i < particleCount; ++i) {
		// find neighborhood
		// compute densities
		calculateDensity();	

		//cout << "density: " << water->at(i)->getDensity()<< "\n";	

		// compute ai
		calculateAlpha();
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
    maxTimestep = 1/params.get<picojson::object>()["fps"].get<double>();
    iterations = (int)(params.get<picojson::object>()["iterations"].get<double>());
    constantAcceleration = params.get<picojson::object>()["constantAcceleration"].get<double>();
    sceneName = params.get<picojson::object>()["sceneName"].get<std::string>();


    particleRadius = params.get<picojson::object>()["particleRadius"].get<double>();
    particleMass = params.get<picojson::object>()["particleMass"].get<double>();
    H = params.get<picojson::object>()["H"].get<double>();
    particleViscosity = params.get<picojson::object>()["particleViscosity"].get<double>();
}

// Update positions with a small timestep
void SPH::simulate(double timeStep)
{
	// Self-advection - Skipped for SPH

	// Calculate gravity, surface tension and viscosity
	calculateNonPressureForces();

	// adapt timestep according to CFL condition
	dT = adaptTimestep(timeStep);

	// predict velocities
	predictVelocities(dT);

	// correctDensityError

	// update positions
	for (int i = 0; i < particleCount; ++i) {
		water->at(i)->updatePosition(dT);
	}

	// update neighborhoods

	// compute densities
	calculateDensity();

	// compute ai factors
	calculateAlpha();

	// correctDivergenceError

	// update velocities
	
}

// Compute non-pressure forces - advect, body forces (gravity) and viscosity
void SPH::calculateNonPressureForces()
{
	
	//dvec3 *particleVel;
	//dvec3 *neighborVel;
	//dvec3 particlePos;
	//dvec3 neighborPos;
	//double steps = 6;
	
	for(int i = 0; i < particleCount; ++i)
	{
		// Gravity
		water->at(i)->setForce(0.0, particleMass * constantAcceleration, 0.0);

		/*
		// Surface tension - skipped
		dvec3 currentPos = water->at(i)->getPosition();
		dvec3 currentVel = water->at(i)->getVelocity();
        float correctionFactor = 2.f * _restDensity / (density_i + density_j);
        forceCohesion += correctionFactor * (r / rn) * _kernel.surfaceTension(rn);
        forceCurvature += correctionFactor * (n_i - n_j);
		

		// Viscosity - skipped
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

double SPH::adaptTimestep(double timeStep)
{
	dvec3 val;
	double mag, dT, vMax = 0.0;

	for (int i = 0; i < particleCount; ++i)
	{
		val = water->at(i)->getVelocity();
		mag = dot(val, val);

		if (vMax < mag)
			vMax = mag;
	}
	dT = (particleRadius * 0.8 / vMax) - EPSILON;

	if (timeStep < dT)
		dT = timeStep;
	
	return dT;
}

// Predicts the velocity of the particle with its non-pressure forces and dirichlet boundary condition
void SPH::predictVelocities(double dT)
{
	dvec3 vel, pos, dPos;
	double x,y,z;
	for (int i = 0; i < particleCount; ++i) {
		vel = water->at(i)->getVelocity() + water->at(i)->getForce() * particleMass * dT;
		pos = water->at(i)->getPosition();
		
		// Dirichlet Boundary Condition
		x = pos.x;
		y = pos.y;
		z = pos.z;
		dPos = vel * dT;
		// is there solid in X?
		if(isSolid(x-dPos.x,y,z, 0) && vel.x < 0.0)
			vel.x = 0.0;
		else if(isSolid(x+dPos.x,y,z, 0) && vel.x > 0.0)
			vel.x = 0.0;
		// Y
		if(isSolid(x,y-dPos.y,z, 1) && vel.y < 0.0)
			vel.y = 0.0;
		else if(isSolid(x,y+dPos.y,z, 1) && vel.y > 0.0)
			vel.y = 0.0;
		// Z
		if(isSolid(x,y,z-dPos.z, 2) && vel.z < 0.0)
			vel.z = 0.0;
		else if(isSolid(x,y,z+dPos.z, 2) && vel.z > 0.0)
			vel.z = 0.0;

		water->at(i)->setVelocity(vel);
	}
	
}

// Check if (x,y,z) is inside the obj object
bool SPH::isSolid(double x, double y, double z, int c)
{

	// Cube
	if (sceneName == "cube") {
		switch (c) {
			case 0:
				if (x > 1.5 || x < -1.5)
					return true;
				break;
			case 1:
				if (y < 0.0)
					return true;
				break;
			case 2:
				if (z > 1.5 || z < -1.5)
					return true;
				break;
			default:
				return false;
		}
		return false;
	}

	// Two levels

	// Sphere
	else if (sceneName == "sphere") {
		double sqR = 4.0;
		switch (c) {
			case 0:
				if (x*x+y*y+z*z-sqR == 0.0)
					return true;
				break;
			case 1:
				if (x*x+y*y+z*z-sqR == 0.0)
					return true;
				break;
			case 2:
				if (x*x+y*y+z*z-sqR == 0.0)
					return true;
				break;
			default:
				return false;
		}
		return false;
	}

	// Default case, if our sceneName is something odd
	else
		return false;

			
}

//calculate density function
void SPH::calculateDensity()
{
	double distance = 0;
	
	glm::vec4 *particleVel;
	glm::vec4 *neighborVel;

	for(int i = 0; i < particleCount; i++)
	{
		water->at(i)->setDensity(0.0); // to be able to reuse this function, maybe not a good solution
		dvec3 particlePos = water->at(i)->getPosition();
		for(int j = 0; j < particleCount; j++)
		{
			dvec3 neighborPos = water->at(j)->getPosition();

			distance = dot(particlePos - neighborPos, particlePos - neighborPos);
				
			if(distance <= H*H) 
				water->at(i)->setDensity( water->at(i)->getDensity() +  particleMass * water->at(i)->kernel(neighborPos, H));
			else 
				continue;	
		}
	}
}

void SPH::calculateAlpha()
{
	double distance = 0, sum2 = 0, tmpAlpha = 0;
	dvec3 sum1 = dvec3(0,0,0);

	for(int i = 0; i < particleCount; i++)
	{	
		dvec3 particlePos = water->at(i)->getPosition();
		for(int j = 0; j < particleCount; j++)
		{
			dvec3 neighborPos = water->at(j)->getPosition();

			distance = dot(particlePos - neighborPos, particlePos - neighborPos);
				
			//only need to calc within neighborhood, kernel gradient will be zero otherwise 	
			if(distance <= H*H) {
				sum1 += particleMass * water->at(i)->gradientKernel(neighborPos, H);
				sum2 += dot(abs(particleMass*water->at(i)->gradientKernel(neighborPos, H)),abs(particleMass*water->at(i)->gradientKernel(neighborPos, H)));
			}
			else 
				continue;	
		}
		tmpAlpha = water->at(i)->getDensity()/(dot(abs(sum1),abs(sum1)) + sum2);
		water->at(i)->setAlpha(tmpAlpha);
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
		// Propagate the solution until requested time is reached
		while (t < maxTimestep && iter < iterations) {
			simulate(maxTimestep);
			t += dT;
			iter++;
		}

		// "   " used as padding as sometimes we get 10 iterations, sometimes 9
		cout << "Iterations: " << iter << ", Timestep: " << t << "        " << '\r' << flush;

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

	glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

	glDrawArrays(GL_POINTS, 0, particleCount);

	glDisable(GL_PROGRAM_POINT_SIZE);


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
