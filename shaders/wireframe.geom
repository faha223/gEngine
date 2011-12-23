#version 330

layout (triangles) in;
layout (line_strip, max_vertices = 4) out;

in vec2 vTexCoord0[];

out vec2 TexCoord0;

void main()
{
	gl_Position = gl_in[0].gl_Position;
	TexCoord0 = vTexCoord0[0];
	EmitVertex();
	gl_Position = gl_in[1].gl_Position;
	TexCoord0 = vTexCoord0[1];
	EmitVertex();
	gl_Position = gl_in[2].gl_Position;
	TexCoord0 = vTexCoord0[2];
	EmitVertex();
	gl_Position = gl_in[0].gl_Position;
	TexCoord0 = vTexCoord0[0];
	EmitVertex();
	EndPrimitive();
}
