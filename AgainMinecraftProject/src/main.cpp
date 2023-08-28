#include "app/app.h"

/** TO DO LIST
* Agenda
* - ECS
* - block placing bug
* Later 
* - Take into consideration having global blocks and construction of meshes from that blocks
* - Investigate math behind coordinate systems and matrices
* Works for now but might be improved
* - updating mesh with ray(i.e. adding block and removing block)
*/

int main(int argc, char **argv)
{
	App::Application* app = new App::Application;
	app->run();
	delete app;

	return 0;
}