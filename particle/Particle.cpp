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

#define CONST_FORCE_CONST 1
//#define WEIGHTLESS

using namespace std;

Particle::Particle():radius(1),mass(1),viscosity(2.034),materialID(WATER),
forceConstant(CONST_FORCE_CONST),threshold(0.5),stretchR(1),stretchA(1),
offsetR(0),offsetA(0),maxR(100),maxA(-100)
{
	position = new vector <double> (3);
	neighbors = new stack<int>;
	velocity  = new glm::vec4(0,0,0,1);
	color = new vector<int> (3);
}


Particle::Particle(const Particle& clone):radius(1),mass(1),
viscosity(2.034),materialID(WATER),forceConstant(CONST_FORCE_CONST),threshold(0.5),stretchR(1),
stretchA(1),offsetR(0),offsetA(0),maxR(100),maxA(-100)
{
	position = new vector<double> (*clone.position);
	neighbors = new stack<int> ;
	velocity = new glm::vec4(*clone.velocity);
	radius = clone.radius;
	mass = clone.mass;
	materialID = clone.materialID;

	color = new vector<int> (*clone.color);
	pressureScale = clone.pressureScale;
}

Particle::~Particle()
{
	delete position;
	delete neighbors;
	delete velocity;
	delete color;
}

void Particle::display(double oldFrameTime)
{
	timeLastFrame = oldFrameTime;

	glPushMatrix();
//	glColor4f(1.0,1.0,1.0,0.0);
	glTranslated(position->at(0), position->at(1), position->at(2));

	if(DL != 0)
	{
		glCallList(DL);
	}
	glPopMatrix();
}



//setters
void Particle::setPosition(double x, double y, double z)
{
	position->at(0) = x;
	position->at(1) = y;
	position->at(2) = z;


}

void Particle::setVelocity(double i, double j, double k)
{
	
	delete velocity;
	glm::vec4 *newVelocity = new glm::vec4(i, j, k, 1.0);
	velocity = newVelocity;


}
void Particle::setRadius(double newRadius)
{
	radius = newRadius;
}
void Particle::setMass(double newMass)
{
	mass = newMass;
}
void Particle::setMaterialID(double newID)
{
	materialID = newID;
}
void Particle::setColor(vector<int>* newColor)
{
	if(newColor != color)
	{
		color = newColor;
	}
}
void Particle::setPressureScale(float newScale)
{
	pressureScale = newScale;
}

void Particle::setDL(GLuint newDL){DL = newDL;}
void Particle::setTimer(timer *currentTime){frameTimer = currentTime;}

//getters  ***************************************************************
vector<double>* Particle::getPosition()
{
	vector <double> *tempV = new vector <double> (*position);
	return tempV;
}

vector<int>* Particle::getColor()
{
	vector<int>* tempV = new vector<int> (*color);
	return tempV;

}

glm::vec4* Particle::getVelocity()
{
	glm::vec4 *tempU = new glm::vec4(*velocity);
	return tempU;
}
double Particle::getRadius(){return radius;}
double Particle::getMass(){return mass;}
double Particle::getMaterialID(){return materialID;}
float Particle::getPressurescale(){return pressureScale;}
GLuint Particle::getDL(){return DL;}

/************************************************************************/
//getForceAtPoint is the heart of this program.  This function dictates the
//interaction between particles, which is what SPH is all about.
//
//
//return value: The force that is returned by this function is the force
//		that this particle enacts on its neighbor.
//
/************************************************************************/
glm::vec4* Particle::getForceAtPoint(Particle *neighbor)
{

	//all variables that are prefixed with the letter n are values that
	//belong to the neighboring particle.  This convention is used to 
	//shorten expression length.
	//eg. neighbor->position->at(0) = nPosition->at(0)
	
	vector <double> *nPosition = neighbor->position;	//do NOT delete this vector
	
	double k = .0000001;

	double distance = 0.0;

	double forceX = 0.0;
	double forceY = 0.0;
	double forceZ = 0.0;
	
	double nMass = neighbor->mass;
	double nDensity = neighbor->density;
	double nViscosity = neighbor->viscosity;

	double pressure = .1; //+ k*(density - nDensity);
	double nPressure = .1; //+ k*(nDensity - density);

	vector <double> diffVector(3);

	//the vector representation of the distance between these
	//two points
	diffVector.at(0) = nPosition->at(0) - position->at(0);
	diffVector.at(1) = nPosition->at(1) - position->at(1);
	diffVector.at(2) = nPosition->at(2) - position->at(2);

	vector <double> *pressureKernelValue = pressureKernel(&diffVector);
	vector <double> *viscosityKernelValue = viscosityKernel(&diffVector);

	//this is the distance between the two particles
	distance = sqrt(diffVector.at(0)*diffVector.at(0) +
			diffVector.at(1)*diffVector.at(1) + 
			diffVector.at(2)*diffVector.at(2));

//	force = -(((distance-2)*(distance - 2)*(distance - 2)) - (distance - 2)*4);
	if(nDensity != 0)
	{
		//force due to pressure
		forceX = -1.0 * nMass * ((nPressure + pressure)/(2*nDensity)) * densityKernel(nPosition);//pressureKernelValue->at(0);
		forceY = -1.0 * nMass * ((nPressure + pressure)/(2*nDensity)) * densityKernel(nPosition);//pressureKernelValue->at(1);
		forceZ = -1.0 * nMass * ((nPressure + pressure)/(2*nDensity)) * densityKernel(nPosition);//pressureKernelValue->at(2);

		//force due to viscosity
		forceX += viscosity * nMass * ((viscosity * nViscosity)/nDensity) * viscosityKernelValue->at(0);
		forceY += viscosity * nMass * ((viscosity * nViscosity)/nDensity) * viscosityKernelValue->at(1);
		forceZ += viscosity * nMass * ((viscosity * nViscosity)/nDensity) * viscosityKernelValue->at(2);
		
		//cout << forceX << endl;
		
	}
	else
	{
		forceX = forceY = forceZ = 0;
	}

	

	//making the return vector
	if(distance != 0)
	{
		//sanity check
		if(forceX > 1)
			forceX = 1;
		if(forceY > 1)
			forceY = 1;
		if(forceZ > 1)
			forceZ = 1;

		glm::vec4 *tempVec = new glm::vec4(forceX, forceY, forceZ, 1);

		delete pressureKernelValue;
		delete viscosityKernelValue;
		return tempVec;
	}

	delete pressureKernelValue;
	delete viscosityKernelValue;
	return NULL;
}

//this function takes a force, and applies it to the velocity of the particle

void Particle::applyForce(glm::vec4 &actingForce, double elapsedTime)
{
	velocity->x += (actingForce.x / mass) * elapsedTime;
	velocity->y += (actingForce.y / mass) * elapsedTime;
	velocity->z += (actingForce.z / mass) * elapsedTime;
}

//this is called after all of the paricles have interacted
//in this time step.  This moves the particles according to
//their velocity and how much time has elapsed.

void Particle::updatePosition(double elapsedTime)
{
	#ifndef WEIGHTLESS
	velocity->z += -9.8 * elapsedTime;
	#endif
	
	position->at(0) += velocity->x * elapsedTime;	
	position->at(1) += velocity->y * elapsedTime;	
	position->at(2) += velocity->z * elapsedTime;	
	
	//cout << position->at(0) << endl;

	if(position->at(2) < 0)
	{
		position->at(2) -= velocity->z * elapsedTime * 2;
		velocity->z *= -.2;

	}
}

//The following three kernel functions are used in th egetForceatPoint
//function

vector <double>* Particle::pressureKernel(vector <double> *r)
{
	vector <double> *tempVect = new vector <double> (3);
	
	double mag = 	sqrt(r->at(0)*r->at(0)+
			r->at(1)*r->at(1)+
			r->at(2)*r->at(2));

	tempVect->at(0) = (45.0/(M_PI*ER*ER*ER*ER*ER*ER)) * ((ER*mag)*
		(ER*mag)*(ER*mag)) * (r->at(0)/mag);
				
	tempVect->at(1) = (45.0/(M_PI*ER*ER*ER*ER*ER*ER)) *
	 	((ER*mag)*(ER*mag)*(ER*mag)) * (r->at(1)/mag);

	tempVect->at(2) = (45.0/(M_PI*ER*ER*ER*ER*ER*ER)) * 
		((ER*mag)*(ER*mag)*(ER*mag)) * (r->at(2)/mag);


	return tempVect;
}

vector <double>* Particle::viscosityKernel(vector <double> *r)
{
	vector <double> *tempVect = new vector <double> (3);

	double mag = 	sqrt(r->at(0)*r->at(0)+
			r->at(1)*r->at(1)+
			r->at(2)*r->at(2));

	tempVect->at(0) = (45.0/(M_PI*ER*ER*ER*ER*ER*ER)) * (ER*mag);
				
	tempVect->at(1) = (45.0/(M_PI*ER*ER*ER*ER*ER*ER)) * (ER*mag);

	tempVect->at(2) = (45.0/(M_PI*ER*ER*ER*ER*ER*ER)) * (ER*mag);


	return tempVect;
}

double Particle::densityKernel(vector <double> *r)
{
	double mag = 	sqrt(abs(r->at(0)*r->at(0)+
			r->at(1)*r->at(1)+
			r->at(2)*r->at(2)));
	
	return ((315.0)/(64 * M_PI * ER*ER*ER*ER*ER*ER*ER*ER*ER))*
		(ER*ER - mag*mag)*(ER*ER - mag*mag)*(ER*ER - mag*mag); 

}

//this is the first thing that is done to this particle
//the density is used in several force calculations.
//Since this particle is a member of a greater system
//the density that this particle represents should be
//"smoothed" over the surrounding area by taking a 
//weighted average of its neighbors.
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