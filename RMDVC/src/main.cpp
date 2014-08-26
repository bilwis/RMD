#include "libtcod.hpp"
#include "Engine.hpp"
#include <stdio.h>



int main() {
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	Engine* engine = new Engine();

    while ( !TCODConsole::isWindowClosed() ) {
    	engine->update();
    	engine->render();
		TCODConsole::flush();
    }

	delete engine;
    return 0;
}
