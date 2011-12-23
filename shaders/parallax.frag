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

	float silhouette = 1.0;
	if((newTexCoord.x > 1.0)||(newTexCoord.y > 1.0)||(newTexCoord.x < 0.0)||(newTexCoord.y < 0.0))
		silhouette = 0.0;

	vec3 lambertTerm = vec3(AMBIENT) + texture2D(lightMap, newTexCoord).xyz;
	vec3 specular = vec3(0.0);
	float specScale = texture2D(specularMap, newTexCoord).r;
	vec4 final_color = texture2D(diffuseMap, newTexCoord);
	vec3 N = normalize(TBN * (normalize(texture2D(normalMap, newTexCoord).xyz * 2.0 - vec3(1.0)))), L;

	// calculate the accumulated light from all 15 potential lights
	for(int i = 0; i < MAX_LIGHTS; ++i)
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
			 final_color.a * color.a * silhouette);
}
