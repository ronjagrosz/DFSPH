/*************************************************************************
Name:	    Isabell Jansson, Jonathan Bosson, Ronja Grosz
File name:  Viewport.cpp

Viewport is used as a OpenGL controller.  Viewport is responsible for managing all openGL related activities.
*************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <string>
#include <FreeImage.h>


#include "../particle/SPH.h"
#include "../json/picojson.h"
#include "../render/Viewport.h"
#include "Shader.h"

#define OUTPUT_FILE_PATH "results/frames/frame"

using namespace std;

std::string ZeroPadNumber(int num) {
  std::ostringstream ss;
  ss << std::setw(6) << std::setfill('0') << num;
  string result = ss.str();
  if (result.length() > 4) {
    result.erase(0, result.length() - 4);
  }
  return result;
}

Viewport::Viewport() {
	phi = 0.0f;
	theta = M_PI / 8.0f;
	rad = 4.0f;
	zoomFactor = M_PI;
	recordTime = deltaTime = currTime = 0.0f;	
	fps = 0.0;
	record = false;
	frameCount = 0;
	timeSinceAction = glfwGetTime();

	phiW = 0.0;
	thetaW = 0.0;
}

Viewport::~Viewport() {}

/* showFPS() - Calculate and report frames per second
(updated once per second) in the window title bar */
void Viewport::displayFPS(GLFWwindow *window) {
	static double t0 = 0.0;
	static int frames = 0;
	double frametime = 0.0;
	static char titlestring[200];

	double t;

	// Get current time
	t = glfwGetTime();  // Gets number of seconds since glfwInit()
	// If one second has passed, or if this is the very first frame
	if ((t - t0) > 1.0 || frames == 0) {
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

//enable texture, lighting, shading.
void Viewport::init(void) {
    //GL calls
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); // with?

	glGenVertexArrays(1, &vao); //maybe move to init()
	glBindVertexArray(vao);
}

void Viewport::initWorld() {

	Viewport::hydro = new SPH();	//this is the object that will manage all of the particles
}

// Just in case some one wants to resize the window
void Viewport::setupPerspective(GLFWwindow *window, GLfloat *P)	{
	glfwGetWindowSize(window, &width, &height);

	P[0] = P[5] * height / width;

	glViewport(0, 0, width, height);

}

// Control the camera with the arrow keys. Hold left control button and UP/DOWN for zoom
void Viewport::interaction(GLFWwindow *window) {
	recordTime = glfwGetTime() - timeSinceAction;
	deltaTime = (glfwGetTime() - currTime) / 10;
	currTime = glfwGetTime();

	// Zoom
	if (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) || glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) {
		if (glfwGetKey(window, GLFW_KEY_UP) || glfwGetKey(window, GLFW_KEY_W)) {
			if (rad > 0.0f)
				rad -= deltaTime*zoomFactor;
		}
		else if (glfwGetKey(window, GLFW_KEY_DOWN) || glfwGetKey(window, GLFW_KEY_S)) {
			rad += deltaTime*zoomFactor;
		}
	}
	// Up/Down
	else {
		if (glfwGetKey(window, GLFW_KEY_UP) || glfwGetKey(window, GLFW_KEY_W)) {
			theta += deltaTime*M_PI / 2.0; // Rotate 90 degrees per second
			if (theta >= M_PI / 2.0) theta = M_PI / 2.0; // Clamp at 90
		}
		else if (glfwGetKey(window, GLFW_KEY_DOWN) || glfwGetKey(window, GLFW_KEY_S)) {
			theta -= deltaTime*M_PI / 2.0; // Rotate 90 degrees per second
			if (theta < -M_PI / 2.0) theta = -M_PI / 2.0f;      // Clamp at -90
		}
	}

	// Left/Right
	if (glfwGetKey(window, GLFW_KEY_RIGHT) || glfwGetKey(window, GLFW_KEY_D)) {
		phi -= deltaTime*M_PI / 2.0; // Rotate 90 degrees per second (pi/2)
		phi = fmod(phi, M_PI*2.0); // Wrap around at 360 degrees (2*pi)
		if (phi < 0.0) phi += M_PI*2.0; // If phi<0, then fmod(phi,2*pi)<0
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT) || glfwGetKey(window, GLFW_KEY_A)) {
		phi += deltaTime*M_PI / 2.0; // Rotate 90 degrees per second (pi/2)
		phi = fmod(phi, M_PI*2.0);
	}

	// Record
	if (glfwGetKey(window, GLFW_KEY_R) && recordTime > 0.5) {
        record = !record;
        if (record)
            std::cout << "Starting to record..                                 " << std::endl;
        else
            std::cout << "Recorded " << recordTime << " seconds (" 
        << frameCount/recordTime << "fps) Approximately " << ((double)width*height*3/10000000)*frameCount << " MB                  \n";
        timeSinceAction = glfwGetTime();
    }

    // Rotate cube - up/down
    if (glfwGetKey(window, GLFW_KEY_I)) {
			thetaW += deltaTime*M_PI / 2.0; // Rotate 90 degrees per second
			if (thetaW >= M_PI / 2.0) thetaW = M_PI / 2.0; // Clamp at 90
	}
	else if (glfwGetKey(window, GLFW_KEY_K)) {
		thetaW -= deltaTime*M_PI / 2.0; // Rotate 90 degrees per second
		if (thetaW < -M_PI / 2.0) thetaW = -M_PI / 2.0f;      // Clamp at -90
	}
	// Rotate cube - left/right
	if (glfwGetKey(window, GLFW_KEY_L)) {
		phiW -= deltaTime*M_PI / 2.0; // Rotate 90 degrees per second (pi/2)
		phiW = fmod(phiW, M_PI*2.0); // Wrap around at 360 degrees (2*pi)
		if (phiW < 0.0) phiW += M_PI*2.0; // If phi<0, then fmod(phi,2*pi)<0
	}
	else if (glfwGetKey(window, GLFW_KEY_J)) {
		phiW += deltaTime*M_PI / 2.0; // Rotate 90 degrees per second (pi/2)
		phiW = fmod(phiW, M_PI*2.0);
	}
}
// Initialize glut and set all of tAe call backs
int Viewport::start(int argc, char** argv) {   
    GLfloat I[16] = { 1.0f, 0.0f, 0.0f, 0.0f,
					  0.0f, 1.0f, 0.0f, 0.0f,
					  0.0f, 0.0f, 1.0f, 0.0f,
					  0.0f, 0.0f, 0.0f, 1.0f };
	GLfloat P[16] = { 1.815f, 0.0f, 0.0f, 0.0f,
					  0.0f, 2.42f, 0.0f, 0.0f,
					  0.0f, 0.0f, -1.0f, -1.0f,
					  0.0f, 0.0f, -0.2f, 0.0f };
	GLint locationP;
	GLint locationMV;
	GLint locationVel;
	GLint locationCamera;

	glm::mat4 viewMatrix;
	glm::vec4 light;
	glm::vec4 cam;

    // start GLEW extension handler
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return 1;
    }
    
    glfwDefaultWindowHints();

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
	locationVel = glGetUniformLocation(phongShader.programID, "maxVelocity");
	locationCamera = glGetUniformLocation(phongShader.programID, "camPos");

	picojson::value param;
	ifstream paramStream ("json/scene_parameters.json");
	paramStream >> param;
	float boundaryDimension = (float)(param.get<picojson::object>()["boundaryDimension"].get<double>());
	float radius = (float)(param.get<picojson::object>()["geometry"].get<picojson::object>()["r"].get<double>());
	string name = param.get<picojson::object>()["sceneName"].get<std::string>();

	/*
		1 0 0 0 
		0 1 0 0
		0 0 0 0
		0 0 0 r	
	*/

    // Let's get started!
    while (!glfwWindowShouldClose(window)) {
    	glfwPollEvents();
    	//GL calls
        init();
        displayFPS(window);
        glUseProgram(phongShader.programID);
		glUniformMatrix4fv(locationP, 1, GL_FALSE, P);

		setupPerspective(window, P);
		interaction(window);
		cameraPosition = glm::vec3(0.0f, -0.5f, rad);


		// I is the normal Identity matrix
		viewMatrix = glm::make_mat4(I);
        // Translate a bit down and backwards
		viewMatrix = viewMatrix * glm::translate(-cameraPosition)
            * glm::rotate(theta, glm::vec3(1.0f, 0.0f, 0.0f))
            * glm::rotate(phi, glm::vec3(0.0f, 1.0f, 0.0f));

        //convert viewMatrix to float
        glUniformMatrix4fv(locationMV, 1, GL_FALSE, glm::value_ptr(viewMatrix));

        //glUniform3fv(locationCamera, 1, glm::value_ptr(cameraPosition));
        float maxVel = (float) hydro->maxVelocity;
        float* maxVelP = &maxVel;
        glUniform1fv(locationVel, 1, maxVelP);

        hydro->display(phiW, thetaW, vao);

        
        boundingBox.draw(vao, boundaryDimension);
        if (name == "cylinder")
        	cylinder.draw(vao, radius, boundaryDimension);





		// Save the frame
		if (record) {
			frameCount++;
			string fileName = OUTPUT_FILE_PATH + ZeroPadNumber(frameCount) + ".png";
	  		
			// Make the BYTE array, factor of 3 because it's RBG.
			BYTE* pixels = new BYTE[ 3 * width * height];
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, pixels);
			// Convert to FreeImage format & save to file
			FIBITMAP* image = FreeImage_ConvertFromRawBits(pixels, width, height, 3 * width, 24, 0x0000FF, 0x00FF00, 0xFF0000, false);
			FreeImage_Save(FIF_PNG, image, fileName.c_str(), 0);

			// Free resources
			FreeImage_Unload(image);
			delete [] pixels;
		}	
		
		glfwSwapBuffers(window);			//swap the buffer
    }
    glfwTerminate();

	return 0;
}
