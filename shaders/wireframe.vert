#version 330

layout (location = 3) in vec4 Vertex;
layout (location = 2) in vec4 Normal;
layout (location = 8) in vec4 MultiTexCoord0;

uniform mat4 Modelview;
uniform mat4 Projection;

out vec2 vTexCoord0;

void main()
{
	gl_Position = Projection * Modelview * Vertex;
	vTexCoord0 = MultiTexCoord0.st;
}
