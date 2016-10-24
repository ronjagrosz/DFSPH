/*************************************************************************
Name:	    Isabell Jansson, Jonathan Bosson, Ronja Grosz
File name:  Particle.cpp

The class Particle represents a single smooth particle.  It is responsible for
all calculations regarding that particle (eg. getting the force from this 
particle at a certain point.
*************************************************************************/

#include <stdlib.h>
#include <time.h>

#include <iostream>
#include <vector>
#include <stack>
#include <cmath>

#include "../particle/Particle.h"

//#define WEIGHTLESS


using namespace std;
using namespace glm;

Particle::Particle():radius(1),mass(1),viscosity(2.034),force(1)
{
	neighbors = new vector<int>;

	velocity  = dvec3(0,0,0);
	force = dvec3(0,0,0);
}

Particle::~Particle()
{
	delete neighbors;
}

// Setters  ***************************************************************
void Particle::setPosition(float x, float y, float z)
{
	position = dvec3(x, y, z);
}

void Particle::setVelocity(double i, double j, double k)
{
	velocity = dvec3(i, j, k);
}
void Particle::setForce(double i, double j, double k)
{
	force = dvec3(i, j, k);
}
void Particle::setColor(vec3 newColor)
{
	if(newColor != color)
		color = newColor;
}
void Particle::setRadius(double newRadius)
{
	radius = newRadius;
}
void Particle::setMass(double newMass)
{
	mass = newMass;
}
void Particle::setViscosity(double newViscosity)
{
	viscosity = newViscosity;
}
void Particle::setDensity(double newDensity)
{
	density = newDensity;
}

// Getters  ***************************************************************
dvec3 Particle::getPosition(){return position;}
dvec3 Particle::getVelocity(){return velocity;}
dvec3 Particle::getForce(){return force;};
vec3 Particle::getColor(){return color;}
double Particle::getRadius(){return radius;}
double Particle::getMass(){return mass;}
double Particle::getViscosity(){return viscosity;};
double Particle::getDensity(){return density;};
double Particle::getStiffness(){return stiffness;};

// Compute non-pressure forces like gravity, surface tension and viscosity
void Particle::calculateForces(Particle *neighbor)
{	
	dvec3 nPosition = neighbor->position;	//do NOT delete this vector
	
	double distance = 0.0;

	double nMass = neighbor->mass;
	double nDensity = neighbor->density;
	double nViscosity = neighbor->viscosity;
	
	// needed?
	dvec3 diffVector = nPosition - position;
	distance = dot(diffVector, diffVector);

	dvec3 *pressureKernelValue = pressureKernel(diffVector);
	dvec3 *viscosityKernelValue = viscosityKernel(diffVector);

	double forceX = 0.0;
	double forceY = 0.0;
	double forceZ = 0.0;

//	force = -(((distance-2)*(distance - 2)*(distance - 2)) - (distance - 2)*4);
	if(nDensity != 0)
	{
		// Force due to viscosity
		// f = viscosity * laplaceOperator(velocity);
		forceX += viscosity * nMass * ((viscosity * nViscosity)/nDensity) * viscosityKernelValue->x;
		forceY += viscosity * nMass * ((viscosity * nViscosity)/nDensity) * viscosityKernelValue->y;
		forceZ += viscosity * nMass * ((viscosity * nViscosity)/nDensity) * viscosityKernelValue->z;
	}

	// Force due to surface tension
	// f = surfaceTensionC * surfaceCurvature * surfaceNormal

	//sanity check
	if(forceX > 1)
		forceX = 1;
	if(forceY > 1)
		forceY = 1;
	if(forceZ > 1)
		forceZ = 1;

	force += vec3(forceX, forceY, forceZ);
	
	delete pressureKernelValue;
	delete viscosityKernelValue;
}

// Predicts the velocity of the particle with its non-pressure forces
void Particle::predictVelocity(double elapsedTime)
{
	velocity += force / mass * elapsedTime;
}

//this is called after all of the paricles have interacted
//in this time step.  This moves the particles according to
//their velocity and how much time has elapsed.

void Particle::updatePosition(double elapsedTime)
{
	#ifndef WEIGHTLESS
	velocity.z += -9.8 * elapsedTime;
	#endif
	
	position += velocity * elapsedTime;

	if(position.z < 0)
	{
		position.z -= velocity.z * elapsedTime * 2;
		velocity.z *= -.2;
	}
}

//The following three kernel functions are used in th egetForceatPoint
//function

dvec3* Particle::pressureKernel(dvec3 r)
{
	dvec3 *tempVect = new dvec3;
	
	double mag = dot(r,r);

	tempVect->x = (45.0/(M_PI*ER*ER*ER*ER*ER*ER)) * ((ER*mag)*
		(ER*mag)*(ER*mag)) * (r.x/mag);
				
	tempVect->y = (45.0/(M_PI*ER*ER*ER*ER*ER*ER)) *
	 	((ER*mag)*(ER*mag)*(ER*mag)) * (r.y/mag);

	tempVect->z = (45.0/(M_PI*ER*ER*ER*ER*ER*ER)) * 
		((ER*mag)*(ER*mag)*(ER*mag)) * (r.z/mag);


	return tempVect;
}

dvec3* Particle::viscosityKernel(dvec3 r)
{
	dvec3 *tempVect = new dvec3;

	double mag = dot(r,r);

	tempVect->x = (45.0/(M_PI*ER*ER*ER*ER*ER*ER)) * (ER*mag);
				
	tempVect->y = (45.0/(M_PI*ER*ER*ER*ER*ER*ER)) * (ER*mag);

	tempVect->z = (45.0/(M_PI*ER*ER*ER*ER*ER*ER)) * (ER*mag);


	return tempVect;
}

double Particle::densityKernel(dvec3 r)
{
	double mag = dot(r,r);
	
	return ((315.0)/(64 * M_PI * ER*ER*ER*ER*ER*ER*ER*ER*ER))*
		(ER*ER - mag*mag)*(ER*ER - mag*mag)*(ER*ER - mag*mag); 

}

// The density is used in several force calculations.
void Particle::calculateDensity(Particle *neighbor)
{
	if(neighbor)
	{
		density += neighbor->mass * densityKernel(neighbor->position);
	}
}

//gives the particle's velocity a random kick
/*
void Particle::perterb()
{
	srand(timer(NULL));

}
*/
