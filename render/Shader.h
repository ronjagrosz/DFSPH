#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

//! Class for loading, compiling and assembeling shader program.
class Shader {

public:
	GLuint programID;

	Shader();
	Shader(const char *vertexFilePath, const char *fragmentFilePath);
	~Shader();

	void createShader();
	void createShader(const char *vertexFilePath, const char *fragmentFilePath);


private:
	string readFile(const char *filePath);

};