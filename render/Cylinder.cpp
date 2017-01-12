/*************************************************************************
Name:       Isabell Jansson, Jonathan Bosson, Ronja Grosz
File name:  BoundingBox.cpp

This class contains the data structure for creating and rendering
a cylinder. 
*************************************************************************/

#include "Cylinder.h"

Cylinder::Cylinder(){}

Cylinder::~Cylinder() {}

void Cylinder::draw(GLuint vao, float radius) {
	GLfloat vertices[72] = {-radius, radius, radius, 	// 1
							-radius, radius, -radius, // 2

							-radius, radius, -radius, // 2
							radius, radius, -radius, 	// 3

							radius, radius, -radius, 	// 3
							radius, radius, radius, 		// 4

							radius, radius, radius, 		// 4
							-radius, radius, radius, 	// 1

							-radius, radius, radius, 	// 1
							-radius, -radius, radius, 	// 5

							-radius, -radius, radius, 	// 5
							-radius, -radius, -radius, 	// 6

							-radius, -radius, -radius, 	// 6
							radius, -radius, -radius, 	// 7

							radius, -radius, -radius, 	// 7
							radius, -radius, radius, 	// 8

							radius, -radius, radius, 	// 8
							-radius, -radius, radius, 	// 5

							-radius, radius, -radius, // 2
							-radius, -radius, -radius, 	// 6

							radius, radius, -radius, 	// 3
							radius, -radius, -radius, 	// 7

							radius, radius, radius, 		// 4
							radius, -radius, radius}; 	// 8

	GLfloat color[72] = {1.0, 1.0, 1.0,
						1.0, 1.0, 1.0,
						1.0, 1.0, 1.0,
						1.0, 1.0, 1.0,
						1.0, 1.0, 1.0,
						1.0, 1.0, 1.0,
						1.0, 1.0, 1.0,
						1.0, 1.0, 1.0,
						1.0, 1.0, 1.0,
						1.0, 1.0, 1.0,
						1.0, 1.0, 1.0,
						1.0, 1.0, 1.0,
						1.0, 1.0, 1.0,
						1.0, 1.0, 1.0,
						1.0, 1.0, 1.0,
						1.0, 1.0, 1.0,
						1.0, 1.0, 1.0,
						1.0, 1.0, 1.0,
						1.0, 1.0, 1.0,
						1.0, 1.0, 1.0,
						1.0, 1.0, 1.0,
						1.0, 1.0, 1.0,
						1.0, 1.0, 1.0,
						1.0, 1.0, 1.0};

	//vertices = (float)radius * vertices;

	// Generate (one) new Vertex Buffer Object and get the associated id
	glGenBuffers(2, vbo_c);

	// Bind the first VBO as being the active buffer and storing vertex attributes (coordinates)
    glBindBuffer(GL_ARRAY_BUFFER, vbo_c[0]);

	// Copy the vertex data from diamond to our buffer 
    // 8 * sizeof(GLfloat) is the size of the diamond array, since it contains 8 GLfloat values 
    glBufferData(GL_ARRAY_BUFFER, 72 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    // Specify that our coordinate data is going into attribute index 0, and contains three floats per vertex 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Enable attribute index 0 as being used 
    glEnableVertexAttribArray(0);

    // Bind the second VBO as being the active buffer and storing vertex attributes (colors)
    glBindBuffer(GL_ARRAY_BUFFER, vbo_c[1]);

    glBufferData(GL_ARRAY_BUFFER, 72 * sizeof(GLfloat), color, GL_STATIC_DRAW);

    // Specify that our color data is going into attribute index 1, and contains three floats per vertex 
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Enable attribute index 1 as being used
    glEnableVertexAttribArray(1);

	glBindVertexArray(vao);

	glDrawArrays(GL_LINES, 0, 72);

}