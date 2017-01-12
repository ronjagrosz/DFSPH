/*************************************************************************
Name:	    Isabell Jansson, Jonathan Bosson, Ronja Grosz
File name: 	Cylinder.h

*************************************************************************/

#include "GL/glew.h"
#include "GLFW/glfw3.h"

class Cylinder {
	private:
		int radius;
		GLuint vbo_c[2];

	public:
		Cylinder();
		~Cylinder();

		void draw(GLuint vao, float radius);
		
};