#include "app/app.h"

/** TO DO LIST
* Agenda
* - Make refactoring over some code in raycasting in App and chunk.cpp
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