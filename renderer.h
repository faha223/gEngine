#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include "matrices.h"
#include "quaternion.h"
#include <string>
#include <vector>
#include <map>
#include <set>

//#define error printf("glError: %s\n", gluErrorString(glGetError()))

enum lightType
{
	POINT_LIGHT,					// A light that illuminates in all directions: has a position and a color
	SPOT_LIGHT,					// A light that illuminates in an infinite number of directions within a confined range of its direction: hs a position, color, direction, and range
	LASER_LIGHT					// A special directional light with a VERY small range
};

enum shaderType
{
	NO_SHADER,					// A shadertype for when there is no shader in use
	PHONG_SHADER,					// A per-pixel shader for normal realistic lighting
	PARALLAX_SHADER,
	TOON_SHADER,					// A per-pixel shader for cartoon style cel lighting
	GOURAUD_SHADER,					// A per vertex shader for faster lighting
	GOURAUD_TOON_SHADER, 				// A per vertex shader for faster cartoon style cel lighting
	WIREFRAME_SHADER				// A wirefram shader for quickly rendering the splines of the models
};

enum textureLevel
{
	DIFFUSE_MAP,					// A diffuse map tells you the base color of the texture at that point (RGB)
	NORMAL_MAP,					// A normal map tells you the normal of the surface that light is reflected off of at that point, this is used for more advanced lighting (RGB)
	SPECULAR_MAP,					// A specular map will tell you the specular level of the material at that point (Grayscale or RGB)
	HEIGHT_MAP,					// A height map will tell the parallax shader how high the given texel is relative to the other texels
	LIGHT_MAP,					// A light map will tell the shader how much light this texel inherently emits
	OCCLUSION_MAP					// An occulsion map will tell you how much light will be allowed into this point of the material (used for better lighting, not likely to be added anytime soon)
};

struct light
{
	vector3 color;					// Stores the color and luminosity of the light (color is the direction, luminosity is the magnitude
	vector3 position;				// Stores the position of the light;
	vector3 direction;				// Stores the direction the light points in: not used for point lights
	float range;					// Stores the range of the light (the maximum angle from the direction the light points in that light can travel from this source): not used in point lights
	lightType type;					// Stored an enumerator for the type of light
};

struct textBufferObject
{
	float x, y;
	std::string text;
	textBufferObject(const float &x, const float &y, const char *text);
};

class transformStack
{
private:
	std::vector<Quaternion> rotationStack;
	std::vector<vector3> translationStack;
	Quaternion localRotation;
	vector3 localTranslation;

public:
	transformStack();
	void pushTransform();
	void popTransform();

	void rotatef(const float&, const float&, const float&, const float&);
	void translatef(const float&, const float&, const float&);

	void rotateq(const Quaternion&);

	mat4 totalTransform() const;

	void clear();
};

struct VBObject						// A vertex buffer object
{
	GLuint handle;					// Has a handle
	GLuint texture[6];
	unsigned int numVerts;				// and a number of vertices
	shaderType shader;				// and the specifier for which shader to draw this with
	mat4 transform;
	vector4 color;
	VBObject();					// Constructor
};

struct vertex						// A single vertex can be used in a trangle or a line, or by itself as a single point
{
	vector3 position, normal;			// A vertex consists of a single position vector, a single normal vector,
	vector2 texCoords;				// and a single set of texture coordinates
};

struct Triangle						// A single generic triangle. Not part of a Vertex Buffer Object directly
{
	vertex verts[3];				// three vertices
	GLuint texid;					// one texture id
	shaderType shader;				// and one shader
	vector4 color;					// and one color modifier
};

struct vertexArray					// Stores a single vertex array. These can be rendered directly, but are faster rendered from gfx memory with VBOs
{
	GLuint texture;					// Stores the texture handle
	float *data;					// Stores the vertex data
	short numVerts, numTris;			// Stores the number of vertices and triangles in the vertex array
	short *indices;					// Stores the indices of the vertices in the order they are to be rendered
	shaderType shader;				// Stores the shader used to render this object
};

mat4 rotmat(const float& = 0.0f, const float& = 0.0f, const float& = 0.0f, const float& = 0.0f);

class Renderer						// The main class of this library. This class is used to handle all rendering in the application
{
private:
	bool cv_fullscreen, cv_vsync;							// Stores the flags for fullscreen rendering and vertical sync
	unsigned short r_width, r_height, r_framerate, r_fov;				// Stores the resolution, framerate, and field of view
	vector3 clearColor;								// stores the clear color (bound 0, 0, 0, to 1, 1, 1)
	vector4 color;
	float cv_af, cv_aa;								// Stores the current levels of anisotrophic filtering and antialiasing
	unsigned int sv_lastFrameUpdate;						// Stores the time of the last frame refresh
	SDL_Surface *screen;								// Stores a pointer to the screen surface
	std::vector<vertexArray> VAS;							// Stores all of the Vertex Arrays before they get put into vertex buffer objects
	std::vector<VBObject> VBOS;							// Stores all the vertex buffer objects until I have the scenegraph working
	std::vector<Triangle> TS;							// Stores individual triangles passed to the renderer
	std::vector<GLuint> textures;							// Stores all the textures currently in use by the program, this is where they get deleted from
	std::vector<textBufferObject> textBuffer;					// Stores all the text to be drawn to the screen at a given time (drawn without the depth buffer, cleared after each frame)
	bool OKAY;									// Maintains whether the renderer is in good working order
	std::vector<light> lights;							// Stores all lights in the current scene
	GLhandleARB toonShader, phongShader, parallaxShader, gouraudShader, gouraudToonShader, wireframe;		// Stores the handles for the shaders
	void loadShader(const shaderType& type, const char* shadername);		// Loads the shader associated with it, and stores it in the space for said shader
	GLint shaderLightCount, shaderLightDirectionPointer, shaderLightColorPointer, shaderDiffuseMap, shaderNormalMap,
		shaderHeightMap, shaderSpecularMap, shaderLightMap, shaderOcclusionMap, shaderModelview, shaderProjection, shaderColor;
	GLuint currentTexture[6];							// Stores the handles of the currently bound texture set
											// Stores the pointer for the shader light array
	float max_anisotropy;								// Stores the maximum level of anistropic filtering supported by the hardware
	int lastFrame;									// Stores the time of the last frame
	void TrisToVBOs();
	float framerate;								// stores the framerate
	transformStack transforms;
public:
	// Constructor
	Renderer(	const unsigned short &width = 640,
			const unsigned short &height = 480,
			const unsigned short &framerate = 125,
			const unsigned short &fov = 90,
			const float &afilter = 1.0f,
			const float &aalias = 1.0f,
			const bool &fullscreen = true);
	// Destructor
	~Renderer();
	// Init() initializes the screen using the passed values of width, height, framerate, and field of view; Returns a bool to tell the program if it succeeded or not
	bool Init(	const unsigned short &width = 640,
			const unsigned short &height = 480,
			const unsigned short &framerate = 125,
			const unsigned short &fov = 90,
			const float &afilter = 1.0f,
			const float &aalias = 1.0f,
			const bool &fullscreen = true);
	// prepareScreen() prepares the screen for rendering. Use this whenever you change the field of view. This does not clear the screen
	bool prepareScreen();
	// DrawScene() draws the current scene to the screen and clears all pending vertex arrays, storing them in video memory
	bool DrawScene();
	// isOkay() returns true if the renderer is in good working order, false otherwise
	bool isOkay();
	// getWidth() returns the width of the screen
	unsigned short getWidth();
	// getHeight() returns the height of the screen
	unsigned short getHeight();
	// getFramerate() returns the current max framerate of the screen (any number between 30 and 125)
	unsigned short getFramerate();
	// getFOV() returns the current FOV (any number between 60 and 150)
	unsigned short getFOV();
	// push_VBO puts a vertex buffer object directly onto the render stack
	bool push_VBO(const VBObject&);
	// push_VA puts a vertex array directly onto the render stack
	bool push_VA(const vertexArray&);
	// push_Triangle puts a triangle directly onto the render stack
	bool push_Triangle(const Triangle&);
	// add Light, returns the index of the light in the vector
	int add_light(const vector3& = vector3(0.0f, 0.0f, 0.0f), const vector3& = vector3(0.0f, -1.0f, 0.0f), const vector3& = vector3(1.0f, 1.0f, 1.0f), const float& = 1.0f, const lightType& = POINT_LIGHT);
	// changeLight, changes the attributes of a light already in the renderer, cannot change the type
	int change_light(const vector3& = vector3(0.0f, 0.0f, 0.0f), const vector3& = vector3(0.0f, 0.0f, 0.0f), const float& = 0.0f);
	// loadTexture loads the texture specified and returns the handle
	GLuint loadTexture(const char* filename, const GLenum& = GL_LINEAR_MIPMAP_LINEAR);
	// deleteTexture deletes a texture when the user calls it. It deletes only the texture specified by the user
	void deleteTexture(const GLuint* handle);
	// getShaderType returns the shaderType of the current shader
	shaderType getShaderType();
	// switchToPhong switches the renderer to Phong (per pixel) shader
	void switchToPhong();
	// switchToParallax switches the renderer to the Parallax Mapping shader
	void switchToParallax();
	// switchToToon switches the renderer to the Toon shader
	void switchToToon();
	// switchToGouraud switches the renderer to the Gouraud (per vertex) shader
	void switchToGouraud();
	// switchToGouraudToon switches the renderer to the Gouraud (per vertex) Toon shader
	void switchToGouraudToon();
	// switchToWireframe switches the renderer to the Wireframe shader
	void switchToWireframe();
	// pushTransform pushes the current quaternion and vector transformation onto the transform stack
	void pushTransform();
	// popTransform pops the current transform off of the transform stack
	void popTransform();
	// rotatef rotates applies the provided axis angle rotation to the current local transform
	void rotatef(const float&, const float&, const float&, const float&);
	// rotateq applies the provided quaternion rotation to the existing localRotation
	void rotateq(const Quaternion&);
	// translatef applies the provided vector3 transformation to the current local transform
	void translatef(const float&, const float&, const float&);
	// clearTransform clears the transform stack
	void clearTransform();
	// Sets the clear color for the screen (the color displayed for each pixel in which there is no geometry)
	void setClearColori(const int& R = 0, const int& G = 0, const int& B = 0);
	// Sets the clear color for the screen (the color displayed for each pixel in which there is no geometry)
	void setClearColorf(const float& R = 0.0f, const float& G = 0.0f, const float& B = 0.0f);
	// Sets the color we modulate with the geometry drawn after this call until the next call is made. default color at the beginning of every draw is { 1.0f, 1.0f, 1.0f, 1.0f }
	void color4f(const float &r, const float &g, const float &b, const float &a);
	// Sets the color we modulate with the geometry drawn after this call until the next call is made. default color at the beginning of every draw is { 1.0f, 1.0f, 1.0f }
	void color3f(const float &r, const float &g, const float &b);
	// Adds text to the text buffer to be drawn this frame
	void drawStringXY(const float &x = 0.0f, const float &y = 0.0f, const char *text = NULL);
	// Adds text to the text buffer to be drawn this frame
	void drawStringXYZ(const float &x = 0.0f, const float &y = 0.0f, const float &z = 0.0f, const char *text = NULL);
};

Renderer &operator <<(Renderer&, const VBObject&);
Renderer &operator <<(Renderer&, const vertexArray&);
Renderer &operator <<(Renderer&, const Triangle&);

#endif
