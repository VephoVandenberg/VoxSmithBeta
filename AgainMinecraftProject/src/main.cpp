#include "app/app.h"

/** TO DO LIST
* Agenda
* - updating mesh with ray(i.e. adding block and removing block)
* Later 
* - ECS
* - Investigate math behind coordinate systems and matrices
*/

int main(int argc, char **argv)
{
	App::Application* app = new App::Application;
	app->run();
	delete app;

	return 0;
}