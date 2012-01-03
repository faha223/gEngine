#version 330

#define MAX_LIGHTS 16
#define MAX_SPECULAR 1.0
#define AMBIENT 0.03
#define shininess 8.0
#define scale 0.4
#define bias 0.2
#define steps 64.0

in vec3 normal, eyeVec, tangent, binormal;
in vec3 lightDirs[MAX_LIGHTS];
in vec2 TexCoord0;
in float dist;

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

	vec2 newTexCoord = TexCoord0;

	float step = max((transpose(TBN)*eyeVec).z/steps, 0.001);
	vec2 delta = vec2(-(transpose(TBN)*eyeVec).x, (transpose(TBN)*eyeVec).y) * (scale * step / (transpose(TBN)*eyeVec).z);

	float texHeight = texture2D(heightMap, TexCoord0).r * scale - bias;
	float height = bias;

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

	for(int s = 0; (s < 32) && (abs(texHeight - midHeight) > 0.01); ++s)
	{
		if(midHeight > texHeight)
		{
			vHi = (vHi + vLo)*0.5;
			vHeightHi = midHeight;
		}
		else if(midHeight < texHeight)
		{
			vLo = (vHi + vLo)*0.5;
			vHeightLo = midHeight;
		}
		midHeight = (vHeightHi + vHeightLo) * 0.5;
		texHeight = texture2D(heightMap, (vHi+vLo)*0.5).r*scale-bias;
	}
	// The final texture coordinate is the midpoint value that gave a height within 0.01 of the correct height
	newTexCoord = (vHi + vLo)*0.5;

	float silhouette = (((newTexCoord.x < 0.0)||(newTexCoord.x > 1.0)||(newTexCoord.y < 0.0)||(newTexCoord.y > 1.0))?0.0:1.0);

	vec3 lambertTerm = texture2D(lightMap, newTexCoord).xyz;
	vec3 specular = vec3(0.0);
	float specScale = texture2D(specularMap, newTexCoord).r;
	vec4 final_color = texture2D(diffuseMap, newTexCoord);
	vec3 N = normalize(TBN * (normalize(texture2D(normalMap, newTexCoord).xyz * 2.0 - vec3(1.0)))), L;

	// calculate the accumulated light from all lights
	for(int i = 0; i < numLights; ++i)
	{
		if(i < MAX_LIGHTS)
		{
			lambertTerm += lightColors[i]*max(dot(N,normalize(lightDirs[i])) * 400.0 / pow(distance(lightDirs[i], vec3(0.0, 0.0, 0.0)), 2), 0.0);
			specular += lightColors[i]*pow(max(dot(N, normalize(normalize(lightDirs[i]) + normalize(eyeVec))), 0.0), shininess)*specScale*MAX_SPECULAR;
		}
	}

	lambertTerm = clamp(lambertTerm, AMBIENT, 1.0);
	FragColor = vec4(final_color.r * color.r * lambertTerm.r + specular.r,
			 final_color.g * color.g * lambertTerm.g + specular.g,
			 final_color.b * color.b * lambertTerm.b + specular.b,
			 final_color.a * color.a * silhouette);
}
