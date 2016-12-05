/*************************************************************************
Name:       Isabell Jansson, Jonathan Bosson, Ronja Grosz
File name:  BoundingBox.cpp

This class contains the data structure for creating and rendering
a bounding box. 
*************************************************************************/

#include "BoundingBox.h"

BoundingBox::BoundingBox(){}

BoundingBox::~BoundingBox() {}

void BoundingBox::draw(GLuint vao, float scale) {
	GLfloat vertices[72] = {-scale, scale, scale, 	// 1
							-scale, scale, -scale, // 2

							-scale, scale, -scale, // 2
							scale, scale, -scale, 	// 3

							scale, scale, -scale, 	// 3
							scale, scale, scale, 		// 4

							scale, scale, scale, 		// 4
							-scale, scale, scale, 	// 1

							-scale, scale, scale, 	// 1
							-scale, -scale, scale, 	// 5

							-scale, -scale, scale, 	// 5
							-scale, -scale, -scale, 	// 6

							-scale, -scale, -scale, 	// 6
							scale, -scale, -scale, 	// 7

							scale, -scale, -scale, 	// 7
							scale, -scale, scale, 	// 8

							scale, -scale, scale, 	// 8
							-scale, -scale, scale, 	// 5

							-scale, scale, -scale, // 2
							-scale, -scale, -scale, 	// 6

							scale, scale, -scale, 	// 3
							scale, -scale, -scale, 	// 7

							scale, scale, scale, 		// 4
							scale, -scale, scale}; 	// 8

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

	//vertices = (float)scale * vertices;

	// Generate (one) new Vertex Buffer Object and get the associated id
	glGenBuffers(2, vbo_bb);

	// Bind the first VBO as being the active buffer and storing vertex attributes (coordinates)
    glBindBuffer(GL_ARRAY_BUFFER, vbo_bb[0]);

	// Copy the vertex data from diamond to our buffer 
    // 8 * sizeof(GLfloat) is the size of the diamond array, since it contains 8 GLfloat values 
    glBufferData(GL_ARRAY_BUFFER, 72 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    // Specify that our coordinate data is going into attribute index 0, and contains three floats per vertex 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Enable attribute index 0 as being used 
    glEnableVertexAttribArray(0);

    // Bind the second VBO as being the active buffer and storing vertex attributes (colors)
    glBindBuffer(GL_ARRAY_BUFFER, vbo_bb[1]);

    glBufferData(GL_ARRAY_BUFFER, 72 * sizeof(GLfloat), color, GL_STATIC_DRAW);

    // Specify that our color data is going into attribute index 1, and contains three floats per vertex 
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Enable attribute index 1 as being used
    glEnableVertexAttribArray(1);

	glBindVertexArray(vao);

	glDrawArrays(GL_LINES, 0, 72);

}