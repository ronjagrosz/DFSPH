//
// Name:       Isabell Jansson, Jonathan Bosson, Ronja Grosz
// File name:  vertexshader.glsl
//

#version 330
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;


//out vec3 Position;
out vec3 outColor;

uniform mat4 MV;
uniform mat4 P;

out vec3 position;

//uniform mat3 NormalMatrix;
//uniform mat4 OMV;

void main () {	
	//Position =  vec3(MV * */vec4(inPosition, 0.0, 1.0));
	//Normal = normalize(mat3(MV) * VertexNormal);
	//UV = vertexUV;
	outColor = inColor; // past to the fragment shader
	float pointRadius = 8.0;
	float pointScale = 2.0;
	position = vec3(MV * vec4(inPosition, 1.0));
    float dist = length(position);
    gl_PointSize = pointRadius * (pointScale / dist);

    position = inPosition;
    
    gl_Position = P * MV * vec4(inPosition, 1.0);
}
