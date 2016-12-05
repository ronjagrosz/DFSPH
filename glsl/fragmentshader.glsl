//
// Name:       Isabell Jansson, Jonathan Bosson, Ronja Grosz
// File name:  fragmentshader.glsl
//

#version 330
//layout( location = 0 ) out vec4 FragColor;


precision highp float;


uniform mat4 MV;

out vec4 FragColor;
in vec3 outColor;
in vec3 position;
//out vec4 gl_FragColor;

/*in vec3 Position;
in vec3 Normal;
in vec2 UV;

const int nLightsources = 4;

uniform vec3 lightPos[nLightsources];
uniform sampler2D tex;

vec3 calculateLight(vec3 lightP, float lightIntensity) {
	vec3 Kd = vec3(0.7f, 0.7f, 0.7f);// * lightIntensity;    	// Diffuse reflectivity
	vec3 Ka = vec3(0.1f, 0.1f, 0.1f);// * lightIntensity;    	// Ambient reflectivity
	vec3 Ks = vec3( 0.5f, 0.5f, 0.5f);// * lightIntensity;		// Specular reflectivity

	// DXT from directX, so we have to invert the UV coordinates 
	//(coord.u, 1.0-coord.v) to fetch the correct texel.
	vec2 invUV = vec2(UV.x, 1.0-UV.y);
	vec3 texcolor = texture( tex, invUV ).rgb;
	
	
	//Diffuse part
	vec3 normal  = normalize( Normal );							
	vec3 lightDir = normalize(lightP - Position );				// lightDir
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * texcolor;

	//specular part
	vec3 viewDir  = normalize(vec3(-Position));			    	// viewDir
	vec3 reflectDir  = reflect( -lightDir, normal  );			// reflectDir
	float Shininess = 4.0f;										// Specular shininess factor

	float spec = 0.0;
	// Ambient
	vec3 ambient = 0.3 * texcolor;
	vec3 halfwayDir = normalize(lightDir + viewDir);  
	spec = pow(max(dot(normal, halfwayDir), 0.0), 16.0);
	vec3 specular = vec3(0.3) * spec;							// assuming bright white light color
	
	return ambient + diffuse + specular;
}
*/

void main () {
	/*
	float LightIntensity = 0.6f;
	vec3 resultLight = calculateLight(lightPos[0], LightIntensity);
	LightIntensity = 0.2f;
	//for (int i = 1; i < nLightsources; i++)
		//resultLight += calculateLight(lightPos[i], LightIntensity);
	FragColor = vec4(resultLight, 1.0f);*/

	//FragColor = vec4(outColor,1.0);

	// calculate normal from texture coordinates
    vec3 n;
    vec2 texCoord = normalize(vec2(0.33, 0.33));
    n.xy = texCoord*vec2(2.0, -2.0) + vec2(-1.0, 1.0);
    //This is a more compatible version which works on ATI and Nvidia hardware
    //However, This does not work on Apple computers. :/
    //n.xy = gl_PointCoord.st*vec2(2.0, -2.0) + vec2(-1.0, 1.0);

    float mag = dot(n.xy, n.xy);
    if (mag > 1.0) discard;   // kill pixels outside circle
    n.z = sqrt(1.0-mag);

	float pointRadius = 3.0;
	float near = 0.0;
	float far = 3.0;

    // point on surface of sphere in eye space
    vec4 spherePosEye =vec4(position + n * pointRadius,1.0);

    vec4 clipSpacePos = MV * spherePosEye;
    float normDepth = clipSpacePos.z/clipSpacePos.w;

    // Transform into window coordinates coordinates
    gl_FragDepth = (((far-near)/2.)*normDepth)+((far+near)/2.);
    
	FragColor = vec4(outColor,1.0);
	
}
