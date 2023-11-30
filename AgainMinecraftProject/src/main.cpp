#include "app/app.h"

/** TO DO LIST
* Agenda
* - We need to apply squashing
* Later
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