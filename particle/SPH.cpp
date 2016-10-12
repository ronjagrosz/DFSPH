/*************************************************************************
Name:       Isabell Jansson, Jonathan Bosson, Ronja Grosz	
File name: 	SPH.cpp

SPH is responsible for orginization of a group of smooth particles.
*************************************************************************/

#include <string.h>
#include <cstdlib>
#include <stdio.h>

#include <iostream>
#include <vector>
#include <stack>
#include <algorithm>
#include <ctime>

#include "../particle/SPH.h"

const int DIMENSION = 20;
//#define VISIBLE_TEST  //this tells the program to only make 5 particles in a horizontal line.

//The sort algorithm in the stl algorithms library needs a 
//comparison function to be able to work. This is it.

bool compareZ(SmoothedParticle* left, SmoothedParticle* right)
{
	return (left->getPosition()->at(2) < right->getPosition()->at(2));
}

//default constructor.
SPH::SPH()
{
	dls = new vector <GLuint> (3);
	//createVBO();	
	frameTimer = new timer;
	timeLastFrame = frameTimer->elapsed();
}

//this constructor creates a vector the size of 
//particles and initializes them with random positions
//and velocities inside a bounding cube of size 4
SPH::SPH(int particles)
{
	dls = new vector <GLuint> (3);
	frameTimer = new timer;
	//createVBO();
	double randX, randY, randZ;
	double randI, randJ, randK; //velocity vector values
	
	srand(time(0));

	particleCount = particles;
	material = new vector<SmoothedParticle*>(particles);

	for(int i = 0; i < particles; i++)
	{
		randX = ((double)rand()/(double)RAND_MAX) * 4.0;
		randY = ((double)rand()/(double)RAND_MAX) * 4.0;
		randZ = ((double)rand()/(double)RAND_MAX) * 4.0;
		
		randI = (((double)rand()/(double)RAND_MAX) * 0.2) - 0.1;
		randJ = (((double)rand()/(double)RAND_MAX) * 0.2) - 0.1;
		randK = (((double)rand()/(double)RAND_MAX) * 0.2) - 0.1;
		material->at(i) = new SmoothedParticle();
		material->at(i)->setDL(dls->at(0));
		material->at(i)->setPosition(randX, randY, randZ);
		material->at(i)->setVelocity(randI, randJ, randK);
		material->at(i)->setMass(5);
	}

	timeLastFrame = frameTimer->elapsed();
}

//this constructor is the same as above, but instead
//of random positions it places the particles in a 
//nice grid pattern with random velocities
SPH::SPH(int particles, int cube)
{
	particleCount = DIMENSION*DIMENSION*DIMENSION;
	dls = new vector <GLuint> (3);
	frameTimer = new timer;
	//createVBO();
	srand(time(0));

	double randI, randJ, randK; //velocity vector values

	material = new vector<SmoothedParticle*>(DIMENSION*DIMENSION*DIMENSION);

	for(int i = 0; i < DIMENSION; i++)
	{
		for(int j = 0; j < DIMENSION; j++)
		{
			for(int k = 0; k <  DIMENSION; k++)
			{
				randI = (((double)rand()/(double)RAND_MAX) * 0.2) - 0.1;
				randJ = (((double)rand()/(double)RAND_MAX) * 0.2) - 0.1;
				randK = (((double)rand()/(double)RAND_MAX) * 0.2) - 0.1;

				material->at(DIMENSION*DIMENSION*i + DIMENSION*j + k) = new SmoothedParticle();
				material->at(DIMENSION*DIMENSION*i + DIMENSION*j + k)->setDL(dls->at(0));
				material->at(DIMENSION*DIMENSION*i + DIMENSION*j + k)->setPosition(i/(DIMENSION/5.0), j/(DIMENSION/5.0), k/(DIMENSION/5.0));
				material->at(i)->setVelocity(randI, randJ, randK);
				material->at(DIMENSION*DIMENSION*i + DIMENSION*j + k)->setMass(5);
			}
		}
	}
	for(int i = 0; i < DIMENSION*DIMENSION*DIMENSION; i++)
	{
		if(material->at(i) == 0)
			cout << i << endl;


	}

	timeLastFrame = frameTimer->elapsed();
}
SPH::~SPH()
{
	for(int i = 0; i < particleCount; i++)
	{
		delete material->at(i);
	}
	delete material;
	delete dls;
	delete frameTimer;

}

//this is one of the most important functions in the 
//program
void SPH::applyForces(double timeDiff)
{
	double distance = 0;

	glm::vec4 *primaryTempUVect;
	glm::vec4 *secondaryTempUVect;
	vector <double> *primaryPositionVector;
	vector <double> *secondaryPositionVector;
	vector <double> vel;

	//the vector with the SmoothedParticles in it has to 
	//be sorted for the distance pruning to work
	sort(material->begin(), material->end(), compareZ);
	calculateDensity();

	for(int i = 0; i < particleCount; i++)
	{
		//primary is the particle we will be comparing the rest to
		primaryPositionVector = material->at(i)->getPosition(); 
		for(int j = i + 1; j < particleCount; j++)
		{
			//secondary is a particle down the line	
			secondaryPositionVector = material->at(j)->getPosition();
			
			//get the distance between the primary and secondary particles
			if(primaryPositionVector && secondaryPositionVector)
			{
				distance = ((primaryPositionVector->at(0) - secondaryPositionVector->at(0))*
					    (primaryPositionVector->at(0) - secondaryPositionVector->at(0))+
					    (primaryPositionVector->at(1) - secondaryPositionVector->at(1))*
					    (primaryPositionVector->at(1) - secondaryPositionVector->at(1))+
					    (primaryPositionVector->at(2) - secondaryPositionVector->at(2))*
					    (primaryPositionVector->at(2) - secondaryPositionVector->at(2)));
			}
				
			//if the distance is less then the effective radius
			if(distance <= ER)
			{
				//get the forces that the two particles enact on each other
				primaryTempUVect = material->at(i)->getForceAtPoint(material->at(j));
				secondaryTempUVect = material->at(j)->getForceAtPoint(material->at(i));
			
				//now apply those forces
				if(primaryTempUVect && secondaryTempUVect)
				{
					material->at(i)->applyForce(*secondaryTempUVect, timeDiff);
					material->at(j)->applyForce(*primaryTempUVect, timeDiff);
						
				}
	
				delete secondaryPositionVector;
				delete primaryTempUVect;
				delete secondaryTempUVect;
			} else 
			{
				delete secondaryPositionVector;
					
				break;
			}
		}
		
		delete primaryPositionVector;
	}

	//all of the forces have been calculated.
	//the velocities for this time step have
	//been updated, now move the particles
	for (int i = 0; i < particleCount; i++)
	{
		material->at(i)->updatePosition(timeDiff);
		material->at(i)->zeroDensity();
	}
}

//this is effectively the same algorith as above, but instead of 
//getting and applying force it just calls the particle's 
//calculate density function.

void SPH::calculateDensity()
{
	double distance = 0;
	
	glm::vec4 *primaryTempUVect;
	glm::vec4 *secondaryTempUVect;

	vector <double> *primaryPositionVector;
	vector <double> *secondaryPositionVector;

	for(int i = 0; i < particleCount; i++)
	{
		primaryPositionVector = material->at(i)->getPosition();
		for(int j = 0; j < particleCount; j++)
		{
			secondaryPositionVector = material->at(j)->getPosition();
			if(primaryPositionVector && secondaryPositionVector)
			{
				distance = ((primaryPositionVector->at(0) - secondaryPositionVector->at(0))*
					    (primaryPositionVector->at(0) - secondaryPositionVector->at(0))+
					    (primaryPositionVector->at(1) - secondaryPositionVector->at(1))*
					    (primaryPositionVector->at(1) - secondaryPositionVector->at(1))+
					    (primaryPositionVector->at(2) - secondaryPositionVector->at(2))*
					    (primaryPositionVector->at(2) - secondaryPositionVector->at(2)));
			}
				
			if(distance <= ER*ER)
			{
				if(primaryPositionVector && secondaryPositionVector)
				{
					material->at(i)->calculateDensity(material->at(j));			
				}
	
				delete secondaryPositionVector;
				delete primaryTempUVect;
				delete secondaryTempUVect;
			} else 
			{
				delete secondaryPositionVector;
	
				break;
			}
				
		}
		delete primaryPositionVector;
	}
	for(int i = 0; i<particleCount; i++)
	{
		material->at(i)->clearNAN();
//		material->at(i)->printDensity();
	}

}

//this is SPH's entrypoint each frame
int SPH::display()	
{
	/*
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 3 * ntris, GL_UNSIGNED_INT, (void*)0);
	// (mode, vertex count, type, element array buffer offset)
	glBindVertexArray(0);
	*/
	int index = 0;
	int success = 0;
	bool cont = true;
	
	//this is used to log the elapsed time since 
	//the last frame
	double currentTime = frameTimer->elapsed();
	
	success = 0;
	
	
	if((currentTime - timeLastFrame) > 0)
		applyForces(currentTime - timeLastFrame); //move the particles

	if ((currentTime - timeLastFrame) > 0) {
		while (cont == true) {
			try {
				if((unsigned int)index < material->capacity()) {
					if(material->at(index))
						material->at(index)->display(timeLastFrame); // display particle
					index++;
				}
			}
			catch(char *str) {
				if(strcmp(str, "out_of_range")) {
					cont = false;
					break;
				}
				else {
					cout << "caught exception " << str << " ending program" << endl;
					exit(1);
				}

			}
			if(index >= particleCount)
				cont = false;


			//update the time.
			timeLastFrame = frameTimer->elapsed();
			success = 1;
		}
	}
	return success;
}

void SPH::setTimer(timer *newTimer)
{
<<<<<<< HEAD:particle/sph.cpp
	using namespace std;
	int n;
	double a;
	double b;
	
	int VertexCount = (90/space)*(360/space)*4;
	VERTICES *VERTEX = new VERTICES[VertexCount];

	n = 0;
	for( b = 0; b <= 90 - space; b+=space)
	{

		for( a = 0; a <= 360 - space; a+=space)
		{



			VERTEX[n].X = radius * sin((a) / 180 * M_PI) * sin((b) / 180 * M_PI) - H;
			VERTEX[n].Y = radius * cos((a) / 180 * M_PI) * sin((b) / 180 * M_PI) + K;
			VERTEX[n].Z = radius * cos((b) / 180 * M_PI) - Z;
			VERTEX[n].V = (2 * b) / 360;
			VERTEX[n].U = (a) / 360;
			n++;

			VERTEX[n].X = radius * sin((a) / 180 * M_PI) * sin((b + space) / 180 * M_PI) - H;
			VERTEX[n].Y = radius * cos((a) / 180 * M_PI) * sin((b + space) / 180 * M_PI) + K;
			VERTEX[n].Z = radius * cos((b + space) / 180 * M_PI) - Z;
			VERTEX[n].V = (2 * (b + space)) / 360;
			VERTEX[n].U = (a) / 360;
			n++;
			
			if (a < 360 -space)			//this is an added conditional
			{					//if this is not the end of the strip, business as usual
				VERTEX[n].X = radius * sin((a + space) / 180 * M_PI) * sin((b) / 180 *
                M_PI) - H;
				VERTEX[n].Y = radius * cos((a + space) / 180 * M_PI) * sin((b) / 180 *
                M_PI) + K;
				VERTEX[n].Z = radius * cos((b) / 180 * M_PI) - Z;
				VERTEX[n].V = (2 * b) / 360;
				VERTEX[n].U = (a + space) / 360;
				n++;

				VERTEX[n].X = radius * sin((a + space) / 180 * M_PI) * sin((b + space) /180 *
                M_PI) - H;
				VERTEX[n].Y = radius * cos((a + space) / 180 * M_PI) * sin((b + space) /180 *
                M_PI) + K;
				VERTEX[n].Z = radius * cos((b + space) / 180 * M_PI) - Z;
				VERTEX[n].V = (2 * (b + space)) / 360;
				VERTEX[n].U = (a + space) / 360;
				n++;
			}
			else if (a >= 360 - space)		//however if the end of the strip has been reached, set the two end points 
			{					//to be equal to the two begining points.
				VERTEX[n].X = VERTEX[0].X;
				VERTEX[n].Y = VERTEX[0].Y;
				VERTEX[n].Z = VERTEX[0].Z;
				VERTEX[n].V = VERTEX[0].U;
				VERTEX[n].U = VERTEX[0].V;
				n++;

				VERTEX[n].X = VERTEX[1].X;
				VERTEX[n].Y = VERTEX[1].Y;
				VERTEX[n].Z = VERTEX[1].Z;
				VERTEX[n].V = VERTEX[1].U;
				VERTEX[n].U = VERTEX[1].V;
				n++;


			}

		}
	}
	return VERTEX;
}

void sph::setTimer(timer *newTimer)
{
	frameTimer = newTimer;
}
















