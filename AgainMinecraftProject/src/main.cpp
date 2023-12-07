#include "app/app.h"

/** TO DO LIST
* Agenda
* Later
* - Block layering how to do that
* - Reducing vertex data
* - More blocks
* - Infinite terrain babe
* - Serialisation
* - Octrees
* - Greedy Meshing
* - SkyBox
* - Investigate math behind coordinate systems and matrices
* - Water Walta
* Works for now but might be improved
* - We need to apply squashing(Working but might be imrpoved)
*/

int main(int argc, char **argv)
{
	App::Application* app = new App::Application;
	app->run();
	delete app;

	return 0;
}