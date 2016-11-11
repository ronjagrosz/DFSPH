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
		randY = ((float)rand()/(float)RAND_MAX) * 1.0 - 0.5 + 1.4;
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

// Loads properties from json-file
void SPH::loadJson(string fileName)
{
	// Load scene properties
	picojson::value params;
    ifstream paramStream (fileName);
    paramStream >> params;
    particleCount = (int)(params.get<picojson::object>()["particlesCount"].get<double>());
    maxTimestep = 1.0/params.get<picojson::object>()["fps"].get<double>();
    iterations = (int)(params.get<picojson::object>()["iterations"].get<double>());
    constantAcceleration = params.get<picojson::object>()["constantAcceleration"].get<double>();
    sceneName = params.get<picojson::object>()["sceneName"].get<std::string>();    
    geometry = dvec4(params.get<picojson::object>()["geometry"]
    		.get<picojson::object>()["a"].get<double>(),
    	params.get<picojson::object>()["geometry"]
    		.get<picojson::object>()["b"].get<double>(),
		params.get<picojson::object>()["geometry"]
	    	.get<picojson::object>()["c"].get<double>(),
	    params.get<picojson::object>()["geometry"]
    		.get<picojson::object>()["r"].get<double>());


    // Load particle properties
    particleRadius = params.get<picojson::object>()["particleRadius"].get<double>();
    //particleMass = params.get<picojson::object>()["particleMass"].get<double>();
    particleMass = 4.0*particleRadius*particleRadius*M_PI/3.0  *  params.get<picojson::object>()["density"].get<double>();

    cout << "Particle radius: " << particleRadius << "m\n";
    cout << "Particle mass calculated from radius: " << particleMass*1000.0 << "g\n";

    particleViscosity = params.get<picojson::object>()["particleViscosity"].get<double>();
}

// Update positions with a small timestep
void SPH::simulate()
{
	// Self-advection - Skipped for SPH

	// Calculate non-pressure forces (gravity)
	//for(int i = 0; i < particleCount; ++i)
		//water->at(i)->setForce(0.0, particleMass * constantAcceleration, 0.0);

	// adapt timestep according to CFL condition
	adaptTimestep();

	// predict velocities
	predictVelocities();

	// correctDensityError

	// update positions
	for (int i = 0; i < particleCount; ++i) {
		water->at(i)->updatePosition(dT);
	}

	// update neighborhoods

	// compute densities and ai factors

	// correctDivergenceError

	// update velocities
	
}

// Adapts the timestep according to the CFL condition
void SPH::adaptTimestep()
{
	dvec3 vel;
	double mag, vMax = 0.0;

	// Find max velocity
	for (int i = 0; i < particleCount; ++i)
	{
		vel = water->at(i)->getVelocity();
		mag = dot(vel, vel);

		if (vMax < mag)
			vMax = mag;
	}

	dT = (particleRadius * 0.8 / vMax) - EPSILON;

	// make sure dT is less than the maximum timestep
	if (maxTimestep < dT)
		dT = maxTimestep;
}

// Predicts the velocity of the particle with its non-pressure forces and dirichlet boundary condition
void SPH::predictVelocities()
{
	dvec3 vel, pos, dPos;
	
	for (int i = 0; i < particleCount; ++i) {
		vel = water->at(i)->getVelocity() + dvec3(0.0, constantAcceleration * dT, 0.0); //water->at(i)->getForce()/particleMass * dT;
		pos = water->at(i)->getPosition();
		dPos = vel * dT;

		// Dirichlet Boundary Condition
		if(isSolid(vec4(pos.x+dPos.x, pos.y, pos.z, 1.0))) // X
			vel.x = 0.0;
		if(isSolid(vec4(pos.x, pos.y+dPos.y, pos.z, 1.0))) // Y
			vel.y = 0.0;
		if(isSolid(vec4(pos.x, pos.y, pos.z+dPos.z, 1.0))) // Z
			vel.z = 0.0;
		
		water->at(i)->setVelocity(vel); // should we store a predictedVelocity vector instead?
	}
	
}

// Check if (x,y,z) is inside an implicit geometry
bool SPH::isSolid(dvec4 p)
{
	dmat4 Q = dmat4(0.0);

	// Cylinder
	if (sceneName == "cylinder") {
		Q[0][0] = 1.0;
		Q[1][1] = 1.0;
		Q[3][3] = -geometry.w;
	}

	// Ellipsoid
	else if (sceneName == "ellipsoid") {
		Q[0][0] = 1.0/pow(geometry.x,2);
		Q[1][1] = 1.0/pow(geometry.y,2);
		Q[2][2] = 1.0/pow(geometry.z,2);
		Q[3][3] = -geometry.w;
	}

	// Plane
	else if (sceneName == "plane") {
		Q[0][3] = geometry.x/2.0;
		Q[2][3] = geometry.y/2.0;
		Q[1][3] = geometry.z/2.0;
		Q[3] = {geometry.x/2.0, geometry.y/2.0, geometry.z/2.0, 0};
	}

	// Paraboloid
	else if (sceneName == "paraboloid") {
		Q[0][0] = geometry.w;
		Q[1][1] = geometry.w; // + or -
		Q[2][3] = -geometry.w/2;
		Q[3][2] = -geometry.w/2;
	}


	// Hyperboloid
	else if (sceneName == "hyperboloid") {
		Q[0][0] = geometry.w;
		Q[1][1] = geometry.w;
		Q[2][2] = -geometry.w;
		Q[3][3] = -geometry.w; // + or -
	}

	if (max(abs(p.x),abs(p.y),abs(p.z)) > 2.0)
		return true;
	// Cube
	else if (sceneName == "cube" && max(abs(p.x),abs(p.y),abs(p.z)) > geometry.w)
		return false;
	else
		return (dot(p,(Q * p)) < 0.0);
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
		// Propagate the solution until requested time is reached
		while (t < maxTimestep && iter < iterations) {
			simulate();
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
