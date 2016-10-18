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
#include "../util/uVect.h"

const int DIMENSION = 20;
//#define VISIBLE_TEST  //this tells the program to only make 5 particles in a horizontal line.

//The sort algorithm in the stl algorithms library needs a 
//comparison function to be able to work. This is it.

bool compareZ(Particle* left, Particle* right)
{
	return (left->getPosition()->at(2) < right->getPosition()->at(2));
}

//default constructor.
SPH::SPH()
{
	//dls = new vector <GLuint> (3);
	//createDL(1,10);	
	frameTimer = new timer;
	timeLastFrame = frameTimer->elapsed();
}

//this constructor creates a vector the size of 
//particles and initializes them with random positions
//and velocities inside a bounding cube of size 4
SPH::SPH(int particles)
{
	
	//dls = new vector <GLuint> (3);
	frameTimer = new timer;


	double randX, randY, randZ;
	double randI, randJ, randK; //velocity vector values
	
	srand(time(0));

	
	/*#ifdef VISIBLE_TEST //only do 5 particles in a line
	particleCount = 5;
	material = new vector<Particle*>(particleCount);
	for(int i = 0; i < 5; i++)
	{
		material->at(i) = new Particle();
		material->at(i)->setDL(dls->at(0));
		material->at(i)->setPosition(0, i/2.0, 0);
		material->at(i)->setMass(1);
	}

	#endif*/



	particleCount = particles;

	createVAO(particleCount);

	//material = new vector<Particle*>(particles);

	/*for(int i = 0; i < particles; i++)
	{
		randX = ((double)rand()/(double)RAND_MAX) * 1.0;
		randY = ((double)rand()/(double)RAND_MAX) * 1.0;
		randZ = ((double)rand()/(double)RAND_MAX) * 1.0;
		
		randI = (((double)rand()/(double)RAND_MAX) * 0.2) - 0.1;
		randJ = (((double)rand()/(double)RAND_MAX) * 0.2) - 0.1;
		randK = (((double)rand()/(double)RAND_MAX) * 0.2) - 0.1;
		material->at(i) = new Particle();
		material->at(i)->setDL(dls->at(0));
		material->at(i)->setPosition(randX, randY, randZ);
		material->at(i)->setVelocity(randI, randJ, randK);
		material->at(i)->setMass(5);
	}*/

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

	uVect *primaryTempUVect;
	uVect *secondaryTempUVect;
	vector <double> *primaryPositionVector;
	vector <double> *secondaryPositionVector;
	vector <double> vel;

	//the vector with the Particles in it has to 
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
	
	uVect *primaryTempUVect;
	uVect *secondaryTempUVect;

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


int SPH::display(int particles)	
{
	int index = 0;
	int success = 0;
	bool cont = true;
	
	//this is used to log the elapsed time since 
	//the last frame
	double currentTime = frameTimer->elapsed();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // make background black
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(vao);

	glEnable(GL_PROGRAM_POINT_SIZE); //enable gl_PointSize in vertex shader
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

	glDrawArrays(GL_POINTS, 0, particles);

	/*if ((currentTime - timeLastFrame) > 0) {
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
		}*/

	return success;

}

void SPH::createVAO ( int particles ) {
	// A VAO (Vertex Array Object) stores information of a complete rendered object.
	// It contains all VBOs (Vertex Buffer Objects)
	// A VBO stores information about the vertices. 
	// Now we're using two VBOs, one for coordinates and one for colors
	GLfloat vertices[particles][3];
	GLfloat colors[particles][3];

	// generate random positions for all vertices and set the color
	for(int i = 0; i < particles; i++) {
		vertices[i][0] = ((float)rand()/(float)RAND_MAX) * 1.0 - 0.5;
		vertices[i][1] = ((float)rand()/(float)RAND_MAX) * 1.0 - 0.5;
		vertices[i][2] = ((float)rand()/(float)RAND_MAX) * 1.0 - 0.5;;
		
		colors[i][0] = 1.0;
		colors[i][1] = 1.0;
		colors[i][2] = 1.0;
		/*cout << vertices[i][0] << " "
			 <<	vertices[i][1] << " "
			 << vertices[i][2] << "\n";*/
	}

	// Allocate and bind Vertex Array Object to the handle vao
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Generate (one) new Vertex Buffer Object and get the associated id
	glGenBuffers(2, vbo);

	// Bind the first VBO as being the active buffer and storing vertex attributes (coordinates)
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);

	// Copy the vertex data from diamond to our buffer 
    // 8 * sizeof(GLfloat) is the size of the diamond array, since it contains 8 GLfloat values 
    glBufferData(GL_ARRAY_BUFFER, 3 * particles * sizeof(GLfloat), vertices,/* 9 * sizeof(GLfloat), diamond, */ GL_STATIC_DRAW);

    // Specify that our coordinate data is going into attribute index 0, and contains three floats per vertex 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Enable attribute index 0 as being used 
    glEnableVertexAttribArray(0);

    // Bind the second VBO as being the active buffer and storing vertex attributes (colors)
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);

    glBufferData(GL_ARRAY_BUFFER, 3 * particles * sizeof(GLfloat), colors, GL_STATIC_DRAW);

    // Specify that our color data is going into attribute index 1, and contains three floats per vertex 
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Enable attribute index 1 as being used
    glEnableVertexAttribArray(1);

}

void SPH::setTimer(timer *newTimer)
{
	frameTimer = newTimer;
}



















