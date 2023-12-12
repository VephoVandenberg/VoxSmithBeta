/** TO DO LIST
* Agenda
* - Reducing vertex data
* Later
* - Use FastNoise lib for the terrain(We might be ok with our implementation)...
* - Block layering how to do that
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

#include "app/app.h"

int main(int argc, char **argv)
{
	int32_t a = 0;
	a |= 14 << 12;
	a |= 122 << 4;
	a |= 4;

	App::Application* app = new App::Application;
	app->run();
	delete app;

	return 0;
}