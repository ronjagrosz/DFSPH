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

	velocity  = dvec3(0.0, 0.0, 0.0);
	force = dvec3(0.0, 0.0, 0.0);

	density = 0;
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

// Update position with current velocity
void Particle::updatePosition(double elapsedTime)
{
	position += velocity * elapsedTime;	
}

//The following three kernel functions are used in th egetForceatPoint
//function

dvec3* Particle::pressureKernel(dvec3 r)
{
	dvec3 *tempVect = new dvec3;
	
	double mag = dot(r,r);

	tempVect->x = (45.0/(M_PI*H*H*H*H*H*H)) * ((H*mag)*
		(H*mag)*(H*mag)) * (r.x/mag);
				
	tempVect->y = (45.0/(M_PI*H*H*H*H*H*H)) *
	 	((H*mag)*(H*mag)*(H*mag)) * (r.y/mag);

	tempVect->z = (45.0/(M_PI*H*H*H*H*H*H)) * 
		((H*mag)*(H*mag)*(H*mag)) * (r.z/mag);


	return tempVect;
}

dvec3* Particle::viscosityKernel(dvec3 r)
{
	dvec3 *tempVect = new dvec3;

	double mag = dot(r,r);

	tempVect->x = (45.0/(M_PI*H*H*H*H*H*H)) * (H*mag);
				
	tempVect->y = (45.0/(M_PI*H*H*H*H*H*H)) * (H*mag);

	tempVect->z = (45.0/(M_PI*H*H*H*H*H*H)) * (H*mag);


	return tempVect;
}

double Particle::densityKernel(dvec3 nPosition)
{
	// Cubic spline kernel
	double q = sqrt(dot(position-nPosition, position-nPosition))/H;
	//cout << "dist: " << sqrt(dot(position-nPosition, position-nPosition)) << "\n";
	if (position - nPosition == dvec3(0.0, 0.0, 0.0))
		return 1.0;
	else if ( q >= 0 && q < 1)
		return (1/(H*H*H))*(1/M_PI)*(1 - 3/2*q*q + 3/4*q*q*q);
	else if ( q >= 1 && q < 2 )
		return (1/(H*H*H))*(1/M_PI)*(1/4*(2-q)*(2-q)*(2-q));
	else 
		return 0;
}

dvec3 Particle::gradientDensityKernel(dvec3 nPosition)
{
	// Cubic spline kernel
	
	double q = sqrt(dot(position-nPosition, position-nPosition))/H;

	if (position - nPosition == dvec3(0.0, 0.0, 0.0))
		return dvec3(1.0, 1.0, 1.0);
	else if ( q >= 0 && q < 1)
		return position*(1/(H*H*H*H))*(1/length(position))*(1/M_PI)*(- 3*q + 9/4*q*q);
	else if ( q >= 1 && q < 2 )
		return position*(1/(H*H*H*H))*(1/length(position))*(1/M_PI)*(-3/4*(2-q)*(2-q));
	else 
		return dvec3(0.0, 0.0, 0.0);

}

// The density is used in several force calculations.
void Particle::calculateDensity(Particle *neighbor)
{
	//cout << "Par: " << position.x << " " << neighbor->position.x << "\n";
	if(neighbor)
	{
		density += particleMass * densityKernel(neighbor->position);
	}
}

//gives the particle's velocity a random kick
/*
void Particle::perterb()
{
	srand(timer(NULL));

}
*/
