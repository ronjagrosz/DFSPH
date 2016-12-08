//
// Name:       Isabell Jansson, Jonathan Bosson, Ronja Grosz
// File name:  fragmentshader.glsl
//
#version 330

precision highp float;

in vec3 outVelocity;
in vec3 mvPosition;
in float pointRadius;

uniform mat4 MV;
uniform mat4 P;
uniform float maxVelocity;

out vec4 FragColor;

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

void main () {
	
	// calculate normal from texture coordinates
    vec3 n;
    n.xy = gl_PointCoord.st*vec2(2.0, -2.0) + vec2(-1.0, 1.0);
    float mag = dot(n.xy, n.xy);
    if (mag > 1.0) discard;   // kill pixels outside circle
    n.z = sqrt(1.0-mag);

    vec3 eye = mvPosition + vec3(0.0, 0.0, pointRadius * n.z);
    float depth = (P[2][2] * eye.z + P[3][2]) / (P[2][3] * eye.z + P[3][3]);

    gl_FragDepth = (depth + 1.0) / 2.0;

    
    // calculate lighting
	const vec3 light_dir = vec3(0.0, 0.0, 1.0);
    float diffuse = max(0.0, dot(light_dir, n));
 
	vec3 halfVector = normalize( eye + light_dir);	
    float spec = pow(max(0.0, dot(n,halfVector)), 100.0);

	// modify color according to the velocity
	vec3 color = vec3(0.3, 0.3, 0.9);
	vec3 hsv = rgb2hsv(color);
	float v = length(outVelocity);
	v = min((1.0/maxVelocity)*v*v, 1.0);
	vec3 fluidColor = hsv2rgb(vec3(hsv.x, max(1.0 - v, 0.0), 1.0));

	// compute final color
	vec3 outColor = 0.25 * fluidColor;
    outColor += 0.7 * diffuse * fluidColor;
    outColor += 0.05 * spec * vec3(1.0);
	outColor = clamp(outColor, 0.0, 1.0);

	FragColor = vec4(outColor,1.0);
}
