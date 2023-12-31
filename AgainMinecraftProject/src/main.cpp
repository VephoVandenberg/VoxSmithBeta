/** TO DO LIST
* Agenda
* - Infinite terrain babe(Maybe we will introduce thread pool)
* Later
* - More blocks
* - Biomes
* - Trees
* - Let there be light
* - Perhaps we need some kind of enabling physics
* - Serialisation
* - FOG
* - Octrees
* - Greedy Meshing
* - SkyBox or maybe clouds, maybe Moon and Sun
* - Investigate math behind coordinate systems and matrices
* - Water Walta
* Works for now but might be improved
* - We are okay for now with therrain gen but might be better
* 
* @NOTES:
* We might have some swiftness over the biomes with the exponent and division stuff.
*/

#include "app/app.h"

int main(int argc, char **argv)
{
	App::Application* app = new App::Application;
	app->run();
	delete app;

	return 0;
}