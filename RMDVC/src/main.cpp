#include "libtcod.hpp"
#include "Actor.hpp"
#include "Map.hpp"
#include "Engine.hpp"
#include <stdio.h>

Engine engine;

int main() {
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

    while ( !TCODConsole::isWindowClosed() ) {
    	engine.update();
    	engine.render();
		TCODConsole::flush();
    }
    return 0;
}
