#include "src/app/app.h"

/** TO DO LIST
* Agenda
* Later
* - Create a mesh
* - Face cullung
* - Investigate math behind coordinate systems and matrices
*/

int main(int argc, char **argv)
{
	App::Application* app = new App::Application;
	app->run();
	delete app;

	return 0;
}