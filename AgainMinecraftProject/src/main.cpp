#include "app/app.h"

/** TO DO LIST
* Agenda
*  - Block or face outline
* Later
*
* - Perlin noise we should find out how 3d noise work
* - SkyBox
* - Investigate math behind coordinate systems and matrices
* Works for now but might be improved
*/

int main(int argc, char **argv)
{
	App::Application* app = new App::Application;
	app->run();
	delete app;

	return 0;
}