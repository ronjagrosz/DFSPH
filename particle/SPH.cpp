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

SPH::SPH()
{

	// load json parameters
	loadJson("json/scene_parameters.json");

	//scene.readOBJ(sceneName.c_str());
	//scene.printInfo();

	GLfloat randX, randY, randZ;
	vec3 newColor = vec3(1.0f,1.0f,1.0f);
	double randI, randJ, randK; //velocity vector values
	
	srand(time(0));
	water = new vector<Particle*>(particleCount);
    cellList = new CellList(dvec3(-2.0, -2.0, -2.0), dvec3(2.0, 2.0, 2.0), H);

    // Initiate particles
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
	}

	for(int i = 0; i < particleCount; ++i) {
		// Divide particles into cells
        cellList->addParticle(water->at(i), i);
    }
		// compute densities
		calculateDensity();	

		//cout << "density: " << water->at(i)->getDensity()<< "\n";	

		// compute ai
		calculateAlpha();
	

	createVAO();
}


SPH::~SPH()
{
	for(int i = 0; i < particleCount; i++)
	{
		delete water->at(i);
	}
	delete water;
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
    H = params.get<picojson::object>()["H"].get<double>();
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

	// Update particles position and cell
	for (int i = 0; i < particleCount; ++i) {
		water->at(i)->updatePosition(dT);
        cellList->moveParticle(water->at(i), i);
	}
    
    // Find neighbours
    for (int i = 0; i < particleCount; ++i) {
        water->at(i)->updateNeighbours(cellList->findNeighbours(water, i));
    }

	// Compute densities
	calculateDensity();

	// Compute ai factors
	calculateAlpha();

	// correctDivergenceError
	correctDivergenceError();

	// Update velocities
	
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
		Q[3] = vec4(geometry.x/2.0, geometry.y/2.0, geometry.z/2.0, 0);
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
	glm::vec4 *particleVel;
	glm::vec4 *neighborVel;

	for(int i = 0; i < particleCount; i++) {
		water->at(i)->setDensity(0.0); // to be able to reuse this function, maybe not a good solution
		dvec3 particlePos = water->at(i)->getPosition();

        // Loop through neighbours and set density
        for (vector<int>::iterator it 
                = water->at(i)->getNeighbours()->begin();
                it != water->at(i)->getNeighbours()->end(); ++it) {
			water->at(i)->setDensity(
                    water->at(i)->getDensity()
                    + particleMass
                    * water->at(i)->kernel(water->at(*it)->getPosition(), H));
		}
	}
}

void SPH::calculateAlpha()
{

	for(int i = 0; i < particleCount; i++) {	
	    double sum2 = 0, alpha = 0;
	    dvec3 sum1 = dvec3(0,0,0);

        // Go through neighbours
        for (vector<int>::iterator it 
                = water->at(i)->getNeighbours()->begin();
                it != water->at(i)->getNeighbours()->end(); ++it) {
				
			// Only need to calc within neighborhood, kernel gradient will be zero otherwise 	
			sum1 += particleMass
                * water->at(i)->gradientKernel(water->at(*it)->getPosition(), H);
			sum2 += dot(abs(particleMass
                * water->at(i)->gradientKernel(water->at(*it)->getPosition(), H)), 
                abs(particleMass
                * water->at(i)->gradientKernel(water->at(*it)->getPosition(), H)));
		}
		alpha = water->at(i)->getDensity()/(dot(abs(sum1),abs(sum1)) + sum2);
		water->at(i)->setAlpha(alpha);
	}	
}

// Maintains the pressure difference = 0 in each simulation loop
void SPH::correctDivergenceError() 
{
	double dPavg = 10.0, dPi;
	int iter = 0;
	while (dPavg > 0.1 && iter < 1) {
		dPi = 0.0;
		dPavg = 0.0;

		// Compute pressure difference in particle i (dPi), dPavg is the average difference
		for (int i = 0; i < particleCount; ++i) {
			for (vector<int>::iterator it = water->at(i)->getNeighbours()->begin();
                it != water->at(i)->getNeighbours()->end(); ++it) {
				dPi += (particleMass * 
					dot((water->at(i)->getVelocity() - water->at(*it)->getVelocity()) 
					, water->at(i)->gradientKernel(water->at(*it)->getPosition(), H)));
			}
			water->at(i)->setdDensity(dPi);
			dPavg += dPi;
		}
		dPavg /= particleCount;

		for (int i = 0; i < particleCount; ++i) {
			// adapt velocities
			double ki = 1/dT * water->at(i)->getdDensity() * water->at(i)->getAlpha(); 

			dvec3 sum = dvec3(0.0, 0.0, 0.0);
			for (vector<int>::iterator it = water->at(i)->getNeighbours()->begin();
                it != water->at(i)->getNeighbours()->end(); ++it) {

				double kj = 1/dT * water->at(*it)->getdDensity() * water->at(*it)->getAlpha();

				sum += particleMass 
				* (ki/water->at(i)->getDensity() + kj/water->at(*it)->getDensity()) 
				* water->at(i)->gradientKernel(water->at(*it)->getPosition(), H);
			}
			dvec3 temp = water->at(i)->getVelocity();// − (sum * dT);
			temp -= sum * dT;
			water->at(i)->setVelocity(temp);
		}
	iter++;		
	}
	
}

void SPH::display()	
{	
	GLfloat vertices[particleCount][3];
	GLfloat colors[particleCount][3];

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

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // make background black
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(vao);

	glEnable(GL_PROGRAM_POINT_SIZE); //enable gl_PointSize in vertex shader

	glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

	glDrawArrays(GL_POINTS, 0, particleCount);

	glDisable(GL_PROGRAM_POINT_SIZE);
}

void SPH::createVAO () {
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
