#version 330

#define MAX_LIGHTS 16

in vec3 Lambert, Specular;
in vec2 TexCoord0;

uniform vec3 lightDir[MAX_LIGHTS];
uniform int numLights;
uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D heightMap;
uniform sampler2D lightMap;
uniform sampler2D occlusionMap;
uniform vec4 color;

out vec4 FragColor;

void main (void)
{
	vec3 l = Lambert + texture2D(lightMap, TexCoord0).xyz;

	vec4 final_color = texture2D(diffuseMap, TexCoord0);

	if(distance(Lambert, vec3(0.0)) <= 0.25)
		l = l * 0.125;
	else if((distance(Lambert, vec3(0.0)) > 0.25)&&(distance(Lambert, vec3(0.0)) <= 0.5))
		l = l * 0.375;
	else if((distance(Lambert, vec3(0.0)) > 0.5)&&(distance(Lambert, vec3(0.0)) <= 0.75))
		l = l * 0.625;
	else if((distance(Lambert, vec3(0.0)) > 0.75)&&(distance(Lambert, vec3(0.0)) <= 1.0))
		l = l * 0.875;

	FragColor = vec4(final_color.r * color.r * l.r + Specular.r,
			 final_color.g * color.g * l.g + Specular.g,
			 final_color.b * color.b * l.b + Specular.b,
				final_color.a * color.a);
}
