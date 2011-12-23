#version 330

// This geometry shader does NOTHING but pass data from the vertex shader to the fragment shader

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 vLambert[], vSpecular[];
in vec2 vTexCoord0[];

out vec2 TexCoord0;
out vec3 Lambert, Specular;

void main()
{
	// For each of the three vertices of the triangle, pass along the values for the lambert term, specular term, position, and texture coordinates respectively
	gl_Position = gl_in[0].gl_Position;
	TexCoord0 = vTexCoord0[0];
	Lambert = vLambert[0];
	Specular = vSpecular[0];
	EmitVertex();
	gl_Position = gl_in[1].gl_Position;
	TexCoord0 = vTexCoord0[1];
	Lambert = vLambert[1];
	Specular = vSpecular[1];
	EmitVertex();
	gl_Position = gl_in[2].gl_Position;
	TexCoord0 = vTexCoord0[2];
	Lambert = vLambert[2];
	Specular = vSpecular[2];
	EmitVertex();
	// Then we're done
	EndPrimitive();
}
