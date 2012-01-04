#include "renderer.h"
#include "mesh.h"
#include "SteelBattalion.h"
#include <fstream>
#include <iostream>
using namespace SteelBattalion;

const unsigned short width = 0;		// Set the screen width (if 0, it uses the current screen resolution)
const unsigned short height = 0;	// Set the screen height (if 0, it uses the current screen height)
const unsigned short framerate = 60;	// Set the framerate
const unsigned short fov = 100;		// Set the field of view
const unsigned short demo = 10;		// Set the demo run time
const float Afilter = 16.0f;		// Set the level of anisotropic filtering
const float Aalias = 4.0f;		// Not yet implemented
const bool fullscreen = true;		// Set whether this should run in fullscreen or not

int main(int argc, char **argv)
{
	bool keyboard[512];
	for(int i = 0; i < 512; ++i)
		keyboard[i] = false;
	bool exit = false;
	Renderer renderer(width, height, framerate, fov, Afilter, Aalias, fullscreen);
	// If the renderer was not created properly, OH SHIT
	if(!renderer.isOkay())
	{
		printf("Renderer did not initialize\n");	// Let the user know
		return 0;					// SEPPUKU
	}

	// otherwise...
        printf("GL Version: %s, The Way it's Meant to be Played\n", glGetString(GL_VERSION));

	SteelBattalionController device;

	float *vertices = NULL;

	mesh Crate, Box;
	if(Crate.load("meshes/crate", renderer))
		printf("Model Crate loaded fine\n");
	else
		printf("Error loading Crate\n");
	if(Box.load("meshes/box", renderer))
		printf("Model Box loaded fine\n");
	else
		printf("Error loading Box\n");
	VBObject box;				// Make a new VBO
	box.handle = 0;				// Default value for its handle

	Crate.getVerts(BASE, 0, vertices, box.numVerts);

	glGenBuffers(1, &box.handle);		// and give it a VBO handle

	if(box.handle == 0)			// if the handle failed
	{
		printf("COLOSSAL FAILURE\n");	// Let the user know
		return 0;			// SEPPUKU
	}

	box.texture[DIFFUSE_MAP] = Crate.getTexture(DIFFUSE_MAP);
	box.texture[NORMAL_MAP] = Crate.getTexture(NORMAL_MAP);
	box.texture[SPECULAR_MAP] = Crate.getTexture(SPECULAR_MAP);
	box.texture[HEIGHT_MAP] = Crate.getTexture(HEIGHT_MAP);
	box.texture[LIGHT_MAP] = Crate.getTexture(LIGHT_MAP);
	box.texture[OCCLUSION_MAP] = Crate.getTexture(OCCLUSION_MAP);

	glBindBuffer(GL_ARRAY_BUFFER, box.handle);				// Bind the VBO
	glBufferData(GL_ARRAY_BUFFER, 14*box.numVerts*sizeof(float), vertices, GL_STATIC_DRAW);// and allocate its space

	renderer.switchToPhong();							// Switch to the phong shader
//	renderer.switchToParallax();							// Switch to parallax shader
//	renderer.switchToToon();							// Switch to the toon shader (to test it)
//	renderer.switchToGouraud();
//	renderer.switchToGouraudToon();
//	renderer.switchToWireframe();

	// Add a light to the scene (position vector, direction vector, color vector, , type)
	renderer.add_light(vector3(5.0f, 20.0f, 0.0f), vector3(0.0f, 1.0f, 0.0f), vector3(1.0f, 1.0f, 1.0f), 1.0f, POINT_LIGHT);
//	renderer.add_light(vector3(5.0f, 20.0f, 0.0f), vector3(0.0f, 1.0f, 0.0f), vector3(1.0f, 0.8f, 0.5f), 1.0f, POINT_LIGHT);
//	renderer.add_light(vector3(8.0f, -20.0f, 0.0f), vector3(0.0f, 1.0f, 0.0f), vector3(0.5f, 0.8f, 1.0f), 1.0f, POINT_LIGHT);
//	renderer.add_light(vector3(0.0f, -20.0f, 0.0f), vector3(0.0f, 1.0f, 0.0f), vector3(0.926f, 0.16f, 0.16f), 1.0f, POINT_LIGHT);

	// make some rotation storing variables
	float x = 0.0f, y = 0.0f;
	// Draw some frames in realtime

	int error;

	unsigned char *animation[45];
	for(size_t i = 0; i < 45; ++i)
	{
		animation[i] = new unsigned char[34];
		for(size_t j = 0; j < 34; ++j)
			animation[i][j] = 0;
	}
	std::ifstream file;
	file.open("animations/startScreen");
	if(!file.is_open())
		printf("animation not found\n");
	else
	{
		for(size_t i = 0; i < 45; ++i)
			for(size_t j = 0; j < 34; ++j)
				animation[i][j] = file.get();
		file.close();
	}

	int frame = 0;
	int time = 0;
	int lastRefresh = 0;
	int i = 0;

	SDL_Event keyevent;
	while(!(device.getButtonState(EJECT)||device.getButtonState(TOGGLE_FILTER_CONTROL)||device.getError()) || (device.getError() && (!exit)))
	{
		renderer.clearTransform();
		while(SDL_PollEvent(&keyevent))
		{
			switch(keyevent.type)
			{
				case SDL_KEYDOWN:
				{
					keyboard[keyevent.key.keysym.sym] = true;
					break;
				}
				case SDL_KEYUP:
				{
					keyboard[keyevent.key.keysym.sym] = false;
					break;
				}
			}
		}
		exit = keyboard[SDLK_ESCAPE];
		shaderType currentShader = renderer.getShaderType();

		if(device.getError())
		{
			x = 0.6f*float(i++)/float(framerate);
			y = x;
			if(keyboard[SDLK_F1] && (currentShader != PHONG_SHADER))
				renderer.switchToPhong();
			else if(keyboard[SDLK_F2] && (currentShader != PARALLAX_SHADER))
				renderer.switchToParallax();
			else if(keyboard[SDLK_F3] && (currentShader != GOURAUD_SHADER))
				renderer.switchToGouraud();
			else if(keyboard[SDLK_F4] && (currentShader != GOURAUD_TOON_SHADER))
				renderer.switchToGouraudToon();
			else if(keyboard[SDLK_F5] && (currentShader != WIREFRAME_SHADER))
				renderer.switchToWireframe();
			if(keyboard[SDLK_F7])
			{
				box.texture[DIFFUSE_MAP] = Crate.getTexture(DIFFUSE_MAP);
				box.texture[NORMAL_MAP] = Crate.getTexture(NORMAL_MAP);
				box.texture[SPECULAR_MAP] = Crate.getTexture(SPECULAR_MAP);
				box.texture[HEIGHT_MAP] = Crate.getTexture(HEIGHT_MAP);
				box.texture[LIGHT_MAP] = Crate.getTexture(LIGHT_MAP);
				box.texture[OCCLUSION_MAP] = Crate.getTexture(OCCLUSION_MAP);
			}
			if(keyboard[SDLK_F8])
			{
				box.texture[DIFFUSE_MAP] = Box.getTexture(DIFFUSE_MAP);
				box.texture[NORMAL_MAP] = Box.getTexture(NORMAL_MAP);
				box.texture[SPECULAR_MAP] = Box.getTexture(SPECULAR_MAP);
				box.texture[HEIGHT_MAP] = Box.getTexture(HEIGHT_MAP);
				box.texture[LIGHT_MAP] = Box.getTexture(LIGHT_MAP);
				box.texture[OCCLUSION_MAP] = Box.getTexture(OCCLUSION_MAP);
			}

		}
		else
		{
			if((abs(device.getSightChangeX()) > 16)||(abs(device.getSightChangeY()) > 16))
			{
				x += 0.00125f*device.getSightChangeX();
				y += 0.00125f*device.getSightChangeY();
			}

			vector3 color = vector3(0.0f, 0.0f, 0.0f);
			if(device.getButtonState(TOGGLE_FUEL_FLOW_RATE))
				color.x = 1.0f;
			if(device.getButtonState(TOGGLE_BUFFER_MAT))
				color.y = 1.0f;
			if(device.getButtonState(TOGGLE_VT_LOCATION))
				color.z = 1.0f;
			renderer.color3f(color.x, color.y, color.z);

			if(device.getButtonState(COM1) && (currentShader != PHONG_SHADER))
				renderer.switchToPhong();
			else if(device.getButtonState(COM2) && (currentShader != PARALLAX_SHADER))
				renderer.switchToParallax();
			else if(device.getButtonState(COM3) && (currentShader != GOURAUD_SHADER))
				renderer.switchToGouraud();
			else if(device.getButtonState(COM4) && (currentShader != GOURAUD_TOON_SHADER))
				renderer.switchToGouraudToon();
			else if(device.getButtonState(COM5) && (currentShader != WIREFRAME_SHADER))
				renderer.switchToWireframe();
			for(size_t i = 0; i < 33; ++i)										// if a Button is pressed
				if(device.getButtonState(ButtonEnum(i)))							//
					device.setLEDState(ButtonToLED(ButtonEnum(i)), 15, false);				// light up the corresponding LED

			if(device.getButtonState(LEFT_JOY_SIGHT_CHANGE))
				x = y = 0.0f;

			if(((time = SDL_GetTicks()) - lastRefresh) >= 10)							// If the time since the last time the lights were refreshed is
			{													// more than 10 milliseconds
				lastRefresh += (time - lastRefresh) - ((time-lastRefresh)%10);					// Add the correct number of milliseconds to the time (a multiple of 10)
				device.setRawLEDData(animation[frame++]);							// And increase the animation frame
				frame %= 45;											// loop back around if we pass 44
			}

			device.refreshLEDs();											// Push the updated LED values to the controller
		}
		if(x >= 6.28f)
			x -= 6.28f;
		if(y >= 6.28f)
			y -= 6.28f;

		renderer.setClearColori(device.getLeftPedal(), device.getMiddlePedal(), device.getRightPedal());		// Set the clear color for the screen to a function of the 3 foot pedals

		renderer.translatef(0.0f, 0.0f, -5.0f);
		renderer.pushTransform();
		renderer.rotatef(x, 0.0f, 1.0f, 0.0f);
		renderer.rotatef(y, 1.0f, 0.0f, 0.0f);

		Crate.getVerts(BASE, 0, vertices, box.numVerts);

		for(int i = 0; i < 3; ++i)
			for(int j = 0; j < 3; ++j)
			{
				renderer.color3f(float(i%2), float(j%2), float((i*j)%2));
				renderer.translatef(2*i-2, 2*j-2, 0.0f);
				renderer.push_VBO(box);
				renderer.translatef(-2*i+2, -2*j+2, 0.0f);
			}
		renderer.DrawScene();
		if((error = glGetError()))
			printf("GL_ERROR: %s\n", gluErrorString(error));
	}

	for(size_t i = 0; i < 45; ++i)
		delete [] animation[i];

	glDeleteBuffers(1, &box.handle);	// We don't need this VBO anymore

	// And... we're done here
	return 0;
}
