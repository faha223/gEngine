#include "sceneGraph.h"

SGnode::SGnode(SGnode* p)
{	parent = p;	}

SGnode::~SGnode()
{	Kill();	}

void SGnode::Kill()
{
	for(size_t i = 0; i < child.size(); ++i)
	{
		child[i]->Kill();
		delete child[i];
		child[i] = NULL;
	}
	child.clear();
}

void SGnode::Draw(Renderer& display)
{
	display.pushTransform();
	display.rotateq(rotation);
	display.translatef(translation.x, translation.y, translation.z);
	display.push_VBO(data);
	for(size_t i = 0; i < child.size(); ++i)
		child[i]->Draw(display);
	display.popTransform();
}
