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

using namespace glm;

SPH::SPH()
{
	// load json parameters
	loadJson("json/scene_parameters.json");

	GLfloat randX, randY, randZ;
	maxVelocity = constantAcceleration * maxTimestep;	
	srand(time(0));
	double step = 3.0 * particleRadius / boundaryDimension;

	water = new vector<Particle*>(particleCount);
    cellList = new CellList(
    	dvec3(-2*boundaryDimension, -2*boundaryDimension, -2*boundaryDimension), 
    	dvec3(2*boundaryDimension, 2*boundaryDimension+3.0, 2*boundaryDimension), H);

    // Initiate particles
	for(int i = 0; i < particleCount; ++i) {

		// Random position
		randX = (((float)rand()/(float)RAND_MAX) * 2.0 - 1.0) * boundaryDimension;
		randY = (((float)rand()/(float)RAND_MAX) * 2.0 - 0.0) * boundaryDimension;
		randZ = (((float)rand()/(float)RAND_MAX) * 2.0 - 1.0) * boundaryDimension;

		if (alongBoundary(dvec4(randX, randY, randZ, 1.0)) != dvec3(0,0,0)) {
			randY += 0.5;
		}

		water->at(i) = new Particle();
		water->at(i)->setPosition(randX, randY, randZ);
		water->at(i)->setVelocity(dvec3(0.0, 0.0, 0.0));
		water->at(i)->id = 0;
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

	// Generate (one) new Vertex Buffer Object and get the associated id
	glGenBuffers(2, vbo);
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

    boundaryDimension = params.get<picojson::object>()["boundaryDimension"].get<double>();
    // Load particle properties
    particleRadius = params.get<picojson::object>()["particleRadius"].get<double>();
    particleMass = params.get<picojson::object>()["particleMass"].get<double>();
    restDensity = params.get<picojson::object>()["density"].get<double>();
    //particleMass = 4.0*pow(particleRadius,3)*M_PI/3.0 * restDensity;
    H = params.get<picojson::object>()["H"].get<double>();
    maxError = params.get<picojson::object>()["maxError"].get<double>();
    maxErrorV = params.get<picojson::object>()["maxErrorV"].get<double>();
    
    frameError = params.get<picojson::object>()["frameError"].get<double>();
    
    cout << "Particle radius: " << particleRadius << "m\n";
    cout << "Particle mass: " << particleMass*1000.0 << "g\n";
}

// Update positions with a small timestep
void SPH::simulate() {
	

	// Predict velocities
	predictVelocities();

	// correctDensityError
	correctDensityError();

	// Update particles position and cell
	for (int i = 0; i < particleCount; ++i) {
		water->at(i)->updatePosition(dT);
		cellList->moveParticle(water->at(i), i);
	}

    // Find neighbours
    for (int i = 0; i < particleCount; ++i) {
        water->at(i)->updateNeighbours(cellList->findNeighbours(water, i));
    }

	// Compute densities and alpha factors
	calculateDensityAndAlpha();
	
	// correctDivergenceError
	correctDivergenceError();
}

// Adapts the timestep according to the CFL condition
void SPH::adaptTimestep() {
	dvec3 vel;
	double mag;
	maxVelocity = 0.0;

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
	gravity *= dT;
	for (int i = 0; i < particleCount; ++i) {
		water->at(i)->setVelocity(water->at(i)->getVelocity() + gravity);
		boundaryCondition(i);
	}
	gravity /= dT;
}

// Dirichlet and geometry boundary conditions. Making sure we don't move particles into solids
void SPH::boundaryCondition(int i) {
	dvec3 vel = water->at(i)->getVelocity();
	dvec3 pos = water->at(i)->getPosition();
	dvec3 dPos = vel * dT;
	dvec3 gradP = alongBoundary(dvec4(pos+dPos, 1.0));

	// Dirichlet
	if(pos.x+dPos.x < -boundaryDimension || pos.x+dPos.x > boundaryDimension) // X
		vel.x = 0.0;
	if(pos.y+dPos.y < -boundaryDimension) // Y
		vel.y = 0.0;
	if(pos.z+dPos.z < -boundaryDimension || pos.z+dPos.z > boundaryDimension) // Z
		vel.z = 0.0;

	// Boundary of geometry
	if (gradP != dvec3(0.0, 0.0, 0.0)) {
		if (vel.y > 0.0 && pos.y < 0.0)
			vel = dvec3(0.0, 0.0, 0.0);
		else
			vel = gradP + gravity*dT;
	}

	water->at(i)->setVelocity(vel);
}

// Check if (x,y,z) is inside an implicit geometry, if so return the gradient in that p
dvec3 SPH::alongBoundary(dvec4 p) {
	dmat4 Q = dmat4(0.0);
	dvec4 gradP = dvec4(0.0, 0.0, 0.0, 0.0);

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
		Q[2][3] = -geometry.w/2.0;
		Q[3][2] = -geometry.w/2.0;
	}

	// Hyperboloid
	else if (sceneName == "hyperboloid") {
		Q[0][0] = geometry.w;
		Q[1][1] = geometry.w;
		Q[2][2] = -geometry.w;
		Q[3][3] = -geometry.w; // + or -
	}
		
	if (dot(p,(Q * p)) < 0.0) {
		dmat4 Qsub = Q;
		for (int i = 0; i < 4; ++i) {
			Qsub[3][i] = 0;
		}
		if (sceneName == "plane")
			gradP = 2.0 * Q * p;
		else 
			gradP = 2.0 * Qsub * p;
	}
	return dvec3(gradP);
}

// Calculate density function
void SPH::calculateDensityAndAlpha() {
	for (int i = 0; i < particleCount; ++i) {
		double alpha = 0, sumGradPk = 0.0;
	    dvec3 gradPi = dvec3(0,0,0);
		water->at(i)->setDensity(0.0); // to be able to reuse this function, maybe not a good solution
        for (vector<int>::iterator it 
                = water->at(i)->getNeighbours()->begin();
                it != water->at(i)->getNeighbours()->end(); ++it) {

        	// Density
			water->at(i)->setDensity(water->at(i)->getDensity() + particleMass
                * water->at(i)->kernel(water->at(*it)->getPosition(), H));

			// Alpha sum denominator
			dvec3 gradPj = particleMass * water->at(i)->gradientKernel(water->at(*it)->getPosition(), H);
			sumGradPk += dot(gradPj,gradPj); 
			gradPi += gradPj;
		}
		sumGradPk += dot(gradPi, gradPi);
		alpha = std::max(sumGradPk, 1.0e-6); // doesnt really do anything?
		alpha = 1.0/alpha;
		water->at(i)->setAlpha(alpha); 
	}
}

void SPH::calculateDensityChange() {
	double densityAdv = 0.0;
	avgDensityAdv = avgDerivedDensity = 0.0;

	for (int i = 0; i < particleCount; ++i) {
		double dDensity = 0.0;
		for (vector<int>::iterator it = water->at(i)->getNeighbours()->begin();
	        it != water->at(i)->getNeighbours()->end(); ++it) {
			
			dDensity += (particleMass * dot(water->at(i)->getVelocity() - water->at(*it)->getVelocity() 
				, water->at(i)->gradientKernel(water->at(*it)->getPosition(), H)));
		}
		densityAdv = water->at(i)->getDensity() + (dT * dDensity);
		densityAdv = std::max(densityAdv, restDensity);

		water->at(i)->setDerivedDensity(dDensity);
		water->at(i)->setDensityAdv(densityAdv);
		avgDerivedDensity += dDensity;
		avgDensityAdv += densityAdv;
	}
	avgDerivedDensity /= (particleCount - 0);
	avgDensityAdv /= (particleCount - 0);
}

// Correct density error
void SPH::correctDensityError() {
	int iter = 0;
	double invdT2 = 1.0/(dT*dT);

	// Calculate average density through euler integration
	calculateDensityChange();
	
	// Lower calculations of ki/kj by adding scaling directly
	for (int i = 0; i < particleCount; ++i) {
		water->at(i)->setAlpha(water->at(i)->getAlpha() * invdT2);
	}

	const double eta = maxError * 0.01 * restDensity;  // maxError is given in percent
	while ((abs(avgDensityAdv - restDensity) > eta || (iter < 2)) && (iter < 100)) {
		for (int i = 0; i < particleCount; ++i) {
			dvec3 sum = dvec3(0.0, 0.0, 0.0);
			double ki = std::max((water->at(i)->getDensityAdv() - restDensity) 
			* water->at(i)->getAlpha(), 0.5); 

			for (vector<int>::iterator it = water->at(i)->getNeighbours()->begin();
                it != water->at(i)->getNeighbours()->end(); ++it) {

				double kj = std::max((water->at(i)->getDensityAdv() - restDensity) 
				* water->at(i)->getAlpha(), 0.5); 
				sum += particleMass * (ki + kj)
					* water->at(i)->gradientKernel(water->at(*it)->getPosition(), H); 
			}

			if (water->at(i)->id == 0) {
				water->at(i)->setVelocity(water->at(i)->getVelocity() - (sum * dT));
				boundaryCondition(i);
			}
			// Last velocity change before updatePosition, make sure we don't go into boundaries
			
		}
		calculateDensityChange();
		iter++;
	}
	// Change alpha back
	for (int i = 0; i < particleCount; ++i) {
		water->at(i)->setAlpha(water->at(i)->getAlpha() * dT*dT);
	}
}

// Maintains the pressure difference = 0 in each simulation loop
void SPH::correctDivergenceError() {
	int iter = 0;
	double invdT = 1.0/dT;

	// Compute pressure difference in particle i (dPi), dPavg is the average difference
	calculateDensityChange();

	// Lower calculations of ki/kj by adding scaling directly
	for (int i = 0; i < particleCount; ++i) {
		water->at(i)->setAlpha(water->at(i)->getAlpha() * invdT / water->at(i)->getDensity());
	}

	const double etaV = maxErrorV;
	while ((abs(avgDerivedDensity) > etaV || (iter < 1)) && (iter < 100)) {
		for (int i = 0; i < particleCount; ++i) {
			dvec3 sum = dvec3(0.0, 0.0, 0.0);
			double ki = water->at(i)->getDerivedDensity() * water->at(i)->getAlpha(); 

            for (vector<int>::iterator it = water->at(i)->getNeighbours()->begin();
                it != water->at(i)->getNeighbours()->end(); ++it) {

            	double kj = water->at(i)->getDerivedDensity() * water->at(i)->getAlpha(); 

            	sum += particleMass * (ki + kj)
				* water->at(i)->gradientKernel(water->at(*it)->getPosition(), H);
			}
			if (water->at(i)->id == 0) {
				water->at(i)->setVelocity(water->at(i)->getVelocity() - (sum * dT));
				boundaryCondition(i);
			}
		}
		calculateDensityChange();
		iter++;		
	}
	// Change alpha back
	for (int i = 0; i < particleCount; ++i) {
		water->at(i)->setAlpha(water->at(i)->getAlpha() * dT * water->at(i)->getDensity());
	}
}


void SPH::display(float phiW, float thetaW, GLuint vao)	{	
	GLfloat vertices[particleCount][3];
	GLfloat velocities[particleCount][3];

	mat4 viewMatrix = mat4(1.0);
	viewMatrix = viewMatrix * glm::rotate(thetaW, vec3(1.0f, 0.0f, 0.0f))
    * glm::rotate(phiW, vec3(0.0f, 0.0f, -1.0f));
    gravity = dvec3(viewMatrix * dvec4(0.0, constantAcceleration, 0.0, 1.0f));

	double t = 0.0;
	int iter = 0;
	// Propagate the solution until requested time is reached
	while (t < maxTimestep && iter < iterations) {
		// Adapt timestep according to CFL condition
		adaptTimestep();
		if (t + dT < maxTimestep) {
			simulate();
		}
		else if (maxTimestep - t > frameError) {
			dT = maxTimestep - t;
			simulate();
		}
		else
			break;

		t += dT;
		iter++;
	}

	// "   " used as padding as sometimes we get 10 iterations, sometimes 9
	cout << "Iterations: " << iter << ", Timestep: " << t << ", Gravity: (" 
	<< gravity.x << ", " << gravity.y << ", " << gravity.z << ")                  " << '\r' << flush;

	// Render stuff
	for(int i = 0; i < particleCount; ++i) {
		vertices[i][0] = water->at(i)->getPosition().x;
		vertices[i][1] = water->at(i)->getPosition().y;
		vertices[i][2] = water->at(i)->getPosition().z;

		velocities[i][0] = water->at(i)->getVelocity().x;
		velocities[i][1] = water->at(i)->getVelocity().y;
		velocities[i][2] = water->at(i)->getVelocity().z;
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

    glBufferData(GL_ARRAY_BUFFER, 3 * particleCount * sizeof(GLfloat), velocities, GL_STATIC_DRAW);

    // Specify that our color data is going into attribute index 1, and contains three floats per vertex 
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Enable attribute index 1 as being used
    glEnableVertexAttribArray(1);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Make background black

	glBindVertexArray(vao);

	glEnable(GL_PROGRAM_POINT_SIZE); // Enable gl_PointSize in vertex shader

	glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

	glDrawArrays(GL_POINTS, 0, particleCount);

	glDisable(GL_PROGRAM_POINT_SIZE);
}
