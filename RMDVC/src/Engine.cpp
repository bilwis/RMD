#include "Engine.hpp"
#include <stdio.h>

Engine::Engine() {
    TCODConsole::initRoot(120,80,"libtcod C++ tutorial",false);
	gameConsole = new TCODConsole(120, 70);

    player = new Actor(40,25,'@',TCODColor::white);

    player->destructible = new Destructible(100);
    player->destructible->body = new Body("Body.xml");

	actors = new std::vector<Actor*>();

    actors->push_back(player);
    actors->push_back(new Actor(60,13,'@',TCODColor::yellow));
    map = new Map(120,70);

	gui = new Gui();
	
	
	guiBodyViewer = new GuiBodyViewer("BodyViewer", 3, 3, 80, 40,
		TCODColor::white, TCODColor::black, true, "BodyViewer");

	guiBodyViewer->setVisibility(false);

	gui->addContainer(guiBodyViewer);
	

	/*
	sampleContainer = new GuiContainer("sampleContainer", 10, 10, 40, 20,
		TCODColor::yellow, TCODColor::darkBlue, true, "TestBox");

	sampleTextBox = new GuiTextBox("sampleTextBox", 1, 1, 38, 10,
		"This is a test string to check the functionality of this TextBox",
		TCODColor::red, TCODColor::green);
	sampleContainer->addElement(sampleTextBox);

	sampleList = new GuiListChooser("sampleList", 1, 11, 38, 8,
		TCODColor::white, TCODColor::black,
		TCODColor::darkYellow, TCODColor::blue,
		TCODColor::darkerYellow, TCODColor::desaturatedBlue);
	sampleList->addItem(std::shared_ptr<Object>(new Object("Item 1")),
		new ColoredText("Item 1 - Red", TCODColor::darkRed, TCODColor::turquoise));
	sampleList->addItem(std::shared_ptr<Object>(new Object("Item 2")),
		new ColoredText("Item 2 - Default"));
	sampleList->addItem(std::shared_ptr<Object>(new Object("Item 3")),
		new ColoredText("Item 3 - Yellow", TCODColor::yellow));
	sampleContainer->addElement(sampleList);
	
	gui->registerActiveElement(sampleList);
	gui->addContainer(sampleContainer);

	sampleContainer->setVisibility(false);*/

	state = GameState::GAME;
}

void createBasicUI(Gui gui)
{

}

Engine::~Engine() {
    actors->erase(actors->begin(), actors->end());
	delete actors;
    delete map;
}

void Engine::update() {
	TCOD_key_t key;
	TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS, &key, NULL);

	//No key pressed = nothing to do!
	if (key.vk == TCODK_NONE) { return; }

	if (state == GameState::GUI) {
		//On Escape, exit the GUI state
		// tell the Gui object to inactivate all ActiveGuiElements
		// and destroy the temporary ones (TBI)
		if (key.vk == TCODK_ESCAPE){
			state = GameState::GAME; 
			gui->exitGUIState();
		} else {
			gui->update(key);
		}
	}

	if (state == GameState::GAME) {
	
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
						//sampleTextBox->setText("OH GOD, WHY!?");
        			break;
					case 'l':
						state = GameState::GUI;
						guiBodyViewer->activate(player->destructible->body);
						gui->makeActive(guiBodyViewer->getUUID());
        		}
        		break;
			default:break;
		}
	
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
	for (std::vector<Actor*>::iterator it = actors->begin();
	    it != actors->end(); it++) {
	    (*it)->render(gameConsole);
	}

	TCODConsole::blit(gameConsole, 0, 0, 0, 0, TCODConsole::root, 0, 0);

	gui->render(TCODConsole::root);
}
