#version 330

#define MAX_LIGHTS 16
#define MAX_SPECULAR 1.0
#define AMBIENT 0.03
#define shininess 8.0
#define scale 0.3
#define bias 0.15
#define steps 5

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
	// calculate the tangent, binormal, normal matrix
	mat3 TBN = mat3(normalize(tangent), normalize(binormal), normalize(normal));

	// And the distance from the camera to the given fragment
	float dist = distance(eyeVec, vec3(0.0));

	// The step size for the linear search is (the distance * the z component of the tangent space view vector)/(256)
	float step = (dist*(transpose(TBN)*eyeVec).z)/(256.0);

	// The delta single shift vector is the xy components of the tangent space view vector multiplied by the step size and scale and divided by the z component of the tangent space view vector
	vec2 delta = (transpose(TBN) * eyeVec).xy;
	delta = vec2(-delta.x, delta.y) * (scale * step / ((transpose(TBN) * eyeVec).z));

	// The starting texHeight is the scaled and biased value from the height map and the current texture coordinate
	float texHeight = texture2D(heightMap, TexCoord0).r * scale - bias;
	// the starting height is 1/8
	float height = 0.125;

	vec2 newTexCoord = TexCoord0;
	for(int p = 0; p < 1.0/step; ++p)
	{
		if(texHeight < height)
		{
			height -= step;
			newTexCoord += delta;
			texHeight = texture2D(heightMap, newTexCoord).r * scale - bias;
		}
	}

	vec2 vHi = newTexCoord - delta;
	vec2 vLo = newTexCoord;
	float vHeightHi = height + step;
	float vHeightLo = height;
	float midHeight = (vHeightHi + vHeightLo)*0.5;
	texHeight = texture2D(heightMap, (vHi+vLo)*0.5).r*scale-bias;

	for(int s = 0; (s < 10) && (abs(texHeight - midHeight) > 0.01); ++s)
	{
		if((midHeight - texHeight) > 0.0)
		{
			vHi = (vHi + vLo)*0.5;
			vHeightHi = midHeight;
		}
		else if((midHeight - texHeight) < 0.0)
		{
			vLo = (vHi + vLo)*0.5;
			vHeightLo = midHeight;
		}
		midHeight = (vHeightHi + vHeightLo) * 0.5;
		texHeight = texture2D(heightMap, (vHi+vLo)*0.5).r*scale-bias;
	}
	// The final texture coordinate is the midpoint value that gave a height within 0.01 of the correct height
	newTexCoord = (vHi + vLo)*0.5;

	float silhouette = 1.0;
	if((newTexCoord.x > 1.0)||(newTexCoord.y > 1.0)||(newTexCoord.x < 0.0)||(newTexCoord.y < 0.0))
		silhouette = 0.0;

	vec3 lambertTerm = vec3(AMBIENT) + texture2D(lightMap, newTexCoord).xyz;
	vec3 specular = vec3(0.0);
	float specScale = texture2D(specularMap, newTexCoord).r;
	vec4 final_color = texture2D(diffuseMap, newTexCoord);
	vec3 N = normalize(TBN * (normalize(texture2D(normalMap, newTexCoord).xyz * 2.0 - vec3(1.0)))), L;

	// calculate the accumulated light from all 15 potential lights
	if(0 < numLights)
	{
		lambertTerm += lightColors[0]*max(dot(N,normalize(lightDirs[0])) * 400.0 / pow(distance(lightDirs[0], vec3(0.0, 0.0, 0.0)), 2), 0.0);
		specular += lightColors[0]*pow(max(dot(N, normalize(normalize(lightDirs[0]) + normalize(eyeVec))), 0.0), shininess)*specScale*MAX_SPECULAR;
	}
	if(1 < numLights)
	{
		lambertTerm += lightColors[1]*max(dot(N,normalize(lightDirs[1])) * 400.0 / pow(distance(lightDirs[1], vec3(0.0, 0.0, 0.0)), 2), 0.0);
		specular += lightColors[1]*pow(max(dot(N, normalize(normalize(lightDirs[1]) + normalize(eyeVec))), 0.0), shininess)*specScale*MAX_SPECULAR;
	}
	if(2 < numLights)
	{
		lambertTerm += lightColors[2]*max(dot(N,normalize(lightDirs[2])) * 400.0 / pow(distance(lightDirs[2], vec3(0.0, 0.0, 0.0)), 2), 0.0);
		specular += lightColors[2]*pow(max(dot(N, normalize(normalize(lightDirs[2]) + normalize(eyeVec))), 0.0), shininess)*specScale*MAX_SPECULAR;
	}
	if(3 < numLights)
	{
		lambertTerm += lightColors[3]*max(dot(N,normalize(lightDirs[3])) * 400.0 / pow(distance(lightDirs[3], vec3(0.0, 0.0, 0.0)), 2), 0.0);
		specular += lightColors[3]*pow(max(dot(N, normalize(normalize(lightDirs[3]) + normalize(eyeVec))), 0.0), shininess)*specScale*MAX_SPECULAR;
	}
	if(4 < numLights)
	{
		lambertTerm += lightColors[4]*max(dot(N,normalize(lightDirs[4])) * 400.0 / pow(distance(lightDirs[4], vec3(0.0, 0.0, 0.0)), 2), 0.0);
		specular += lightColors[4]*pow(max(dot(N, normalize(normalize(lightDirs[4]) + normalize(eyeVec))), 0.0), shininess)*specScale*MAX_SPECULAR;
	}
	if(5 < numLights)
	{
		lambertTerm += lightColors[5]*max(dot(N,normalize(lightDirs[5])) * 400.0 / pow(distance(lightDirs[5], vec3(0.0, 0.0, 0.0)), 2), 0.0);
		specular += lightColors[5]*pow(max(dot(N, normalize(normalize(lightDirs[5]) + normalize(eyeVec))), 0.0), shininess)*specScale*MAX_SPECULAR;
	}
	if(6 < numLights)
	{
		lambertTerm += lightColors[6]*max(dot(N,normalize(lightDirs[6])) * 400.0 / pow(distance(lightDirs[6], vec3(0.0, 0.0, 0.0)), 2), 0.0);
		specular += lightColors[6]*pow(max(dot(N, normalize(normalize(lightDirs[6]) + normalize(eyeVec))), 0.0), shininess)*specScale*MAX_SPECULAR;
	}
	if(7 < numLights)
	{
		lambertTerm += lightColors[7]*max(dot(N,normalize(lightDirs[7])) * 400.0 / pow(distance(lightDirs[7], vec3(0.0, 0.0, 0.0)), 2), 0.0);
		specular += lightColors[7]*pow(max(dot(N, normalize(normalize(lightDirs[7]) + normalize(eyeVec))), 0.0), shininess)*specScale*MAX_SPECULAR;
	}
	if(8 < numLights)
	{
		lambertTerm += lightColors[8]*max(dot(N,normalize(lightDirs[8])) * 400.0 / pow(distance(lightDirs[8], vec3(0.0, 0.0, 0.0)), 2), 0.0);
		specular += lightColors[8]*pow(max(dot(N, normalize(normalize(lightDirs[8]) + normalize(eyeVec))), 0.0), shininess)*specScale*MAX_SPECULAR;
	}
	if(9 < numLights)
	{
		lambertTerm += lightColors[9]*max(dot(N,normalize(lightDirs[9])) * 400.0 / pow(distance(lightDirs[9], vec3(0.0, 0.0, 0.0)), 2), 0.0);
		specular += lightColors[9]*pow(max(dot(N, normalize(normalize(lightDirs[9]) + normalize(eyeVec))), 0.0), shininess)*specScale*MAX_SPECULAR;
	}
	if(10 < numLights)
	{
		lambertTerm += lightColors[10]*max(dot(N,normalize(lightDirs[10])) * 400.0 / pow(distance(lightDirs[10], vec3(0.0, 0.0, 0.0)), 2), 0.0);
		specular += lightColors[10]*pow(max(dot(N, normalize(normalize(lightDirs[10]) + normalize(eyeVec))), 0.0), shininess)*specScale*MAX_SPECULAR;
	}
	if(11 < numLights)
	{
		lambertTerm += lightColors[11]*max(dot(N,normalize(lightDirs[11])) * 400.0 / pow(distance(lightDirs[11], vec3(0.0, 0.0, 0.0)), 2), 0.0);
		specular += lightColors[11]*pow(max(dot(N, normalize(normalize(lightDirs[11]) + normalize(eyeVec))), 0.0), shininess)*specScale*MAX_SPECULAR;
	}
	if(12 < numLights)
	{
		lambertTerm += lightColors[12]*max(dot(N,normalize(lightDirs[12])) * 400.0 / pow(distance(lightDirs[12], vec3(0.0, 0.0, 0.0)), 2), 0.0);
		specular += lightColors[12]*pow(max(dot(N, normalize(normalize(lightDirs[12]) + normalize(eyeVec))), 0.0), shininess)*specScale*MAX_SPECULAR;
	}
	if(13 < numLights)
	{
		lambertTerm += lightColors[13]*max(dot(N,normalize(lightDirs[13])) * 400.0 / pow(distance(lightDirs[13], vec3(0.0, 0.0, 0.0)), 2), 0.0);
		specular += lightColors[13]*pow(max(dot(N, normalize(normalize(lightDirs[13]) + normalize(eyeVec))), 0.0), shininess)*specScale*MAX_SPECULAR;
	}
	if(14 < numLights)
	{
		lambertTerm += lightColors[14]*max(dot(N,normalize(lightDirs[14])) * 400.0 / pow(distance(lightDirs[14], vec3(0.0, 0.0, 0.0)), 2), 0.0);
		specular += lightColors[14]*pow(max(dot(N, normalize(normalize(lightDirs[14]) + normalize(eyeVec))), 0.0), shininess)*specScale*MAX_SPECULAR;
	}
	if(15 < numLights)
	{
		lambertTerm += lightColors[15]*max(dot(N,normalize(lightDirs[15])) * 400.0 / pow(distance(lightDirs[15], vec3(0.0, 0.0, 0.0)), 2), 0.0);
		specular += lightColors[15]*pow(max(dot(N, normalize(normalize(lightDirs[15]) + normalize(eyeVec))), 0.0), shininess)*specScale*MAX_SPECULAR;
	}

	lambertTerm = min(lambertTerm, 1.0);
	FragColor = vec4(final_color.r * color.r * lambertTerm.r + specular.r,
			 final_color.g * color.g * lambertTerm.g + specular.g,
			 final_color.b * color.b * lambertTerm.b + specular.b,
			 final_color.a * color.a * silhouette);
}
