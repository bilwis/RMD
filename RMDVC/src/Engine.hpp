#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "libtcod.hpp"
#include "Actor.hpp"
#include "Map.hpp"
#include "GUI.hpp"

/** This class provides the render() and update() methods for the game loop,
* as well as holding the loaded Actors, Map and providing
* functions for GUI handling.
*/
enum class GameState {GUI, GAME};

class Engine {
private:
	/** During rendering, actors and the map are rendered on
	* the gameConsole object, which is blitted onto the root Console.
	*/
	TCODConsole *gameConsole;

	GameState state;

public :

    TCODList<Actor *> actors;
    Actor *player;
    Map *map;

	Gui *gui;
	GuiBodyViewer *guiBodyViewer;
	GuiTextBox *sampleTextBox;
	GuiListChooser *sampleList;
 
    Engine();
    ~Engine();
    void update();
    void render();

};
 
extern Engine engine;

#endif
