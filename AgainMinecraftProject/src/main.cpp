#include "app/app.h"

/** TO DO LIST
* Agenda
* - Think more about how the texture is being set to the face,
*	it might be improved for certain
* Later
* - Investigate math behind coordinate systems and matrices
* Works for now but might be improved
* - Results are pretty solid in block editing but some outline must be taken into consideration
*/

int main(int argc, char **argv)
{
	App::Application* app = new App::Application;
	app->run();
	delete app;

	return 0;
}