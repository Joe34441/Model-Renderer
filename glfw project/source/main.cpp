#include "ObjectRenderer.h"

#define DEFAULT_SCREENWIDTH 1200
#define DEFAULT_SCREENHEIGHT 800

int main()
{
	ObjectRenderer* myApp = new ObjectRenderer();
	myApp->Run("Object Renderer", DEFAULT_SCREENWIDTH, DEFAULT_SCREENHEIGHT, false);
	delete myApp;
	return 0;
}
