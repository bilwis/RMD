#include "Engine.hpp"
#include <stdio.h>


Engine::Engine() {
    TCODConsole::initRoot(120,80,"libtcod C++ tutorial",false);
	gameConsole = new TCODConsole(120, 70);

    player = new Actor(40,25,'@',TCODColor::white);

    player->destructible = new Destructible(100);
    player->destructible->body = new Body("Body.xml");

    actors.push(player);
    actors.push(new Actor(60,13,'@',TCODColor::yellow));
    map = new Map(120,70);

	gui = new Gui();
	sampleContainer = new GuiContainer("sampleContainer", 10, 10, 40, 20,
		TCODColor::yellow, TCODColor::darkBlue, true, "TestBox");
	sampleTextBox = new GuiTextBox("sampleTextBox", 1, 1, 38, 10,
		"This is a test string to check the functionality of this TextBox",
		TCODColor::red, TCODColor::green);
	sampleContainer->addElement(sampleTextBox);
	gui->addContainer(sampleContainer);
}

void createBasicUI(Gui gui)
{

}

Engine::~Engine() {
    actors.clearAndDelete();
    delete map;
}

void Engine::update() {
    TCOD_key_t key;
    TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS,&key,NULL);
    switch(key.vk) {
        case TCODK_UP :
            if ( ! map->isWall(player->x,player->y-1)) {
                player->y--;
            }
        break;
        case TCODK_DOWN :
            if ( ! map->isWall(player->x,player->y+1)) {
                player->y++;
            }
        break;
        case TCODK_LEFT :
            if ( ! map->isWall(player->x-1,player->y)) {
                player->x--;
            }
        break;
        case TCODK_RIGHT :
            if ( ! map->isWall(player->x+1,player->y)) {
                player->x++;
            }
        break;
        case TCODK_CHAR:
        	switch (key.c) {
        		case 'k':
        			player->destructible->body->removeRandomPart();
					sampleTextBox->setText("OH GOD, WHY!?");
        		break;
        	}
        	break;
        default:break;
    }
}

/** This function performs the rendering.
  * Actors and the map are rendered on gameConsole, UI is rendered on uiConsole,
  * and then both are blitted onto the root console.
  *
  * @brief Rendering function.
  */
void Engine::render() {
	
	TCODConsole::root->clear();
	gameConsole->clear();

	// draw the map
	map->render(gameConsole);
	// draw the actors
	for (Actor **iterator=actors.begin();
	    iterator != actors.end(); iterator++) {
	    (*iterator)->render(gameConsole);
	}

	TCODConsole::blit(gameConsole, 0, 0, 0, 0, TCODConsole::root, 0, 0);

	gui->render(TCODConsole::root);
}
