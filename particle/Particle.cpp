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

Particle::Particle() {
	neighbours = new vector<int>;

	velocity  = dvec3(0.0, 0.0, 0.0);
	force = dvec3(0.0, 0.0, 0.0);

	density = 0;
}

Particle::~Particle() {
	delete neighbours;
}

// Setters  ***************************************************************
void Particle::setPosition(float x, float y, float z) {
	position = dvec3(x, y, z);
}

void Particle::setVelocity(dvec3 vel) {
	velocity = vel;
}
void Particle::setForce(double i, double j, double k) {
	force = dvec3(i, j, k);
}
void Particle::setColor(vec3 newColor) {
	if(newColor != color)
		color = newColor;
}
void Particle::setCellIndex(ivec4 cell) {
    cellIndex = cell;
}
void Particle::setDensity(double newDensity) {
	density = newDensity;
}
void Particle::setdDensity(double newDensity) {
	dDensity = newDensity;
}
void Particle::setAlpha(double newAlpha) {
	alpha = newAlpha;
}

// Getters  ***************************************************************
dvec3 Particle::getPosition(){return position;}
dvec3 Particle::getVelocity(){return velocity;}
dvec3 Particle::getForce(){return force;}
vec3 Particle::getColor(){return color;}
ivec4 Particle::getCellIndex(){return cellIndex;}
vector<int>* Particle::getNeighbours(){return neighbours;}
double Particle::getDensity(){return density;}
double Particle::getdDensity(){return dDensity;}
double Particle::getAlpha(){return alpha;}
double Particle::getStiffness(){return stiffness;}

// Update position with current velocity
void Particle::updatePosition(double elapsedTime) {
	position += velocity * elapsedTime;	
}
// Update neighbour list
void Particle::updateNeighbours(vector<int>* neighbourList) {
    neighbours = neighbourList;
}
// Cubic spline kernel function
double Particle::kernel(dvec3 nPosition, double H) {
	double q = sqrt(dot(position-nPosition, position-nPosition))/H;

	if (position - nPosition == dvec3(0.0, 0.0, 0.0)) 
		return 1.0;
	else if ( q >= 0 && q <= 1)
		return (1/(H*H*H)*(1/M_PI)*(1 - 3/2*q*q + 3/4*q*q*q));
	else if ( q >= 1 && q <= 2 ) 
		return (1/(H*H*H))*(1/M_PI)*(1/4*(2-q)*(2-q)*(2-q));
	else 
		return 0;
}
// Gradient for cubic spline kernel function
dvec3 Particle::gradientKernel(dvec3 nPosition, double H) {
	double q = sqrt(dot(position-nPosition, position-nPosition))/H;

	if (position - nPosition == dvec3(0.0, 0.0, 0.0))
		return dvec3(1.0, 1.0, 1.0);
	else if ( q >= 0 && q <= 1)
		return position*(1/(H*H*H*H))*(1/length(position))*(1/M_PI)*(- 3*q + 9/4*q*q);
	else if ( q >= 1 && q <= 2 )
		return position*(1/(H*H*H*H))*(1/length(position))*(1/M_PI)*(-3/4*(2-q)*(2-q));
	else 
		return dvec3(0.0, 0.0, 0.0);
}
