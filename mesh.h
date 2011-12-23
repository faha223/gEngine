#ifndef _MESH_H_
#define _MESH_H_

#include "matrices.h"
#include "vectorMath.h"
#include "renderer.h"
#include <GL/gl.h>

enum animEnum
{
	BASE, WALK, RUN, CROUCH, CROUCHWALK, JUMP, AIM, SHOOT
};

class mesh
{
public:
	mesh();						// Empty Constructor
	~mesh();					// destructor
	bool load(const char* filename, Renderer &renderer);
							// loads the model from a file
	bool loadTexture(textureLevel texture, const char* filename);
							// loads the texture from a file
	void rotate(const mat3 &rotationMatrix);		// apply a rotation to the mesh
	void translate(const vector3 &translationVector);	// apply a translation to the mesh
	mat4 getTransform() const;					// returns the transformation matrix
	void transform(const mat4 &transformationMatrix);	// apply a transformation to the mesh
	void getVerts(animEnum animation, int time, float *&verts, unsigned int &numVerts) const;
								// get the vertices at the current animation frame
	void loadTexture(textureLevel target, const char* filename, Renderer &r, GLenum filter);
								// Load a texture (use the renderer passed to load the texture, because
								//	that's the only way to make sure the texture memory truly gets freed
	GLuint* getTextures() const;				// Get the array of texture handles
	GLuint getTexture(textureLevel texture)const;	// Get a particular texture
	void clearTransform();				// clears the transformation by storing an identity matrix in the transformation matrix
private:
	unsigned short numTris, numVerts;	// Number of vertices and triangles
	unsigned short *tris;
	vector3 *vertices;			// Vertices of each triangle
	vector3 *tangents;			// Tangent vectors for each vertex
	vector3 *binormals;			// Binormal vectors for each vertex
	vector3 *normals;			// Normal vectors for each vertex
	vector2 *texCoords;			// texture coordinates in 2 dimensions
	GLuint texture[6];			// 5 texture handles (diffuse, normal, height, light, occlusion, and specular maps)
	mat4 transformation;			// 4x4 matrix to store the current transformation matrix
	// TODO: Add amination information to the mesh class
};

#endif
