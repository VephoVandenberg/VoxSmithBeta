#include "app/app.h"

/** TO DO LIST
* Agenda
* - Get comfortable with noise algorithms
* Later
* - Perlin noise we should find out how 3d noise work
* - More blocks
* - Infinite terrain babe
* - Serialisation
* - Octrees
* - Greedy Meshing
* - SkyBox
* - Investigate math behind coordinate systems and matrices
* - Water Walta
* Works for now but might be improved
* - We have some working jumping, but it's still shit
*/

int main(int argc, char **argv)
{
	App::Application* app = new App::Application;
	app->run();
	delete app;

	return 0;
}