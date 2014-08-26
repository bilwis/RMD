#define _SCL_SECURE_NO_WARNINGS

#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "libtcod.hpp"

class Actor;
class ActorMap;
class Map;
class ActionScheduler;
class Gui;
class GuiBodyViewer;
class GuiTextBox;
class GuiListChooser;

#include <fstream>
#include <stdio.h>
#include <boost/archive/xml_oarchive.hpp> // saving
#include <boost/archive/xml_iarchive.hpp> // loading
#include <map>

enum class GameState { GUI, GAME, INIT };

/** This class provides the render() and update() methods for the game loop,
* as well as holding the loaded Actors, Map and providing
* functions for GUI handling.
*/
class Engine {
private:
	/** During rendering, actors and the map are rendered on
	* the gameConsole object, which is blitted onto the root Console.
	*/
	TCODConsole* gameConsole;

	GameState state;

public :

	ActorMap* actors;

    Actor* player;
    Map* map;

	ActionScheduler* scheduler;

	Gui* gui;
	GuiBodyViewer* guiBodyViewer;
	GuiTextBox* sampleTextBox;
	GuiListChooser* sampleList;
 
    Engine();
    ~Engine();

    void update();
    void render();

};
 
#endif
