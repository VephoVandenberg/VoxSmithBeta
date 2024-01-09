/** TO DO LIST
* AGENDA
* - Let there be light(ambient light for now)
* LATER
* - Water Walta
* - Perhaps we need some kind of enabling physics
* - Trees
* - Serialisation
* - FOG
* - Octrees
* - Greedy Meshing
* - SkyBox or maybe clouds, maybe Moon and Sun
* - Investigate math behind coordinate systems and matrices 
* NOTES:
* - We might have some swiftness over the biomes with the exponent and division stuff.
* - Multithreading works but still some bugs occur, also consider threadpool, and using futures instead threads
* - Biomes will work for now, but it can be better
*/

#include "app/app.h"

int main(int argc, char **argv)
{
	App::Application* app = new App::Application;
	app->run();
	delete app;

	return 0;
}