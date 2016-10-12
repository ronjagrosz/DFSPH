/*************************************************************************
Name:	    Isabell Jansson, Jonathan Bosson, Ronja Grosz
File name:  ogl.cpp

ogl is used as a OpenGL controller.  ogl is responsible for managing all openGL related activities.
*************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>

#include "../particle/SPH.h"
#include "../render/ogl.h"
#include "../util/uVect.h"
#include "Shader.h"

#define GLUT_SCROLL_UP 		3	//Used in the mouseButtonEvent callback.  freeglut has functionality for mouse scolling, but aparently not the defines.
#define GLUT_SCROLL_DOWN	4	//so I added them to make my code more readable.

const int PARTICLE_COUNT = 10000;	//This variable dictates how many particles will be in the simulation


using namespace std;

//static functions need static variables, which are protected.

vector 	<double> *ogl::cameraPosition;
uVect	*ogl::cameraOrientation;

int 	ogl::mouseButtonState;
vector	<int> *ogl::mousePosition;

rect	*ogl::viewPaneSize;

SPH 	*ogl::hydro;

timer	*ogl::timeSinceStart;
/************************************************************************/

ogl::ogl()
{
	ogl::cameraPosition = new vector <double> (3);
	ogl::mousePosition = new vector <int> (3);

	ogl::cameraOrientation = new uVect(0,0,1,0);
	ogl::viewPaneSize = new rect;
	
	ogl::timeSinceStart = new timer();

	for(int i = 0;i<3;i++)
	{
		ogl::cameraPosition->at(i) = 0;
		ogl::mousePosition->at(i) = 0;
	}
	
	ogl::mouseButtonState = 0;
	
}

ogl::~ogl(){}

/* showFPS() - Calculate and report frames per second
(updated once per second) in the window title bar */
void ogl::displayFPS(GLFWwindow *window)
{
	static double t0 = 0.0;
	static int frames = 0;
	double fps = 0.0;
	double frametime = 0.0;
	static char titlestring[200];

	double t;

	// Get current time
	t = glfwGetTime();  // Gets number of seconds since glfwInit()
	// If one second has passed, or if this is the very first frame
	if ((t - t0) > 1.0 || frames == 0)
	{
		fps = (double)frames / (t - t0);
		if (frames > 0) frametime = 1000.0 * (t - t0) / frames;
		sprintf(titlestring, "SPH, %.2f ms/frame (%.1f FPS)", frametime, fps);
		glfwSetWindowTitle(window, titlestring);
		// printf("Speed: %.1f FPS\n", fps);
		t0 = t;
		frames = 0;
	}
	frames++;
	//return fps;
}

void ogl::init(void)		//enable texture, lighting, shading.
{
    //GL calls
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); // with?

/*
	glClearColor(0.0,0.0,0.0,1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LEQUAL);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	GLfloat ambient[] = {1.0,1.0,1.0,1.0};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,ambient);
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel(GL_SMOOTH);
*/
}

void ogl::initWorld()
{
	ogl::hydro = new SPH(PARTICLE_COUNT);	//this is the object that will manage all of the particles
	ogl::hydro->setTimer(timeSinceStart);	//I'm setting a timer to bind the particles to real time regardless of the coputer that they are run on
}

void ogl::setupViewport(GLFWwindow *window, GLfloat *P)		//just in case some one wants to resize the window
{
	int width, height;
	glfwGetWindowSize(window, &width, &height);

	P[0] = P[5] * height / width;

	glViewport(0, 0, width, height);

}

// Control the camera with the arrow keys. Hold left control button and UP/DOWN for zoom
void ogl::controlView(GLFWwindow *window)
{
	newTime = glfwGetTime();
	deltaTime = newTime - currTime;
	currTime = newTime;

	if (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) || glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) {
		if (glfwGetKey(window, GLFW_KEY_UP)) {
			if (rad > 0.0f)
				rad -= deltaTime*zoomFactor;
		}
		else if (glfwGetKey(window, GLFW_KEY_DOWN)) {
			rad += deltaTime*zoomFactor;
		}
	}
	else {
		if (glfwGetKey(window, GLFW_KEY_UP)) {
			theta += deltaTime*PI / 2.0; // Rotate 90 degrees per second
			if (theta >= PI / 2.0) theta = PI / 2.0; // Clamp at 90
		}
		else if (glfwGetKey(window, GLFW_KEY_DOWN)) {
			theta -= deltaTime*PI / 2.0; // Rotate 90 degrees per second
			if (theta < 0.1f) theta = 0.1f;      // Clamp at -90
		}
	}

	if (glfwGetKey(window, GLFW_KEY_RIGHT)) {
		phi -= deltaTime*PI / 2.0; // Rotate 90 degrees per second (pi/2)
		phi = fmod(phi, PI*2.0); // Wrap around at 360 degrees (2*pi)
		if (phi < 0.0) phi += PI*2.0; // If phi<0, then fmod(phi,2*pi)<0
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT)) {
		phi += deltaTime*PI / 2.0; // Rotate 90 degrees per second (pi/2)
		phi = fmod(phi, PI*2.0);
	}
}

int ogl::start(int argc, char** argv)	//initialize glut and set all of the call backs
{   

    GLfloat I[16] = { 1.0f, 0.0f, 0.0f, 0.0f,
					  0.0f, 1.0f, 0.0f, 0.0f,
					  0.0f, 0.0f, 1.0f, 0.0f,
					  0.0f, 0.0f, 0.0f, 1.0f };
	GLfloat P[16] = { 2.42f, 0.0f, 0.0f, 0.0f,
					  0.0f, 2.42f, 0.0f, 0.0f,
					  0.0f, 0.0f, -1.0f, -1.0f,
					  0.0f, 0.0f, -0.2f, 0.0f };
	GLint locationP;
	GLint locationMV;

	glm::mat4 viewMatrix;

    // start GLEW extension handler
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return 1;
    }
    

#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
  
    //create GLFW window and select context
    GLFWwindow* window = glfwCreateWindow(640, 480, "Fluid Simulation", NULL, NULL);
    if (!window) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    
    //start GLEW extension handler
    glewExperimental = GL_TRUE;
    glewInit();
    
    // get version info
    const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
    const GLubyte* version = glGetString(GL_VERSION); // version as a string
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);

    initWorld();

    
    Shader phongShader;
	phongShader.createShader("glsl/vertexshader.glsl", "glsl/fragmentshader.glsl");
    //link variables to shader
    locationMV = glGetUniformLocation(phongShader.programID, "MV");
	locationP = glGetUniformLocation(phongShader.programID, "P");

	int success = 0;
	phi = 0.0f;
	theta = PI / 4.0f;
	rad = 15.0f;
	zoomFactor = PI;
	newTime = deltaTime = currTime = 0.0f;

    // Let's get started!
    while (!glfwWindowShouldClose(window)) {
    	glfwPollEvents();
    	//GL calls
        init();
        displayFPS(window);
        glUseProgram(phongShader.programID);
		glUniformMatrix4fv(locationP, 1, GL_FALSE, P);

		setupViewport(window, P);
		controlView(window);

		// I is the normal Identity matrix
		viewMatrix = glm::make_mat4(I);
        // Translate a bit down and backwards
		viewMatrix = glm::translate(viewMatrix, glm::vec3(-3.0f, 3.0f, -rad));
        // Rotate with theta around X
        viewMatrix = viewMatrix * glm::rotate(theta, glm::vec3(1.0f, 0.0f, 0.0f));
        // Rotate with phi around Y
        viewMatrix = viewMatrix * glm::rotate(phi, glm::vec3(0.0f, 1.0f, 0.0f));

        //convert viewMatrix to float
        glUniformMatrix4fv(locationMV, 1, GL_FALSE, glm::value_ptr(viewMatrix));

        // viewMatrix is the I matrix after all operations
        /*
        li = glm::vec4(0.0, 5.0, 0.0, 1.0);
        cam = glm::vec4(0.0, 0.0, 0.0, 1.0);
        li = glm::inverse(viewMatrix)*li;
        cam = glm::inverse(viewMatrix)*cam;
        
        Ca[0] = cam.x;
        Ca[1] = cam.y;
        Ca[2] = cam.z;
        L[0] = li.x;
        L[1] = li.y;
        L[2] = li.z;
        // send in light and camera location to glsl (not done in shaders yet)
        glUniform3fv(locationL, 1, L);
        glUniform3fv(locationCa, 1, Ca);
        */

		success = hydro->display();	//the success variable is used because the timer function in linux only has
					//a resolution of .001 sec.  For small numbers of particles I can get a LOT
					//better performance then this, so the frame rate sky rockets and particles
					//whiz around to fast to be seen.
		//if(success)
			glfwSwapBuffers(window);			//swap the buffer
    }
    glfwTerminate();

	return 0;
}
 

