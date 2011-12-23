#ifndef _SCENEGRAPH_H_
#define _SCENEGRAPH_H_

#include "renderer.h"
#include "quaternion.h"
#include <vector>

struct SGnode					// A single node of the scene graph
{
	SGnode(SGnode *p);			// You must provide it the pointer to its parent when you create it (I don't know how long this will stay in here)
	~SGnode();				// It has to clear its dynamic memory before it can be let to die
	void Kill();				// This function prepares this node to die
	void Draw(Renderer&);			// This method draws the object to the scene with its own transformation relative to its parent (basically just adds its transformation to the stack, draws, and then pops back)
	VBObject data;				// This VBOobject holds the geometry data for this node. It's basically a pointer to the VBO that holds this node's data
	std::vector<SGnode*> child;			// This vector points to all the children nodes of this node
	SGnode *parent;				// This is a pointer to the parent node. It might not be necessary when this is all done
	Quaternion rotation;			// A quaternion to hold the rotation component of this nodes transformation relative to its parent
	vector3 translation;			// This vector3 holds the translation component of this nodes transformation relative to its parent
};

class SceneGraph				// A scenegraph is a tree representation of the spatial dispertion of objects relative to their parents. It makes transformation of things a little easier
{
private:
	SGnode *tree;				// It needs to have a pointer to the root node of its tree of goodies

public:
	SceneGraph();				// A constructor
	~SceneGraph();				// A destructor (kills off the tree)
	void DrawScene(Renderer&);		// A method to draw the scene graph to the screen using the provided renderer
	void clear();				// and a method to clear it out (kills the tree)
};

#endif
