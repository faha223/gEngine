#version 330

#define MAX_LIGHTS 16
#define MAX_SPECULAR 1.0
#define AMBIENT 0.03
#define shininess 8.0
#define scale 1.0
#define bias 0.5

in vec3 normal, eyeVec, tangent, binormal;
in vec3 lightDirs[MAX_LIGHTS];
in vec2 TexCoord0;

uniform int numLights;
uniform vec3 lightColors[MAX_LIGHTS];
uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D specularMap;
uniform sampler2D heightMap;
uniform sampler2D lightMap;
uniform sampler2D occlusionMap;
uniform vec4 color;

out vec4 FragColor;

void main (void)
{
	mat3 TBN = mat3(normalize(tangent), normalize(binormal), normalize(normal));

	vec3 lambertTerm = vec3(AMBIENT) + texture2D(lightMap, TexCoord0).xyz;

	vec3 specular = vec3(0.0);

	float specScale = texture2D(specularMap, TexCoord0).r;
	vec4 final_color = texture2D(diffuseMap, TexCoord0);
        vec3 N = normalize(TBN * (normalize( texture2D(normalMap, TexCoord0).xyz * 2.0 - vec3(1.0)))), L;
	for(int i=0; i<MAX_LIGHTS; ++i)
	{
		if(i < numLights)
		{
			lambertTerm += lightColors[i]*max(dot(N,normalize(lightDirs[i])) * 400.0 / pow(distance(lightDirs[i], vec3(0.0, 0.0, 0.0)), 2), 0.0);
			specular += lightColors[i]*pow(max(dot(N, normalize(normalize(lightDirs[i]) + normalize(eyeVec))), 0.0), shininess)*specScale*MAX_SPECULAR;
		}
	}
	lambertTerm = min(lambertTerm, 1.0);
        FragColor = vec4(final_color.r * color.r * lambertTerm.r + specular.r,
                         final_color.g * color.g * lambertTerm.g + specular.g,
                         final_color.b * color.b * lambertTerm.b + specular.b,
                         final_color.a * color.a);
}
