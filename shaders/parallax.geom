#version 330

#define MAX_LIGHTS 16

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3 vTangent[];
in vec3 vBinormal[];
in vec3 vNormal[];
in vec3 vEyeVec[];
in vec2 vTexCoord0[];

uniform vec3 lightDir[MAX_LIGHTS];
uniform int numLights;

out vec2 TexCoord0;
out vec3 eyeVec;
out vec3 lightDirs[MAX_LIGHTS];
out vec3 tangent;
out vec3 binormal;
out vec3 normal;

void main()
{
	int i;
	gl_Position = gl_in[0].gl_Position;
	tangent = vTangent[0];
	binormal = vBinormal[0];
	normal = vNormal[0];
	eyeVec = vEyeVec[0];
	TexCoord0 = vTexCoord0[0];
	for(i=0; i < MAX_LIGHTS; ++i)
		lightDirs[i] = vec3(lightDir[i] + vEyeVec[0]);
	EmitVertex();

	gl_Position = gl_in[1].gl_Position;
	tangent = vTangent[1];
	binormal = vBinormal[1];
	normal = vNormal[1];
	eyeVec = vEyeVec[1];
	TexCoord0 = vTexCoord0[1];
	for(i=0; i < MAX_LIGHTS; ++i)
		lightDirs[i] = vec3(lightDir[i] + vEyeVec[1]);
	EmitVertex();

	gl_Position = gl_in[2].gl_Position;
	tangent = vTangent[2];
	binormal = vBinormal[2];
	normal = vNormal[2];
	eyeVec = vEyeVec[2];
	TexCoord0 = vTexCoord0[2];
	for(i=0; i < MAX_LIGHTS; ++i)
		lightDirs[i] = vec3(lightDir[i] + vEyeVec[2]);
	EmitVertex();

	EndPrimitive();
}
