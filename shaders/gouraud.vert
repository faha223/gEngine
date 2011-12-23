#version 330

#define MAX_LIGHTS 16

layout (location = 3) in vec4 Vertex;
layout (location = 2) in vec4 Normal;
layout (location = 8) in vec4 MultiTexCoord0;

uniform vec3 lightDir[MAX_LIGHTS];
uniform vec3 lightColors[MAX_LIGHTS];
uniform int numLights;
uniform sampler2D diffuseMap;
uniform mat4 Modelview;
uniform mat4 Projection;

out vec3 vLambert;
out vec3 vSpecular;
out vec2 vTexCoord0;

void main()
{
	vLambert = vec3(0.0);
	vSpecular = vec3(0.0);
	gl_Position = Projection * Modelview * Vertex;
	vec3 normal = mat3(Modelview) * Normal.xyz;
	vTexCoord0 = MultiTexCoord0.st;
	vec4 vVertex = Modelview * Vertex;
	int i;
	vec3 L;
	for(i=0; (i<numLights)&&(i<MAX_LIGHTS); ++i)
	{
		L = lightDir[i] - vVertex.xyz;
		vLambert += vec3(	lightColors[i].r * clamp (dot(normal, normalize(L)) * 20.0/distance(lightDir[i] - vVertex.xyz, vec3(0.0, 0.0, 0.0)), 0.0, 1.0),
					lightColors[i].g * clamp (dot(normal, normalize(L)) * 20.0/distance(lightDir[i] - vVertex.xyz, vec3(0.0, 0.0, 0.0)), 0.0, 1.0),
					lightColors[i].b * clamp (dot(normal, normalize(L)) * 20.0/distance(lightDir[i] - vVertex.xyz, vec3(0.0, 0.0, 0.0)), 0.0, 1.0));
		vSpecular += vec3(	lightColors[i].r * clamp(pow(max(dot(reflect(-normalize(L), normal), normalize(-vVertex.xyz)), 0.0), 8.0), 0.0, 1.0)*0.5,
					lightColors[i].g * clamp(pow(max(dot(reflect(-normalize(L), normal), normalize(-vVertex.xyz)), 0.0), 8.0), 0.0, 1.0)*0.5,
					lightColors[i].b * clamp(pow(max(dot(reflect(-normalize(L), normal), normalize(-vVertex.xyz)), 0.0), 8.0), 0.0, 1.0)*0.5);
	}
}
