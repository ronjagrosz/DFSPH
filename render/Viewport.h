/*************************************************************************
Name:	    Isabell Jansson, Jonathan Bosson, Ronja Grosz
File name:  Viewport.h

Viewport is used as a OpenGL controller.  Viewport is responsible for managing all OpenGL related activities.
*************************************************************************/
#ifndef CLIENT_H
#define CLIENT_H

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <iostream>
#include <vector>
#include <math.h>
#include "boost/timer.hpp"
#include "../render/BoundingBox.h"


#ifdef __linux__
#include "../glm/glm/glm.hpp"
#include "../glm/glm/gtc/matrix_transform.hpp"
#include "../glm/glm/gtc/type_ptr.hpp"	
#include "../glm/glm/gtx/transform.hpp"
#include "../glm/glm/ext.hpp"
#elif __APPLE__
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"	
#include "glm/gtx/transform.hpp"
#include "glm/ext.hpp"
#endif

class SPH;

using namespace std;
using namespace boost;

struct rect {
	float width;
	float height;
	float x;
	float y;
};

class Viewport {	
	public:	
		Viewport();
		~Viewport();
		virtual int start(int argc, char** argv);			//This functions is called to start the program.

		GLuint vao;

	private:
		// void detectCollisions(stellarBody*,stellarBody*);	//I chose to implement a collision detection algorithm
		int width;
		int height;
		double fps;

		glm::vec3 cameraPosition;
		float phi;
		float theta;
		float rad;

		float zoomFactor;

		double deltaTime;
		double timeSinceAction;
		double recordTime;
		double currTime;
		bool record;
		int frameCount;

		SPH 		*hydro;
		BoundingBox	boundingBox;
		timer		*timeSinceStart;
		//CAVIGenerator mov;
		//BYTE* bm

		void init(void);					//Initializes a lot of openGL features, mostly just glEnable calls
		void setupPerspective(GLFWwindow *window, GLfloat *P);			//OpenGL window reshape callback
		void initWorld();
		void displayFPS(GLFWwindow *window);
		void interaction(GLFWwindow *window);
};

#endif

