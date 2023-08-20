#include "src/app/app.h"

/** TO DO LIST
* Agenda
* - Create a mesh
* - Fix camera
* Later
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