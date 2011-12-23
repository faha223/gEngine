#version 330

in vec2 TexCoord0;

uniform sampler2D diffuseMap;

out vec4 FragColor;

void main()
{
	FragColor = texture2D(diffuseMap, TexCoord0);
}
