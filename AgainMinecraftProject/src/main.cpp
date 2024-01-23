/** TO DO LIST
* AGENDA
* - Let there be light(Shadow maps)
* -		steps: 
*			1. Learn framebuffers.
*			2. Learn and make shadow map
* LATER
* - Perhaps we need some kind of enabling physics
* - SkyBox or maybe clouds, maybe Moon and Sun
* - Trees
* - Serialisation
* - Octrees
* - Greedy Meshing
* - More vegetation
* - Investigate math behind coordinate systems and matrices 
* NOTES:
* - We might have some swiftness over the biomes with the exponent and division stuff.
* - Multithreading works but still some bugs occur, also consider threadpool, and using futures instead threads.
* - Biomes will work for now, but it can be better
* - Having simple fog but could be performed better.
* - Having nice water, transperency, but I need to add some color change when underwater.
*/

#include "app/app.h"

int main(int argc, char **argv)
{
	App::Application* app = new App::Application;
	app->run();
	delete app;

	return 0;
}