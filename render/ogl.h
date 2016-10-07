/*************************************************************************
Name:	Casey Beach
Date:	6/27/2010
Terminus: ogl.h

Terminus is meant to be an MMOG (Massively Multiplayer Online Game), as
such it will be, roughly speaking split into two parts, ogl and
server.  The ogl will handle all user interaction and graphics.
*************************************************************************/
#ifndef CLIENT_H
#define CLIENT_H

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <iostream>
#include <vector>
#include <math.h>
#include "boost/timer.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"	
#include "glm/gtx/transform.hpp"
#include "glm/ext.hpp"

class uVect;
class sph;

using namespace std;
using namespace boost;

struct rect
{
	float width;
	float height;
	float x;
	float y;
};

class ogl
{


	protected:

		static sph 		*hydro;
//		vector <mesh> 		objects;
		static vector <double> 	*cameraPosition;
		static uVect 		*cameraOrientation;
		static rect 		*viewPaneSize;
		static int 		mouseButtonState;
		static vector <int> 	*mousePosition;
		static timer		*timeSinceStart;	

		

		/***********************************************************************
		Because of my decision to use C++ with openGL I had to make extensive
		use of static funtions, which are all protected.  The reason for all
		of the static functions is that openGL is a C library and uses function*'s
		for all of it's callbacks. Since function* != ogl::function* I had to 
		use either use protected static functions or recompile openGL as a C++
		library.			
		***********************************************************************/

		static void init(void);					//Initializes a lot of openGL features, mostly just glEnable calls
		static void setupViewport(GLFWwindow *window, GLfloat *P);			//OpenGL window reshape callback
		static void initWorld();
		static void displayFPS(GLFWwindow *window);
		void controlView(GLFWwindow *window);


/*Disabled until needed*/

//		static void keyboardDown(unsigned char key, int x, int y);	//The next six functions are OpenGl callbacks for keyboard and mouse inputs
//		static void keyboardSpecialKey(int key, int x, int y);
//		static void keyboardUp(unsigned char key, int x, int y);
//		static void mouseButtonEvent(int button, int state, int x, int y);
//		static void mouseActiveMove(int x,int y);
//		static void mousePassiveMove(int x, int y);
		
//		static void detectCollisions(stellarBody*,stellarBody*);	//I chose to implement a collision detection algorithm
		

	public:	
		ogl();
		~ogl();
		virtual int start(int argc, char** argv);			//This functions is called to start the program.  Mostly just a generic glut initialization function.

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

