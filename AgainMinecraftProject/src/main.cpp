/** TO DO LIST
* AGENDA
* - MipMaps
* - FrameTime and FPS
* - Let there be light(Shadow maps)
* LATER
* - Perhaps we need some kind of instrument for enabling physics
* - SkyBox or maybe clouds, maybe Moon and Sun
* - Trees
* - Serialisation
* - More vegetation
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