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

Particle::Particle()
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

void Particle::setVelocity(dvec3 vel)
{
	velocity = vel;
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
void Particle::setDensity(double newDensity)
{
	density = newDensity;
}

// Getters  ***************************************************************
dvec3 Particle::getPosition(){return position;}
dvec3 Particle::getVelocity(){return velocity;}
dvec3 Particle::getForce(){return force;}
vec3 Particle::getColor(){return color;}
double Particle::getDensity(){return density;}
double Particle::getStiffness(){return stiffness;}

// Predicts the velocity of the particle with its non-pressure forces
void Particle::predictVelocity(double elapsedTime)
{
	velocity += force * elapsedTime;
}

// Update position with current velocity
void Particle::updatePosition(double elapsedTime)
{
	position += velocity * elapsedTime;
	
	if(position.y < 0)
	{
		position.y -= velocity.y * elapsedTime * 2;
		velocity.y *= -.2;
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
	/*if(neighbor)
	{
		density += neighbor->mass * densityKernel(neighbor->position);
	}*/
}

//gives the particle's velocity a random kick
/*
void Particle::perterb()
{
	srand(timer(NULL));

}
*/
