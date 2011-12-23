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
	int t, i;
	for(t = 0; t < 3; ++t)
	{
		gl_Position = gl_in[t].gl_Position;
		tangent = vTangent[t];
		binormal = vBinormal[t];
		if((vNormal[0] == vNormal[1])&&(vNormal[1] == vNormal[2]))
			normal = vNormal[t];
		else
			normal = vec3(1.0);
		eyeVec = vEyeVec[t];
		TexCoord0 = vTexCoord0[t];
		for(i=0; i < MAX_LIGHTS; ++i)
			lightDirs[i] = vec3(lightDir[i] + vEyeVec[t]);
		EmitVertex();
	}
	EndPrimitive();
}
