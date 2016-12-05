/*************************************************************************
Name:	    Isabell Jansson, Jonathan Bosson, Ronja Grosz
File name: 	BoundingBox.h

*************************************************************************/

#include "GL/glew.h"
#include "GLFW/glfw3.h"

class BoundingBox {
	private:
		int scale;
		GLuint vbo_bb[2];

	public:
		BoundingBox();
		~BoundingBox();

		void draw(GLuint vao, double scale);
		
};