#version 330

#define MAX_LIGHTS 16

in vec3 Lambert;
in vec3 Specular;
in vec2 TexCoord0;

uniform vec3 lightDir[MAX_LIGHTS];
uniform int numLights;
uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D lightMap;
uniform vec4 color;

out vec4 FragColor;

void main ()
{
	vec4 final_color = texture2D(diffuseMap, TexCoord0);

	vec3 l = vec3(	max(0.1, Lambert.r),
		 	max(0.1, Lambert.g),
		 	max(0.1, Lambert.b));

	FragColor = vec4(clamp( final_color.r * color.r * l.r + Specular.r, 0.0, 1.0),
			 clamp( final_color.g * color.g * l.g + Specular.g, 0.0, 1.0),
			 clamp( final_color.b * color.b * l.b + Specular.b, 0.0, 1.0),
				final_color.a * color.a);
}
