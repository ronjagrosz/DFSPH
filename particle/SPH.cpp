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
	double randI, randJ, randK; // Velocity vector values
	
	srand(time(0));

	water = new vector<Particle*>(particleCount);
    cellList = new CellList(glm::dvec3(-0.5, -0.5, -0.5), glm::dvec3(0.5, 0.5, 0.5), H);

    // Initiate particles
	for(int i = 0; i < particleCount; ++i) {

		// Random position
		randX = ((float)rand()/(float)RAND_MAX) * 1.0 - 0.5;
		randY = ((float)rand()/(float)RAND_MAX) * 1.0 - 0.5;
		randZ = ((float)rand()/(float)RAND_MAX) * 1.0 - 0.5;

		// Random velocity
		randI = (((double)rand()/(double)RAND_MAX) * 0.2) - 0.1;
		randJ = (((double)rand()/(double)RAND_MAX) * 0.2) - 0.1;
		randK = (((double)rand()/(double)RAND_MAX) * 0.2) - 0.1;
		
		water->at(i) = new Particle();
		water->at(i)->setPosition(randX, randY, randZ);
		water->at(i)->setVelocity(dvec3(randI, randJ, randK));
	}

	for(int i = 0; i < particleCount; ++i) {
		// Divide particles into cells
        cellList->addParticle(water->at(i), i);
	}

	// Find neighbours
    for (int i = 0; i < particleCount; ++i) {
        water->at(i)->updateNeighbours(cellList->findNeighbours(water, i));
    }

	// Compute densities and alpha factors
	calculateDensityAndAlpha();		

	restDensity = 0.0;
	for (int i = 0; i < particleCount; ++i) 
		restDensity += water->at(i)->getDensity();
	restDensity /= particleCount;

	cout << "restDensity: " << restDensity << "\n";

	createVAO();
}


SPH::~SPH() {
	for(int i = 0; i < particleCount; ++i) {
		delete water->at(i);
	}
	delete water;
}

// Loads properties from json-file
void SPH::loadJson(string fileName) {
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
    particleMass = 4.0*pow(particleRadius,3)*M_PI/3.0  *  params.get<picojson::object>()["density"].get<double>() * 1000;
    H = params.get<picojson::object>()["H"].get<double>();
    
    cout << "Particle radius: " << particleRadius << "m\n";
    cout << "Particle mass calculated from radius: " << particleMass*1000.0 << "g\n";
}

// Update positions with a small timestep
void SPH::simulate() {
	cout << "New simulate                         \n";

	// Adapt timestep according to CFL condition
	adaptTimestep();

	//cout << water->at(0)->getVelocity().y << " 1\n";

	// Predict velocities
	predictVelocities();


	//cout << water->at(0)->getVelocity().y << " 2\n";

	// correctDensityError
	correctDensityError();

	//cout << water->at(0)->getVelocity().y << " 3\n";

	// Update particles position and cell
	for (int i = 0; i < particleCount; ++i) {
		water->at(i)->updatePosition(dT);
        cellList->moveParticle(water->at(i), i);
	}

	//cout << water->at(0)->getVelocity().y << " 4\n";
   
    // Find neighbours
    for (int i = 0; i < particleCount; ++i) {
        water->at(i)->updateNeighbours(cellList->findNeighbours(water, i));
    }

	//cout << water->at(0)->getVelocity().y <<  " 5\n";

	// Compute densities and alpha factors
	calculateDensityAndAlpha();

	//cout << water->at(0)->getVelocity().y << " 6\n";
	
	// correctDivergenceError
	correctDivergenceError();
}

// Adapts the timestep according to the CFL condition
void SPH::adaptTimestep() {
	dvec3 vel;
	double mag, maxVelocity = 0.0;

	// Find max velocity
	for (int i = 0; i < particleCount; ++i) {
		vel = water->at(i)->getVelocity();
		mag = dot(vel, vel);

		if (maxVelocity < mag)
			maxVelocity = mag;
	}

	dT = (particleRadius * 0.8 / sqrt(maxVelocity));

	// make sure dT is less than the maximum timestep
	if (maxTimestep < dT)
		dT = maxTimestep;
}

// Predicts the velocity of the particle with its non-pressure forces and dirichlet boundary condition
void SPH::predictVelocities() {
	dvec3 vel, pos, dPos;
	
	for (int i = 0; i < particleCount; ++i) {
		vel = water->at(i)->getVelocity() + dvec3(0.0, constantAcceleration * dT, 0.0);
		pos = water->at(i)->getPosition();
		dPos = vel * dT;

		// Dirichlet Boundary Condition
		vel = dirichletBoundary(pos, dPos, vel);
		
		water->at(i)->setVelocity(vel);
	}
	
}

dvec3 SPH::dirichletBoundary(dvec3 pos, dvec3 dPos, dvec3 vel) {
	if(isSolid(dvec4(pos.x+dPos.x, pos.y, pos.z, 1.0))) // X
		vel.x = 0.0; //-0.1*vel.x;
	if(isSolid(dvec4(pos.x, pos.y+dPos.y, pos.z, 1.0))) // Y
		vel.y = 0.0; //-0.1*vel.y;
	if(isSolid(dvec4(pos.x, pos.y, pos.z+dPos.z, 1.0))) // Z
		vel.z = 0.0; //-0.1*vel.z;

	return vel;
}

// Check if (x,y,z) is inside an implicit geometry
bool SPH::isSolid(dvec4 p) {
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

	if (max(abs(p.x),abs(p.y),abs(p.z)) > 0.5)
		return true;
	// Cube
	else if (sceneName == "cube") {
		if (max(abs(p.x),abs(p.y+1.0),abs(p.z)) > geometry.w)
			return false;
		else
			return true;
	}
	else
		return (dot(p,(Q * p)) < 0.0);
}

// Calculate density function
void SPH::calculateDensityAndAlpha() {
	for(int i = 0; i < particleCount; ++i) {
		double sum2 = 0, alpha = 0;
	    dvec3 sum1 = dvec3(0,0,0);
		water->at(i)->setDensity(0.0); // to be able to reuse this function, maybe not a good solution
		
		// Loop through neighbours and set density and alpha
        for (vector<int>::iterator it 
                = water->at(i)->getNeighbours()->begin();
                it != water->at(i)->getNeighbours()->end(); ++it) {

			water->at(i)->setDensity(water->at(i)->getDensity() + particleMass
                * water->at(i)->kernel(water->at(*it)->getPosition(), H));

			// Only need to calc within neighborhood, kernel gradient will be zero otherwise 	
			sum1 += particleMass
                * water->at(i)->gradientKernel(water->at(*it)->getPosition(), H);
			
			sum2 += dot(abs(particleMass
                * water->at(i)->gradientKernel(water->at(*it)->getPosition(), H)),
                 abs(particleMass
                * water->at(i)->gradientKernel(water->at(*it)->getPosition(), H)));

		}
		//cout << i << ": " << water->at(i)->getDensity() << "                                 \n";
		alpha = water->at(i)->getDensity()/(dot(abs(sum1),abs(sum1)) + sum2);
		water->at(i)->setAlpha(alpha); 
	}
}

double SPH::calculateDensityChange(int i) {
	double dDensity = 0.0;
	vector<int>::iterator check = water->at(i)->getNeighbours()->begin();
	if (check == water->at(i)->getNeighbours()->end())
        cout << "No neighbours" << endl;
	for (vector<int>::iterator it = water->at(i)->getNeighbours()->begin();
        it != water->at(i)->getNeighbours()->end(); ++it) {
		dDensity += (particleMass * 
			dot((water->at(i)->getVelocity() - water->at(*it)->getVelocity()) 
			, water->at(i)->gradientKernel(water->at(*it)->getPosition(), H)));
	}
	water->at(i)->setdDensity(dDensity);
	return dDensity;
}

// Correct density error
void SPH::correctDensityError()
{
	double avgDensity = 0.0;
	int iter = 0;

	// Calculate average density through euler integration
	for (int i = 0; i < particleCount; ++i) {
		double tmp = water->at(i)->getDensity() + dT*calculateDensityChange(i);
		water->at(i)->setDensity(tmp);
		avgDensity += tmp;
	}
	avgDensity /= particleCount;

	cout << "DensityError:                              \n";

	// Print what we get in:
	dvec3 avgVel = dvec3(0.0,0.0,0.0);
	for (int i = 0; i < particleCount; ++i) {
		avgVel += water->at(i)->getVelocity();
	}
	avgVel /= particleCount;

	cout << "avgDensity: " << avgDensity << ", avgVel: " << to_string(avgVel)
	<< ", Go in: " << (abs(avgDensity - restDensity) > 0.1*restDensity) << "\n";

	while (abs(avgDensity - restDensity) > 0.1*restDensity || iter < 1) {
		avgDensity = 0.0;

		for (int i = 0; i < particleCount; ++i) {
			double ki = (water->at(i)->getDensity() - restDensity) / (dT*dT) * water->at(i)->getAlpha(); 
			dvec3 sum = dvec3(0.0, 0.0, 0.0);
			
			for (vector<int>::iterator it = water->at(i)->getNeighbours()->begin();
                it != water->at(i)->getNeighbours()->end(); ++it) {
				
				double kj = (water->at(*it)->getDensity() - restDensity) / (dT*dT) * water->at(*it)->getAlpha(); 
				
				sum += particleMass 
					* (ki/water->at(i)->getDensity() + kj/water->at(*it)->getDensity())
					* water->at(i)->gradientKernel(water->at(*it)->getPosition(), H); 
			}
			dvec3 temp = water->at(i)->getVelocity() - (sum * dT);
			water->at(i)->setVelocity(temp);
		}
		// Calculate average density through euler integration
		for (int i = 0; i < particleCount; ++i) {
			double tmp = water->at(i)->getDensity() + dT*calculateDensityChange(i);
			water->at(i)->setDensity(tmp);
			avgDensity += tmp;
		}
		avgDensity /= particleCount;
		
		dvec3 avgVel = dvec3(0.0,0.0,0.0);
		for (int i = 0; i < particleCount; ++i) {
			avgVel += water->at(i)->getVelocity();
		}
		avgVel /= particleCount;

		cout << iter << ": " << avgDensity << ", " << restDensity << ", " 
		<< "avgVel: " << to_string(avgVel) << ", Go in: " << (abs(avgDensity - restDensity) > 0.1*restDensity) << "\n";
		iter++;
	}
}

// Maintains the pressure difference = 0 in each simulation loop
void SPH::correctDivergenceError() {
	double dDensityAvg = 0.0;
	int iter = 0;

	// Compute pressure difference in particle i (dPi), dPavg is the average difference
	for (int i = 0; i < particleCount; ++i) {
		dDensityAvg += calculateDensityChange(i);
	}
	dDensityAvg /= particleCount;
	
	cout << "DivergenceError:                      \n";
	// Print what we get in:
	dvec3 avgVel = dvec3(0.0,0.0,0.0);
	for (int i = 0; i < particleCount; ++i) {
		avgVel += water->at(i)->getVelocity();
	}
	avgVel /= particleCount;

	cout << "dDensityAvg: " << dDensityAvg << ", avgVel: " << to_string(avgVel)
	<< ", Go in: " << (abs(dDensityAvg) > 0.2) << "\n";

	while (abs(dDensityAvg) > 0.2 || iter < 1) {
		dDensityAvg = 0.0;

		for (int i = 0; i < particleCount; ++i) {
			//cout << "Alpha " << i << ": " << water->at(i)->getAlpha() << "             \n";
			double ki = 1/dT * water->at(i)->getdDensity() * water->at(i)->getAlpha();
			
			dvec3 sum = dvec3(0.0, 0.0, 0.0);
            vector<int>::iterator check = water->at(i)->getNeighbours()->begin();
            for (vector<int>::iterator it = water->at(i)->getNeighbours()->begin();
                it != water->at(i)->getNeighbours()->end(); ++it) {

				double kj = 1/dT * water->at(*it)->getdDensity() * water->at(*it)->getAlpha();
				sum += particleMass 
					* (ki/water->at(i)->getDensity() + kj/water->at(*it)->getDensity()) 
					* water->at(i)->gradientKernel(water->at(*it)->getPosition(), H);
			}

			dvec3 temp = water->at(i)->getVelocity() - (sum * dT);
			water->at(i)->setVelocity(temp);
		}

		for (int i = 0; i < particleCount; ++i) {
			dDensityAvg += calculateDensityChange(i);
		}
		dDensityAvg /= particleCount;

		cout << iter << ": " << dDensityAvg << "                            \n";
		iter++;		
	}
}

void SPH::display()	{	
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

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Make background black
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(vao);

	glEnable(GL_PROGRAM_POINT_SIZE); // Enable gl_PointSize in vertex shader

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
