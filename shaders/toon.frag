#version 330

#define MAX_LIGHTS 16

in vec3 tangent, binormal, normal, eyeVec;
in vec3 lightDirs[MAX_LIGHTS];
in vec2 TexCoord0;

uniform vec3 lightDir[MAX_LIGHTS];
uniform int numLights;
uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform vec4 color;

out vec4 FragColor;

void main (void)
{
	mat3 TBN = mat3(normalize(tangent), normalize(binormal), normalize(normal));
	vec4 final_color = texture2D(diffuseMap, TexCoord0);
	vec3 N = normalize(TBN * ((2.0*texture2D(normalMap, TexCoord0).xyz) - vec3(0.5))), L;
	int i;
	float lambertTerm = 0.0, specular = 0.0, s, l;
	for (i=0; (i<numLights)&&(i<MAX_LIGHTS); ++i)
	{
		L = normalize(lightDirs[i]);

		l = dot(N,L) * 20.0/distance(lightDirs[i], vec3(0.0, 0.0, 0.0));

		if (l > 0.0)
			lambertTerm += l;

		s = pow(max(dot(reflect(-L, N), normalize(eyeVec)), 0.0), 8.0);
		specular += s*0.5;
	}
        if(lambertTerm < 0.25)
                lambertTerm = 0.125;
	else if((lambertTerm > 0.25)&&(lambertTerm <= 0.5))
                lambertTerm = 0.375;
	else if((lambertTerm > 0.5)&&(lambertTerm <= 0.75))
		lambertTerm = 0.625;
        else if((lambertTerm > 0.75)&&(lambertTerm <= 1.0))
                lambertTerm = 0.875;
        else
                lambertTerm = 1.0;
	FragColor = vec4(clamp(	final_color.r * color.r * lambertTerm + specular, 0.0, 1.0),
			 clamp( final_color.g * color.g * lambertTerm + specular, 0.0, 1.0),
			 clamp( final_color.b * color.b * lambertTerm + specular, 0.0, 1.0),
				final_color.a * color.a);
}
