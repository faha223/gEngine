#include "renderer.h"
#include <cstdio>
#include <cmath>
#include <fstream>

using namespace std;

#define pi 3.14159265f

// This is used to load a shader from file on the disk (you could write a shader and have it plaintext,
//	or better yet, include it to cheat and make it part of the source code, but I'm not gonna do
//	either of those things because having the shader source outside the app is more useful to the
//	user of the engine
char* loadTextFromFile(const char* = NULL);

Renderer::Renderer(const unsigned short &width, const unsigned short &height, const unsigned short &framerate, const unsigned short &fov, const float &afilter, const float &aalias, const bool &fullscreen)
{
	screen = NULL;					// Initialize the screen pointer, no dingleberries
	Init(width, height, framerate, fov, afilter, aalias, fullscreen);		// Init the renderer, and keep track of its state
}

bool Renderer::Init(const unsigned short &width, const unsigned short &height, const unsigned short &framerate, const unsigned short &fov, const float &afilter, const float &aalias, const bool &fullscreen)
{
	// Store the given values for width, height, framerate, field of view, anisotrophic filtering, antialiasing, and fullscreen
	lastFrame = 0;
	color = vector4(1.0f, 1.0f, 1.0f, 1.0f);
	r_width = width;
	r_height = height;
	r_framerate = framerate;
	r_fov = fov;
	cv_af = afilter;
	cv_aa = aalias;
	cv_fullscreen = fullscreen;
	// We haven't yet renderered the screen
	sv_lastFrameUpdate = 0;
	// Init the SDL video subsystem
	int i_error = SDL_Init(SDL_INIT_VIDEO);
	// if the video subsystem failed to init, init has failed
	if(i_error == -1)
	{
		printf("SDL_Init Failed: %s\n", SDL_GetError());
		SDL_Quit();
		OKAY = false;
		return false;
	}

	// Ask the video card what's going on
	const SDL_VideoInfo* vidcard = SDL_GetVideoInfo();

	// If the given width was 0, ask the card what width it recommends
	if(r_width == 0)
		r_width = vidcard->current_w;
	// If the given height was 0, ask the card what height it recommends
	if(r_height == 0)
		r_height = vidcard->current_h;

	// create a video screen, fullscreen only if it has been decided
	if(cv_fullscreen)
		screen = SDL_SetVideoMode(r_width, r_height, 32, SDL_HWSURFACE | SDL_FULLSCREEN | SDL_OPENGL | SDL_DOUBLEBUF);
	else
		screen = SDL_SetVideoMode(r_width, r_height, 32, SDL_HWSURFACE | SDL_OPENGL | SDL_DOUBLEBUF);

	// If the screen could not be created, then init has failed
	if(screen == NULL)
	{
		OKAY = false;
		return false;
	}
	// Don't draw the cursor in this window
	SDL_ShowCursor(SDL_DISABLE);
	// Everything has init just fine
	glShadeModel(GL_SMOOTH);	// Use a smooth shading model when not using shaders to render (not often)

	glClearColor(clearColor.x, clearColor.y, clearColor.z, 1.0f);
//	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);				// Black
//	glClearColor(0.390625f, 0.58203125f, 0.92578125f, 1.0f);	// Cornflower Blue

	glEnable(GL_DEPTH_TEST);					// Enable the depth test
	glEnable(GL_TEXTURE_2D);					// Enable texturing
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glDepthFunc(GL_LEQUAL);						// Use linear depth testing
	glEnableClientState(GL_VERTEX_ARRAY);				// Enable support for vertex arrays
	glEnableClientState(GL_NORMAL_ARRAY);				// Enable support for normal arrays
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);			// Enable support for texture coordinate arrays

	OKAY = prepareScreen();						// prepare the screen, store the output into the OKAY variable

	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);			// Ensure that we have control over the buffers for buffer swaps (double buffering)
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anisotropy);// Check the max supported level of anisotrophic filtering
	if(max_anisotropy < cv_af)					// if the current level is higher than the max
		cv_af = max_anisotropy;					// cut it back

	// Load Shaders
	loadShader(PHONG_SHADER, "phong");				// Loads the phong shader from phong.vert and phong.frag
	loadShader(PARALLAX_SHADER, "parallax");			// Loads the phong shader from phong.vert and phong.frag
	loadShader(TOON_SHADER, "toon");				// Loads the toon shader from toon.vert and toon.frag
	loadShader(GOURAUD_SHADER, "gouraud");				// Loads the gouraud shader from gouraud.vert and gouraud.frag
	loadShader(GOURAUD_TOON_SHADER, "gouraud-toon");		// Loads the gouraud toon shader from gouraudToon.vert and gouraudToon.frag
	loadShader(WIREFRAME_SHADER, "wireframe");			// Loads the wireframe shader from wireframe.vert and wireframe.frag

	currentTexture[DIFFUSE_MAP] = 0;
	currentTexture[NORMAL_MAP] = 0;
	currentTexture[SPECULAR_MAP] = 0;
	currentTexture[HEIGHT_MAP] = 0;
	currentTexture[LIGHT_MAP] = 0;
	currentTexture[OCCLUSION_MAP] = 0;

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(8);

	return OKAY;							// We're good
}

// This prepares the screen for drawing, and must be called anytime the video settings change
bool Renderer::prepareScreen()
{
	glViewport(0, 0, r_width, r_height);					// Make sure the game knows how big to make the viewport
	glMatrixMode(GL_PROJECTION);						// And use the projection matrix
	glLoadIdentity();							// Load the identity matrix into the projection matrix
	float fH = tan(r_fov / 720.0f * pi);
	float fW = float(r_width)*fH/float(r_height);
	glFrustum(-fW, fW, -fH, fH, 1.0f, 1000.0f);
	glMatrixMode(GL_MODELVIEW);						// Go back to the modelview matrix
	glLoadIdentity();							// and load the identity again
	return true;								// I have no way of knowing if this failed or not
}

// This call uses all the geometry passed to the renderer since the last time it was called and draws it all to the screen (then gets rid of it)
bool Renderer::DrawScene()
{
	float Projection[16];
	glClear(GL_DEPTH_BUFFER_BIT);	// Clear the depth buffer
	glClear(GL_COLOR_BUFFER_BIT);	// And color buffer
	glLoadIdentity();

	GLint currentProg;

	glGetIntegerv(GL_CURRENT_PROGRAM, &currentProg);

	shaderLightCount = glGetUniformLocation(currentProg, "numLights");
	glUniform1i(shaderLightCount, lights.size());

	shaderColor = glGetUniformLocation(currentProg, "color");
<<<<<<< HEAD
	glUniform4f(shaderColor, color.x, color.y, color.z, color.w );

=======
>>>>>>> 509b30421f90c574540548a4fcd025aea7b1d5a3

	if(lights.size() > 0)
	{
		GLfloat lightDirs[lights.size() * 3];
		GLfloat lightCols[lights.size() * 3];

		for(unsigned int i = 0; i < lights.size(); ++i)
		{
			lightDirs[3*i] = lights[i].position.x;
			lightDirs[3*i+1] = lights[i].position.y;
			lightDirs[3*i+2] = lights[i].position.z;
			lightCols[3*i] = lights[i].color.x;
			lightCols[3*i+1] = lights[i].color.y;
			lightCols[3*i+2] = lights[i].color.z;
		}

		shaderLightDirectionPointer = glGetUniformLocation(currentProg, "lightDir");
		shaderLightColorPointer = glGetUniformLocation(currentProg, "lightColors");
		glUniform3fv(shaderLightDirectionPointer, lights.size(), lightDirs);
		glUniform3fv(shaderLightColorPointer, lights.size(), lightCols);
	}
	else
	{
		glUniform1i(shaderLightCount, 0);
	}

	TrisToVBOs();			// Draw the triangles (this doesn't work correctly yet)
	TS.clear();			// and get rid of them

	// Oh dearest me, I've forgotten my vertex arrays
	for(unsigned int i = 0; i < VBOS.size(); ++i)	// then draw the VBOS
	{
//		glMultMatrixf(VBOS[i].transform.data);
//		glGetFloatv(GL_MODELVIEW_MATRIX, Modelview);
		glGetFloatv(GL_PROJECTION_MATRIX, Projection);
		glUniformMatrix4fv(shaderModelview, 1, GL_FALSE, VBOS[i].transform.data);
		glUniformMatrix4fv(shaderProjection, 1, GL_FALSE, Projection);
<<<<<<< HEAD
=======
		glUniform4f(shaderColor, VBOS[i].color.x, VBOS[i].color.y, VBOS[i].color.z, VBOS[i].color.w );
>>>>>>> 509b30421f90c574540548a4fcd025aea7b1d5a3
		glBindBuffer(GL_ARRAY_BUFFER, VBOS[i].handle);		// Bind the VBO
		glVertexAttribPointer(8, 2, GL_FLOAT, GL_FALSE, 14*sizeof(float), (void *)0);			// Specify the size, type, stride, and offset of the texcoords
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14*sizeof(float), (void *)(2*sizeof(float)));	// tangents
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14*sizeof(float), (void *)(5*sizeof(float)));	// binormals
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 14*sizeof(float), (void *)(8*sizeof(float)));	// normals
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14*sizeof(float), (void *)(11*sizeof(float)));	// and vertex coordinates
		if(glIsTexture(VBOS[i].texture[DIFFUSE_MAP]) == GL_TRUE)
		{
			if(currentTexture[DIFFUSE_MAP] != VBOS[i].texture[DIFFUSE_MAP])
			{
				currentTexture[DIFFUSE_MAP] = VBOS[i].texture[DIFFUSE_MAP];
				glActiveTexture(GL_TEXTURE0);					// Specify and
				glBindTexture(GL_TEXTURE_2D, VBOS[i].texture[DIFFUSE_MAP]);	// Bind the first texture (diffuse map)
				printf("Diffuse map: %d\n", VBOS[i].texture[DIFFUSE_MAP]);
			}
			glUniform1i(shaderDiffuseMap, 0);
		}
		else
			printf("No diffuse map: %d\n", VBOS[i].texture[DIFFUSE_MAP]);
		if(glIsTexture(VBOS[i].texture[NORMAL_MAP]) == GL_TRUE)
		{
			if(currentTexture[NORMAL_MAP] != VBOS[i].texture[NORMAL_MAP])
			{
				currentTexture[NORMAL_MAP] = VBOS[i].texture[NORMAL_MAP];
				glActiveTexture(GL_TEXTURE1);					// Specify and
				glBindTexture(GL_TEXTURE_2D, VBOS[i].texture[NORMAL_MAP]);	// Bind the second texture (normal map)
				printf("Normal map: %d\n", VBOS[i].texture[NORMAL_MAP]);
			}
			glUniform1i(shaderNormalMap, 1);
		}
		else
			printf("No normal map: %d\n", VBOS[i].texture[NORMAL_MAP]);
		if(glIsTexture(VBOS[i].texture[SPECULAR_MAP]) == GL_TRUE)
		{
			if(currentTexture[SPECULAR_MAP] != VBOS[i].texture[SPECULAR_MAP])
			{
				currentTexture[SPECULAR_MAP] = VBOS[i].texture[SPECULAR_MAP];
				glActiveTexture(GL_TEXTURE2);					// Specify and
				glBindTexture(GL_TEXTURE_2D, VBOS[i].texture[SPECULAR_MAP]);	// Bind the second texture (normal map)
				printf("Specular map: %d\n", VBOS[i].texture[SPECULAR_MAP]);
			}
			glUniform1i(shaderSpecularMap, 2);
		}
		else
			printf("No specular map: %d\n", VBOS[i].texture[SPECULAR_MAP]);
		if(glIsTexture(VBOS[i].texture[HEIGHT_MAP]) == GL_TRUE)
		{
			if(currentTexture[HEIGHT_MAP] != VBOS[i].texture[HEIGHT_MAP])
			{
				currentTexture[HEIGHT_MAP] = VBOS[i].texture[HEIGHT_MAP];
				glActiveTexture(GL_TEXTURE3);					// Specify and
				glBindTexture(GL_TEXTURE_2D, VBOS[i].texture[HEIGHT_MAP]);	// Bind the first texture (diffuse map)
				printf("Height map: %d\n", VBOS[i].texture[HEIGHT_MAP]);
			}
			glUniform1i(shaderHeightMap, 3);
		}
		else
			printf("No height map: %d\n", VBOS[i].texture[HEIGHT_MAP]);
		if(glIsTexture(VBOS[i].texture[LIGHT_MAP]) == GL_TRUE)
		{
			if(currentTexture[LIGHT_MAP] != VBOS[i].texture[LIGHT_MAP])
			{
				currentTexture[LIGHT_MAP] = VBOS[i].texture[LIGHT_MAP];
				glActiveTexture(GL_TEXTURE4);					// Specify and
				glBindTexture(GL_TEXTURE_2D, VBOS[i].texture[LIGHT_MAP]);	// Bind the second texture (normal map)
				printf("Light map: %d\n", VBOS[i].texture[LIGHT_MAP]);
			}
			glUniform1i(shaderLightMap, 4);
		}
		else
			printf("No light map: %d\n", VBOS[i].texture[LIGHT_MAP]);
		if(glIsTexture(VBOS[i].texture[OCCLUSION_MAP]) == GL_TRUE)
		{
			if(currentTexture[OCCLUSION_MAP] != VBOS[i].texture[OCCLUSION_MAP])
			{
				currentTexture[OCCLUSION_MAP] = VBOS[i].texture[OCCLUSION_MAP];
				glActiveTexture(GL_TEXTURE5);					// Specify and
				glBindTexture(GL_TEXTURE_2D, VBOS[i].texture[OCCLUSION_MAP]);	// Bind the second texture (normal map)
				printf("Occlusion map: %d\n", VBOS[i].texture[OCCLUSION_MAP]);
			}
			glUniform1i(shaderOcclusionMap, 5);
		}
		else
			printf("No occlusion map: %d\n", VBOS[i].texture[OCCLUSION_MAP]);
		glDrawArrays(GL_TRIANGLES, 0, VBOS[i].numVerts);	// And Draw!
		glLoadIdentity();
	}

	VBOS.clear();		// forget the VBOs. They're not leaving graphics memory, we just don't know in advance if we need to draw them next frame

	// I was gonna add in a simple font based FPS ticker, but it got complicated

	SDL_GL_SwapBuffers();								// swap buffers
	glFinish();									// and flush to the screen

	return true;
}

bool Renderer::isOkay()				// Quick access to renderer attribues
{	return OKAY;	}

unsigned short Renderer::getWidth()		//
{	return r_width;	}

unsigned short Renderer::getHeight()		//
{	return r_height;	}

unsigned short Renderer::getFramerate()		//
{	return r_framerate;	}

unsigned short Renderer::getFOV()		//
{	return r_fov;	}

void Renderer::setClearColori(const int& R, const int& G, const int& B)
{
	clearColor = vector3(float(R)/255.0f, float(G)/255.0f, float(B)/255.0f);
	glClearColor(clearColor.x, clearColor.y, clearColor.z, 1.0f);
}

void Renderer::setClearColorf(const float& R, const float& G, const float& B)
{
	clearColor = vector3(R, G, B);
	glClearColor(clearColor.x, clearColor.y, clearColor.z, 1.0f);
}

bool Renderer::push_VBO(const VBObject& vbo)
{
	VBOS.push_back(vbo);			// It's ready to be drawn, this is not intended for use with scene objects as it doesn't depend on the camera or world space
	VBOS[VBOS.size()-1].transform = transforms.totalTransform();
<<<<<<< HEAD
=======
	VBOS[VBOS.size()-1].color = color;
>>>>>>> 509b30421f90c574540548a4fcd025aea7b1d5a3
	return true;				// At some point, there will be error checking to prevent using too much video ram
}

bool Renderer::push_VA(const vertexArray& va)
{
	return true;				// Doesn't work yet
}

bool Renderer::push_Triangle(const Triangle& t)
{
	return true;
}

// This reaches the destination, but it takes the wrong(est) path to get there
void Renderer::TrisToVBOs()
{
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_TRIANGLES);
	for(size_t i = 0; i < TS.size(); ++i)
	{
		for(int j = 0; j < 3; ++j)
		{
			glNormal3f(TS[i].verts[j].normal.x, TS[i].verts[j].normal.y, TS[i].verts[j].normal.z);
			glVertex3f(TS[i].verts[j].position.x, TS[i].verts[j].position.y, TS[i].verts[j].position.z);
		}
	}
	glEnd();

	//TODO: all of it
}

GLuint Renderer::loadTexture(const char* filename, const GLenum& filter)
{
	// go ahead and declare/initialize the basics
	int error = 0;
	GLuint texture = 0;
	SDL_Surface *surface = NULL;

	// Load the image if a filename was provided
	if(filename != NULL)
		surface = IMG_Load(filename);
	else
	{
		printf("No filename provided\n");
		return texture;
	}
	// If loading the image failed, we're done here
	if(surface == NULL)
	{
		printf("Error loading image %s\n", filename);
		return texture;
	}

	// Create a texture buffer to store it in when we're done
	glGenTextures(1, &texture);
	if((error = glGetError())) printf("Error loading texture: %s\n", gluErrorString(error));

	// If creating a buffer failed, we're done here
	if((texture == 0)||(glIsTexture(texture == GL_TRUE)))
	{
		printf("Error generating texture\n");
		return texture;
	}

	// Now we begin turning this surface into a texture
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);			// Not sure why I use this
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);			// Use the specified filtering technique
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);			// Always wrap
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);			// Always wrap
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, cv_af);		// use the specified level of anisotropic filtering

	switch(surface->format->BitsPerPixel)
	{
		case 3:
		case 6:
		case 12:
		case 24:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, 3, surface->w, surface->h, 0, GL_RGB, GL_UNSIGNED_BYTE, surface->pixels);
			break;
		}
		case 4:
		case 8:
		case 16:
		case 32:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, 4, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
			break;
		}
	}

	glGenerateMipmap(GL_TEXTURE_2D);

	textures.push_back(texture);
	SDL_FreeSurface(surface);
	return texture;
}

void Renderer::deleteTexture(const GLuint* handle)
{
	glDeleteBuffers(1, handle);
}

transformStack::transformStack()
{
	localRotation = Quaternion::identity;
	localTranslation = vector3(0.0f, 0.0f, 0.0f);
}

void transformStack::rotatef(const float& w, const float& x, const float& y, const float& z)
{
	localRotation = Quaternion::fromAxisAngle(w, x, y, z)*localRotation;
	localTranslation = Quaternion::fromAxisAngle(w, x, y, z).matrixRotation()*localTranslation;
}

void transformStack::rotateq(const Quaternion &q)
{
	localRotation = q*localRotation;
	localTranslation = q.matrixRotation()*localTranslation;
}

void Renderer::rotateq(const Quaternion& q)
{	transforms.rotateq(q);	}

void Renderer::rotatef(const float& w, const float& x, const float& y, const float& z)
{	transforms.rotatef(w, x, y, z);	}

void transformStack::translatef(const float& x, const float& y, const float& z)
{	localTranslation += vector3(x, y, z);	}

void Renderer::translatef(const float& x, const float& y, const float& z)
{	transforms.translatef(x, y, z);	}

void Renderer::color4f(const float& r, const float& g, const float& b, const float& a)
{
	color = vector4(r, g, b, a);
}

void Renderer::color3f(const float& r, const float& g, const float& b)
{
	color = vector4(r, g, b, color.w);
}

void transformStack::pushTransform()
{
	rotationStack.push_back(localRotation);
	translationStack.push_back(localTranslation);
	localTranslation = vector3(0.0f, 0.0f, 0.0f);
	localRotation = Quaternion::identity;
}

void Renderer::pushTransform()
{	transforms.pushTransform();	}

void transformStack::popTransform()
{
	localRotation = rotationStack.back();
	rotationStack.pop_back();
	localTranslation = translationStack.back();
	translationStack.pop_back();
}

void Renderer::popTransform()
{	transforms.popTransform();	}

mat4 transformStack::totalTransform() const
{
	Quaternion qOut = Quaternion::identity;
	vector3 vOut = vector3(0.0f, 0.0f, 0.0f);

	for(size_t i = 0; i < rotationStack.size(); ++i)
	{
		qOut = rotationStack[i] * qOut;// combine the quaternions, and use them to rotate the translation vectors
		vOut += translationStack[i];
	}

	qOut = localRotation * qOut;
	vOut += localTranslation;

	mat4 out = qOut.matrixTransformation();
	out.data[12] = vOut.x;
	out.data[13] = vOut.y;
	out.data[14] = vOut.z;
	return out;
}

void transformStack::clear()
{
	rotationStack.clear();
	translationStack.clear();
	localRotation = Quaternion::identity;
	localTranslation = vector3(0.0f, 0.0f, 0.0f);
}

void Renderer::clearTransform()
{	transforms.clear();	}

int Renderer::add_light(const vector3& position, const vector3& direction, const vector3& color, const float& range, const lightType& type)
{
	light newLight;
	newLight.position = position;
	newLight.direction = direction;
	newLight.range = range;
	newLight.color = color;
	newLight.type = type;
	lights.push_back(newLight);
	return 0;
}

Renderer::~Renderer()		// Close everything and clean up
{
	VBOS.clear();		// Clear
	VAS.clear();		// All
	TS.clear();		// Dynamically
	lights.clear();		// Allocated Memory
	glDeleteObjectARB(phongShader);		// delete shaders
	glDeleteObjectARB(parallaxShader);	//
	glDeleteObjectARB(toonShader);		//
	glDeleteObjectARB(gouraudShader);	//
	glDeleteObjectARB(gouraudToonShader);	//
	glDeleteObjectARB(wireframe);		//
	for(unsigned int i = 0; i < textures.size(); ++i)	// gotta delete the textures before we get rid of their references
		glDeleteBuffers(1, &textures[i]);
	textures.clear();
	SDL_FreeSurface(screen);// And return it to the system
	SDL_Quit();
}

Renderer& operator <<(Renderer& R, const VBObject& vbo)
{
	R.push_VBO(vbo);	// Call the native function
	return R;
}

Renderer& operator <<(Renderer& R, const vertexArray& va)
{
	R.push_VA(va);		// Call the native function
	return R;
}

Renderer& operator <<(Renderer& R, const Triangle& t)
{
	R.push_Triangle(t);	// Call the native function
	return R;
}

// This loads text from a file, I plan to use this for loading shaders
char* loadTextFromFile(const char* filename)
{
	char *out = NULL;			// Be prepared to return null if no filename was provided or a bad filename was provided
	if(filename == NULL)			// if no filename
		return out;			// return
	ifstream file(filename, ios::in);	// open the file
	if(!file.is_open())			// if bad filename
		return out;			// return
	file.seekg(0, ios::end);		// go to the end
	unsigned int count = file.tellg();	// record the cursor position so we know how many characters are in this bitch
	file.seekg(0, ios::beg);		// go back to the beginning

	out = new char[count + 1];			// allocate space for all these characters

	for(unsigned int i = 0; i < count; ++i)	// step through the file
		out[i] = file.get();		// and get them all in the array

	out[count] = '\0';			// end with null
	file.close();				// close the file

	return out;				// and return the array pointer
}

void Renderer::loadShader(const shaderType &type, const char *shadername)
{
	bool GEOM = false;

        if(shadername == NULL)                  // if no filename given
		return;
        ifstream file((string("shaders/") + string(shadername) + string(".vert")).c_str(), ios::in);    // try to open the vertex shader
        if(!file.is_open())                     // if the file doesn't exist
		return;
        file.close();
        file.open((string("shaders/") + string(shadername) + string(".frag")).c_str(), ios::in);        // try to open the fragment shader
        if(!file.is_open())                     // if the file doesn't exist
                return;                       // cant load the shader
        file.close();

        file.open((string("shaders/") + string(shadername) + string(".geom")).c_str(), ios::in);        // try to open the fragment shader
        GEOM = file.is_open();                     // don't bother with geometry shaders if they aren't there
        file.close();

        char *vs, *gs, *fs;

        GLhandleARB vertBMap, geomBMap, fragBMap, shader;

        shader = glCreateProgramObjectARB();
        vertBMap = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
        fragBMap = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

        vs = loadTextFromFile((string("shaders/") + string(shadername) + string(".vert")).c_str()); // load the file
        fs = loadTextFromFile((string("shaders/") + string(shadername) + string(".frag")).c_str()); // load the file
        const char* vBMap = vs;
        const char* fBMap = fs;

        glShaderSourceARB(vertBMap, 1, &vBMap, NULL);
        glShaderSourceARB(fragBMap, 1, &fBMap, NULL);

        free(vs);
	free(fs);

        char *infoLog = NULL;
        GLint length = 0;

        glCompileShaderARB(fragBMap);
        glCompileShaderARB(vertBMap);

        glGetShaderiv(fragBMap, GL_INFO_LOG_LENGTH, &length);
        infoLog = new char[length + 1];
        glGetShaderInfoLog(fragBMap, length, &length, infoLog);
        infoLog[length] = '\0';
	printf("%s, frag-compilelog: %s\n", shadername, infoLog);
        delete [] infoLog;

	if(GEOM)
	{
		geomBMap = glCreateShaderObjectARB(GL_GEOMETRY_SHADER_ARB);
		gs = loadTextFromFile((string("shaders/") + string(shadername) + string(".geom")).c_str()); // load the file
		const char* gBMap = gs;
		glShaderSourceARB(geomBMap, 1, &gBMap, NULL);
		free(gs);
		glCompileShaderARB(geomBMap);

	        length = 0;
	       	glGetShaderiv(geomBMap, GL_INFO_LOG_LENGTH, &length);
	       	infoLog = new char[length + 1];
	        glGetShaderInfoLog(geomBMap, length, &length, infoLog);
	        infoLog[length] = '\0';
		printf("%s, geom-compilelog: %s\n", shadername, infoLog);
	        delete [] infoLog;
	}

        length = 0;
        glGetShaderiv(vertBMap, GL_INFO_LOG_LENGTH, &length);
        infoLog = new char[length + 1];
        glGetShaderInfoLog(vertBMap, length, &length, infoLog);
        infoLog[length] = '\0';
	printf("%s, vert-compilelog: %s\n", shadername, infoLog);
        delete [] infoLog;

        glAttachObjectARB(shader, vertBMap);
	if(GEOM)
	        glAttachObjectARB(shader, geomBMap);
        glAttachObjectARB(shader, fragBMap);

	glBindAttribLocation(shader, 0, "Tangent");
	glBindAttribLocation(shader, 1, "Binormal");
	glBindAttribLocation(shader, 2, "Normal");
	glBindAttribLocation(shader, 3, "Vertex");
	glBindAttribLocation(shader, 8, "MultiTexCoord0");
	glBindFragDataLocation(shader, 0, "FragColor");

        glLinkProgramARB(shader);

	switch(type)
	{
		case TOON_SHADER:
		{
			toonShader = shader;
			break;
		}
		case PHONG_SHADER:
		{
			phongShader = shader;
			break;
		}
		case PARALLAX_SHADER:
		{
			parallaxShader = shader;
			break;
		}
		case GOURAUD_SHADER:
		{
			gouraudShader = shader;
			break;
		}
		case GOURAUD_TOON_SHADER:
		{
			gouraudToonShader = shader;
			break;
		}
		case WIREFRAME_SHADER:
		{
			wireframe = shader;
			break;
		}
		default:
		{
			break;
		}
	}
}

shaderType Renderer::getShaderType()
{
	GLint shaderi;
	glGetIntegerv(GL_CURRENT_PROGRAM, &shaderi);
	GLuint shader = shaderi;
	if(shader == phongShader)
		return PHONG_SHADER;
	else if(shader == parallaxShader)
		return PARALLAX_SHADER;
	else if(shader == gouraudShader)
		return GOURAUD_SHADER;
	else if(shader == gouraudToonShader)
		return GOURAUD_TOON_SHADER;
	else if(shader == wireframe)
		return WIREFRAME_SHADER;
	return NO_SHADER;
}

void Renderer::switchToPhong()
{
	glUseProgramObjectARB(phongShader);		// Renderer with the Phong shader
	shaderLightCount = glGetUniformLocationARB(phongShader, "numLights");
	shaderLightDirectionPointer = glGetUniformLocationARB(phongShader, "lightDir");
	shaderLightColorPointer = glGetUniformLocationARB(phongShader, "lightColors");
	shaderDiffuseMap = glGetUniformLocationARB(phongShader, "diffuseMap");
	shaderNormalMap = glGetUniformLocationARB(phongShader, "normalMap");
	shaderHeightMap = glGetUniformLocationARB(phongShader, "heightMap");
	shaderSpecularMap = glGetUniformLocationARB(phongShader, "specularMap");
	shaderLightMap = glGetUniformLocationARB(phongShader, "lightMap");
	shaderOcclusionMap = glGetUniformLocationARB(phongShader, "occlusionMap");
	shaderModelview = glGetUniformLocationARB(phongShader, "Modelview");
	shaderProjection = glGetUniformLocationARB(phongShader, "Projection");
	shaderColor = glGetUniformLocationARB(phongShader, "color");
}

void Renderer::switchToParallax()
{
	glUseProgramObjectARB(parallaxShader);		// Renderer with the Phong shader
	shaderLightCount = glGetUniformLocationARB(parallaxShader, "numLights");
	shaderLightDirectionPointer = glGetUniformLocationARB(parallaxShader, "lightDir");
	shaderLightColorPointer = glGetUniformLocationARB(parallaxShader, "lightColors");
	shaderDiffuseMap = glGetUniformLocationARB(parallaxShader, "diffuseMap");
	shaderNormalMap = glGetUniformLocationARB(parallaxShader, "normalMap");
	shaderHeightMap = glGetUniformLocationARB(parallaxShader, "heightMap");
	shaderSpecularMap = glGetUniformLocationARB(parallaxShader, "specularMap");
	shaderLightMap = glGetUniformLocationARB(parallaxShader, "lightMap");
	shaderOcclusionMap = glGetUniformLocationARB(parallaxShader, "occlusionMap");
	shaderModelview = glGetUniformLocationARB(parallaxShader, "Modelview");
	shaderProjection = glGetUniformLocationARB(parallaxShader, "Projection");
	shaderColor = glGetUniformLocationARB(parallaxShader, "color");
}

void Renderer::switchToToon()
{
	glUseProgramObjectARB(toonShader);		// Renderer with the Toon shader
	shaderLightCount = glGetUniformLocationARB(toonShader, "numLights");
	shaderLightDirectionPointer = glGetUniformLocation(toonShader, "lightDir");
	shaderLightColorPointer = glGetUniformLocation(toonShader, "lightColors");
	shaderDiffuseMap = glGetUniformLocationARB(toonShader, "diffuseMap");
	shaderNormalMap = glGetUniformLocationARB(toonShader, "normalMap");
	shaderHeightMap = glGetUniformLocationARB(toonShader, "heightMap");
	shaderSpecularMap = glGetUniformLocationARB(toonShader, "specularMap");
	shaderOcclusionMap = glGetUniformLocationARB(toonShader, "occlusionMap");
	shaderModelview = glGetUniformLocationARB(toonShader, "Modelview");
	shaderProjection = glGetUniformLocationARB(toonShader, "Projection");
	shaderColor = glGetUniformLocationARB(toonShader, "color");
}

void Renderer::switchToGouraud()
{
	glUseProgramObjectARB(gouraudShader);		// Renderer with the Gouraud shader
	shaderLightCount = glGetUniformLocationARB(gouraudShader, "numLights");
	shaderLightDirectionPointer = glGetUniformLocation(gouraudShader, "lightDir");
	shaderLightColorPointer = glGetUniformLocation(gouraudShader, "lightColors");
	shaderDiffuseMap = glGetUniformLocationARB(gouraudShader, "diffuseMap");
	shaderNormalMap = glGetUniformLocationARB(gouraudShader, "normalMap");
	shaderHeightMap = glGetUniformLocationARB(gouraudShader, "heightMap");
	shaderSpecularMap = glGetUniformLocationARB(gouraudShader, "specularMap");
	shaderOcclusionMap = glGetUniformLocationARB(gouraudShader, "occlusionMap");
	shaderModelview = glGetUniformLocationARB(gouraudShader, "Modelview");
	shaderProjection = glGetUniformLocationARB(gouraudShader, "Projection");
	shaderColor = glGetUniformLocationARB(gouraudShader, "color");
}

void Renderer::switchToGouraudToon()
{
	glUseProgramObjectARB(gouraudToonShader);	// Renderer with the Gouraud Toon shader
	shaderLightCount = glGetUniformLocationARB(gouraudToonShader, "numLights");
	shaderLightDirectionPointer = glGetUniformLocation(gouraudToonShader, "lightDir");
	shaderLightColorPointer = glGetUniformLocation(gouraudToonShader, "lightColors");
	shaderDiffuseMap = glGetUniformLocationARB(gouraudToonShader, "diffuseMap");
	shaderNormalMap = glGetUniformLocationARB(gouraudToonShader, "normalMap");
	shaderHeightMap = glGetUniformLocationARB(gouraudToonShader, "heightMap");
	shaderSpecularMap = glGetUniformLocationARB(gouraudToonShader, "specularMap");
	shaderOcclusionMap = glGetUniformLocationARB(gouraudToonShader, "occlusionMap");
	shaderModelview = glGetUniformLocationARB(gouraudToonShader, "Modelview");
	shaderProjection = glGetUniformLocationARB(gouraudToonShader, "Projection");
	shaderColor = glGetUniformLocationARB(gouraudToonShader, "color");
}

void Renderer::switchToWireframe()
{
	glUseProgramObjectARB(wireframe);		// Renderer with Wireframe shader
	shaderDiffuseMap = glGetUniformLocationARB(wireframe, "diffuseMap");
	shaderModelview = glGetUniformLocationARB(wireframe, "Modelview");
	shaderProjection = glGetUniformLocationARB(wireframe, "Projection");
}

VBObject::VBObject()
{
	texture[0] = 0;
	texture[1] = 0;
	texture[2] = 0;
	texture[3] = 0;
	texture[4] = 0;
}
