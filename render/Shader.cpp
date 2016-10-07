#include "Shader.h"

Shader::Shader () {
	this->programID = 0;
}

//! Loads the files, comiles the shaders and assembles the shader program.
Shader::Shader (const char *vertexFilePath, const char *fragmentFilePath) {
	this->createShader(vertexFilePath, fragmentFilePath);
}

//! Deletes the program if it was compiled
Shader::~Shader () {
	if (programID != 0)
		glDeleteProgram(programID);
}

//! Creates, loads, compiles and links the GLSL shader objects.
void Shader::createShader(const char *vertexFilePath, const char *fragmentFilePath) {
	
	char str[4096]; // for wrinting error msg
	
	GLint isCompiled = 0;
	GLint isLinked = 0;

	//Read the source code in shader files into the buffers
	string vertexSource = readFile(vertexFilePath);
	string fragmentSource = readFile(fragmentFilePath);

	// Create empty vertex shader handle
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

	// Send the source code in teh shader to GL
	const GLchar *sourceVertex = (const GLchar *)vertexSource.c_str();
	glShaderSource(vertexShader, 1, &sourceVertex, 0);

	glCompileShader(vertexShader);

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
	
	if (isCompiled == GL_FALSE) {

		glGetShaderInfoLog(vertexShader, sizeof(str), NULL, str);
		fprintf(stderr, "%s: %s\n", "Vertex shader compile error", str);

		glDeleteShader(vertexShader);

		return;
	}

	// Create empty fragment shader handle
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Send the source code in the shader to GL
	const GLchar *sourceFragment = (const GLchar *)fragmentSource.c_str();
	glShaderSource(fragmentShader, 1, &sourceFragment, 0);

	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);

	if (isCompiled == GL_FALSE) {

		glGetShaderInfoLog(fragmentShader, sizeof(str), NULL, str);
		fprintf(stderr, "%s: %s\n", "Fragment shader compile error", str);

		glDeleteShader(fragmentShader);
		glDeleteShader(vertexShader);

		return;
	}

	// create program object
	GLuint program = glCreateProgram();

	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked);

	if (isLinked == GL_FALSE) {

		glGetProgramInfoLog(program, sizeof(str), NULL, str);
		fprintf(stderr, "%s: %s\n", "Program object linking error", str);

		glDeleteProgram(program);
		glDeleteShader(fragmentShader);
		glDeleteShader(vertexShader);

		return;
	}

	glDetachShader(program, vertexShader);
	glDetachShader(program, fragmentShader);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	programID = program;
}

//! Reads the source code in the shader file into a string.
string Shader::readFile(const char *filePath) {

	string sourceCode;
	string line = "";
	ifstream fileStream (filePath);

	if (fileStream.is_open()) {
		
		while (!fileStream.eof()) {

			std::getline(fileStream, line);
			sourceCode.append(line + "\n");
		}
	} else {
		cerr << "Could not open file: " << filePath << endl;
		return "";
	}
	
	return sourceCode;
}