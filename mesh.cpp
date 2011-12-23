#include "mesh.h"		// This is a no-brainer
#include "renderer.h"		// This is for texture handling
#include <fstream>		// this is for File loading
#include <string>
using namespace std;

mesh::mesh()
{
	vertices = NULL;
	tangents = NULL;
	binormals = NULL;
	normals = NULL;
	texCoords = NULL;
	transformation = mat4::identity;
	for(size_t i = 0; i < 5; ++i)
		texture[i] = 0;
}

bool mesh::load(const char* filename, Renderer &renderer)
{
	// Clear any previously loaded data;
	if(vertices != NULL)
	{
		delete vertices;
		vertices = NULL;
	}
	if(tangents != NULL)
	{
		delete tangents;
		tangents = NULL;
	}
	if(binormals != NULL)
	{
		delete binormals;
		binormals = NULL;
	}
	if(normals != NULL)
	{
		delete normals;
		normals = NULL;
	}
	if(texCoords != NULL)
	{
		delete texCoords;
		texCoords = NULL;
	}
	numVerts = 0;
	numTris = 0;

	// No filename provided
	if(filename == NULL)
		return false;
	ifstream file;
	// Open the file
	file.open((string(filename)+"/mesh").c_str());
	// If the file did not exist, we exit this here
	if(!file.is_open())
		return false;

	// TODO: Load the formatted file once it's open
	file >> numVerts;
	vertices = new vector3[numVerts];
	for(unsigned int i = 0; i < numVerts; ++i)
		file >> vertices[i].x >> vertices[i].y >> vertices[i].z;

	file >> numTris;
	tris = new unsigned short[numTris * 3];
	for(unsigned int i = 0; i < numTris; ++i)
		file >> tris[3*i] >> tris[3*i+1] >> tris[3*i+2];

	tangents = new vector3[numTris * 3];
	binormals = new vector3[numTris * 3];
	normals = new vector3[numTris * 3];

	for(unsigned int i = 0; i < numTris; ++i)
	{
		file >> tangents[3*i].x >> tangents[3*i].y >>tangents[3*i].z >> binormals[3*i].x >> binormals[3*i].y >> binormals[3*i].z >> normals[3*i].x >> normals[3*i].y >> normals[3*i].z;
		file >> tangents[3*i+1].x >> tangents[3*i+1].y >> tangents[3*i+1].z >> binormals[3*i+1].x >> binormals[3*i+1].y >> binormals[3*i+1].z >> normals[3*i+1].x >> normals[3*i+1].y >> normals[3*i+1].z;
		file >> tangents[3*i+2].x >> tangents[3*i+2].y >> tangents[3*i+2].z >> binormals[3*i+2].x >> binormals[3*i+2].y >> binormals[3*i+2].z >> normals[3*i+2].x >> normals[3*i+2].y >> normals[3*i+2].z;
	}

	texCoords = new vector2[numTris * 3];
	for(unsigned int i = 0; i < numTris; ++i)
		file >> texCoords[3*i].x >> texCoords[3*i].y >> texCoords[3*i+1].x >> texCoords[3*i+1].y >> texCoords[3*i+2].x >> texCoords[3*i+2].y;

	// Close the file after we're done reading it
	file.close();

	loadTexture(DIFFUSE_MAP, (string(filename) + string("/diffuse.png")).c_str(), renderer, GL_LINEAR_MIPMAP_LINEAR);
	loadTexture(NORMAL_MAP, (string(filename) + string("/normal.png")).c_str(), renderer, GL_LINEAR_MIPMAP_LINEAR);
	loadTexture(SPECULAR_MAP, (string(filename) + string("/specular.png")).c_str(), renderer, GL_LINEAR_MIPMAP_LINEAR);
	loadTexture(HEIGHT_MAP, (string(filename) + string("/height.png")).c_str(), renderer, GL_LINEAR_MIPMAP_LINEAR);
	loadTexture(LIGHT_MAP, (string(filename) + string("/light.png")).c_str(), renderer, GL_LINEAR_MIPMAP_LINEAR);
	loadTexture(OCCLUSION_MAP, (string(filename) + string("/occlusion.png")).c_str(), renderer, GL_LINEAR_MIPMAP_LINEAR);
	if((texture[DIFFUSE_MAP] * texture[NORMAL_MAP] * texture[SPECULAR_MAP] * texture[HEIGHT_MAP] * texture[LIGHT_MAP] * texture[OCCLUSION_MAP]))
		return true;
	return false;
}

void mesh::loadTexture(textureLevel target, const char *filename, Renderer &r, GLenum filter)
{
	texture[target] = r.loadTexture(filename, filter);
}

GLuint mesh::getTexture(textureLevel target) const
{
	return texture[target];
}

void mesh::rotate(const mat3 &r)
{
	mat4 transform(	r.data[0], 	r.data[1],	r.data[2],	0.0f,
			r.data[3], 	r.data[4],	r.data[5],	0.0f,
			r.data[3], 	r.data[4],	r.data[5],	0.0f,
			0.0f,		0.0f,		0.0f,		1.0f	);
	transformation = transform * transformation;
}

void mesh::translate(const vector3 &t)
{
	transformation = transformation + mat4(	1.0f, 0.0f, 0.0f, t.x,
						0.0f, 1.0f, 0.0f, t.y,
						0.0f, 0.0f, 1.0f, t.z,
						0.0f, 0.0f, 0.0f, 1.0f	);
}

void mesh::transform(const mat4 &t)
{
	transformation = t * transformation;
}

mat4 mesh::getTransform() const
{
	return transformation;
}

void mesh::getVerts(animEnum a, int t, float *&v, unsigned int &numV) const
{
	if(v != NULL)
	{
		delete [] v;
		v = NULL;
	}
	v = new float[42*numTris];
	numV = 3*numTris;
	vector3 buffer3;
	vector4 buffer4;
	for(unsigned int i = 0; i < numTris; ++i)
	{
		v[42*i]		= texCoords[3*i].x;
		v[42*i+1]	= texCoords[3*i].y;
		v[42*i+2] 	= tangents[3*i].x;
		v[42*i+3] 	= tangents[3*i].y;
		v[42*i+4] 	= tangents[3*i].z;
		v[42*i+5] 	= binormals[3*i].x;
		v[42*i+6] 	= binormals[3*i].y;
		v[42*i+7] 	= binormals[3*i].z;
		v[42*i+8] 	= normals[3*i].x;
		v[42*i+9]	= normals[3*i].y;
		v[42*i+10]	= normals[3*i].z;
		v[42*i+11]	= vertices[tris[3*i]].x;
		v[42*i+12]	= vertices[tris[3*i]].y;
		v[42*i+13]	= vertices[tris[3*i]].z;

		v[42*i+14]	= texCoords[3*i+1].x;
		v[42*i+15]	= texCoords[3*i+1].y;
		v[42*i+16] 	= tangents[3*i].x;
		v[42*i+17] 	= tangents[3*i].y;
		v[42*i+18] 	= tangents[3*i].z;
		v[42*i+19] 	= binormals[3*i].x;
		v[42*i+20] 	= binormals[3*i].y;
		v[42*i+21] 	= binormals[3*i].z;
		v[42*i+22] 	= normals[3*i+1].x;
		v[42*i+23]	= normals[3*i+1].y;
		v[42*i+24]	= normals[3*i+1].z;
		v[42*i+25]	= vertices[tris[3*i+1]].x;
		v[42*i+26]	= vertices[tris[3*i+1]].y;
		v[42*i+27]	= vertices[tris[3*i+1]].z;

		v[42*i+28]	= texCoords[3*i+2].x;
		v[42*i+29]	= texCoords[3*i+2].y;
		v[42*i+30] 	= tangents[3*i].x;
		v[42*i+31] 	= tangents[3*i].y;
		v[42*i+32] 	= tangents[3*i].z;
		v[42*i+33] 	= binormals[3*i].x;
		v[42*i+34] 	= binormals[3*i].y;
		v[42*i+35] 	= binormals[3*i].z;
		v[42*i+36] 	= normals[3*i+2].x;
		v[42*i+37]	= normals[3*i+2].y;
		v[42*i+38]	= normals[3*i+2].z;
		v[42*i+39]	= vertices[tris[3*i+2]].x;
		v[42*i+40]	= vertices[tris[3*i+2]].y;
		v[42*i+41]	= vertices[tris[3*i+2]].z;
	}
}

void mesh::clearTransform()
{
	transformation = mat4::identity;
}

mesh::~mesh()
{
	if(vertices != NULL)
		delete vertices;
	if(tangents != NULL)
		delete tangents;
	if(binormals != NULL)
		delete binormals;
	if(normals != NULL)
		delete normals;
	if(texCoords != NULL)
		delete texCoords;
	for(unsigned int i = 0; i < 6; ++i)
		if(texture[i] != 0)
			glDeleteTextures(1, &texture[i]);
}
