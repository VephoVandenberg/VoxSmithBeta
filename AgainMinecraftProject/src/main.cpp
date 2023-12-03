#include "app/app.h"

/** TO DO LIST
* Agenda
* - Think about chunk rendering and storing blocks only about blocktype and face bools
* - Now think about how texture IDs must be set to every block face
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
* - We need to apply squashing(Working but might be imrpoved)
*/

int main(int argc, char **argv)
{
	App::Application* app = new App::Application;
	app->run();
	delete app;

	return 0;
}