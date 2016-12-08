//
// Name:       Isabell Jansson, Jonathan Bosson, Ronja Grosz
// File name:  vertexshader.glsl
//

#version 330
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inVelocity;

uniform mat4 MV;
uniform mat4 P;

out vec3 outVelocity;
out vec3 mvPosition;
out vec3 normal;
out float pointRadius;

void main () {	
	outVelocity = inVelocity; // past to the fragment shader
	pointRadius = 0.05;
	vec4 mvPos = MV * vec4(inPosition, 1.0);
	normal = vec3(MV * vec4(inVelocity, 1.0));

	vec4 proj = P * vec4(pointRadius, 0.0, mvPos.z, mvPos.w);
    gl_PointSize = 640 * proj.x / proj.w; // not on if we want smooth 
    mvPosition = vec3(mvPos);

    gl_Position = P * mvPos;
}
