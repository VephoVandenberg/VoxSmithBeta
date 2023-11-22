#include "app/app.h"

/** TO DO LIST
* Agenda
* Later
* - Perlin noise we should find out how 3d noise work
* - More blocks
* - SkyBox
* - Investigate math behind coordinate systems and matrices
* - Water Walta
* Works for now but might be improved
* - We probably shouldn't place block in the mid air, and shouldn't be placed where the player is
* - We need to improve jumping because it works like shit, but I don't care for now at least
*/

int main(int argc, char **argv)
{
	App::Application* app = new App::Application;
	app->run();
	delete app;

	return 0;
}