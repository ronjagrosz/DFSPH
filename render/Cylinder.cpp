/*************************************************************************
Name:       Isabell Jansson, Jonathan Bosson, Ronja Grosz
File name:  BoundingBox.cpp

This class contains the data structure for creating and rendering
a cylinder. 
*************************************************************************/

#include "Cylinder.h"
#include <math.h>
#include <iostream>
using namespace std;

Cylinder::Cylinder(){}


Cylinder::~Cylinder(){}

void Cylinder::draw(GLuint vao, float radius, float boundary) {


	// o=========o

	// how many segments? - 36
	float segments = 200;
	int l = 6;
	const int num = l * segments;
	float angle;
	float x, y;
	
	GLfloat vertices[num];
	float sqrtRadius = sqrt(radius);
	for (int i = 0; i < segments; ++i) {
		angle = i * 2/segments * M_PI;	
		x = sqrtRadius * cos(angle);
		y = sqrtRadius * sin(angle);

		// Lines on sides
		vertices[l*i+0] = x;
		vertices[l*i+1] = y;
		vertices[l*i+2] = -boundary - 0.5;

		vertices[l*i+3] = x;
		vertices[l*i+4] = y;
		vertices[l*i+5] = boundary + 0.5;

		/*
		// Back
		vertices[l*i+6] = x;
		vertices[l*i+7] = y;
		vertices[l*i+8] = -boundary;

		vertices[l*i+9] = 0.0;
		vertices[l*i+10] = 0.0;
		vertices[l*i+11] = -boundary;

		// Front
		vertices[l*i+12] = x;
		vertices[l*i+13] = y;
		vertices[l*i+14] = boundary;

		vertices[l*i+15] = 0.0;
		vertices[l*i+16] = 0.0;
		vertices[l*i+17] = boundary;
		*/
	}

	GLfloat color[num]; // num
	for (int i = 0; i < num; ++i)
		color[i] = 1000;

	//vertices = (float)radius * vertices;

	// Generate (one) new Vertex Buffer Object and get the associated id
	glGenBuffers(2, vbo_c);

	// Bind the first VBO as being the active buffer and storing vertex attributes (coordinates)
    glBindBuffer(GL_ARRAY_BUFFER, vbo_c[0]);

	// Copy the vertex data from diamond to our buffer 
    // 8 * sizeof(GLfloat) is the size of the diamond array, since it contains 8 GLfloat values 
    glBufferData(GL_ARRAY_BUFFER, num * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    // Specify that our coordinate data is going into attribute index 0, and contains three floats per vertex 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Enable attribute index 0 as being used 
    glEnableVertexAttribArray(0);

    // Bind the second VBO as being the active buffer and storing vertex attributes (colors)
    glBindBuffer(GL_ARRAY_BUFFER, vbo_c[1]);

    glBufferData(GL_ARRAY_BUFFER, num * sizeof(GLfloat), color, GL_STATIC_DRAW);

    // Specify that our color data is going into attribute index 1, and contains three floats per vertex 
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Enable attribute index 1 as being used
    glEnableVertexAttribArray(1);

	glBindVertexArray(vao);

	glEnable(GL_LINE_SMOOTH);
	glLineWidth(20.0f);
	glDrawArrays(GL_LINES, 0, num);
	glDisable(GL_LINE_SMOOTH);
}





