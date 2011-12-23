#version 330

#define MAX_LIGHTS 16

layout(location = 0) in vec3 Tangent;
layout(location = 1) in vec3 Binormal;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec3 Vertex;
layout(location = 8) in vec2 MultiTexCoord0;

uniform mat4 Modelview, Projection;

out vec3 vTangent, vBinormal, vNormal, vEyeVec;
out vec2 vTexCoord0;

void main()
{
	gl_Position = Projection * Modelview * vec4(Vertex, 1.0);
	vTexCoord0 = MultiTexCoord0.st;
	vTangent = mat3(Modelview) * Tangent;
	vBinormal = mat3(Modelview) * Binormal;
	vNormal = mat3(Modelview) * Normal;
	vEyeVec = -vec3(Modelview * vec4(Vertex, 1.0));
}
