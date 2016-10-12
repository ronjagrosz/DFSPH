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

class uVect;
class SPH;

using namespace std;
using namespace boost;

struct rect
{
	float width;
	float height;
	float x;
	float y;
};

class Viewport
{


	protected:

		static SPH 		*hydro;
//		vector <mesh> 		objects;
		static glm::vec3 	*cameraPosition;
		static uVect 		*cameraOrientation;
		static rect 		*viewPaneSize;
		static int 			mouseButtonState;
		static glm::vec3 	*mousePosition;
		static timer		*timeSinceStart;	

		

		/***********************************************************************
		Because of my decision to use C++ with OpenGL I had to make extensive
		use of static funtions, which are all protected.  The reason for all
		of the static functions is that OpenGL is a C library and uses function*'s
		for all of it's callbacks. Since function* != Viewport::function* I had to 
		use either use protected static functions or recompile OpenGL as a C++
		library.			
		***********************************************************************/

		static void init(void);					//Initializes a lot of openGL features, mostly just glEnable calls
		static void setupPerspective(GLFWwindow *window, GLfloat *P);			//OpenGL window reshape callback
		static void initWorld();
		static void displayFPS(GLFWwindow *window);
		void controlView(GLFWwindow *window);


/*Disabled until needed*/

//		static void mouseButtonEvent(int button, int state, int x, int y);
//		static void mouseActiveMove(int x,int y);
//		static void mousePassiveMove(int x, int y);
		
//		static void detectCollisions(stellarBody*,stellarBody*);	//I chose to implement a collision detection algorithm
		

	public:	
		Viewport();
		~Viewport();
		virtual int start(int argc, char** argv);			//This functions is called to start the program.

	private:
		float phi;
		float theta;
		float rad;

		float zoomFactor;

		double newTime;
		double deltaTime;
		double currTime;

};

#endif

