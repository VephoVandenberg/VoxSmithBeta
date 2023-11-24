#include "app/app.h"

/** TO DO LIST
* Agenda
* - We probably shouldn't place block in the mid air, and shouldn't be placed where the player is
* Later
* - Perlin noise we should find out how 3d noise work
* - More blocks
* - SkyBox
* - Investigate math behind coordinate systems and matrices
* - Water Walta
* Works for now but might be improved
* - We have some working jumping, but it's still shit
*/

int main(int argc, char **argv)
{
	App::Application* app = new App::Application;
	app->run();
	delete app;

	return 0;
}