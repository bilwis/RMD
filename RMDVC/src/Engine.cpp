#include "Engine.hpp"

#include <boost/serialization/export.hpp>

BOOST_CLASS_EXPORT_GUID(BodyPart, "BodyPart")
BOOST_CLASS_EXPORT_GUID(Organ, "Organ")

Engine::Engine() {
    TCODConsole::initRoot(120,80,"libtcod C++ tutorial",false);
	gameConsole = new TCODConsole(120, 70);

	TCODConsole::root->print(1, 1, "Press 'n' for new game, Press 'l' to load...");
	TCODConsole::root->flush();

	TCOD_key_t key;
	TCODSystem::waitForEvent(TCOD_EVENT_KEY_PRESS, &key, nullptr, true);

	actors = new std::vector<Actor*>();

	if (key.c == 'n')
	{
		player = new Actor(40, 25, '@', TCODColor::white, 100);

		player->destructible = new Destructible(100);
		player->destructible->body = new Body("Body.xml");
		
		actors->push_back(player);
	}

	if (key.c == 'l')
	{
		std::ifstream ifs("save.xml");
		boost::archive::xml_iarchive ia(ifs);
		ia & BOOST_SERIALIZATION_NVP(player);
		ifs.close();

		actors->push_back(player);
	}

	actors->push_back(new Actor(60, 13, '@', TCODColor::yellow, 100));
	map = new Map(120, 70);
	gui = new Gui();
	scheduler = new ActionScheduler();


	guiBodyViewer = new GuiBodyViewer("BodyViewer", 3, 3, 80, 40,
		TCODColor::white, TCODColor::black, true, "BodyViewer");

	guiBodyViewer->setVisibility(false);

	gui->addContainer(guiBodyViewer);

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

		Action* action = nullptr;
	
		switch(key.vk) {
			case TCODK_UP :
				action = new MoveAction(player, map, 0, -1);
			break;
			case TCODK_DOWN :
				action = new MoveAction(player, map, 0, 1);
			break;
			case TCODK_LEFT :
				action = new MoveAction(player, map, -1, 0);
			break;
			case TCODK_RIGHT :
				action = new MoveAction(player, map, 1, 0);
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
					break;
					case 's':
						debug_print("Saving...");
						std::ofstream ofs("save.xml");
						boost::archive::xml_oarchive oa(ofs);
						oa << BOOST_SERIALIZATION_NVP(player);
						ofs.close();
						debug_print("done.\n");
        		}
        		break;
			default:break;
		}

		if (action != nullptr) {
			scheduler->scheduleAction(action);

			//Perform actions until players turn
			Action* nextAction = nullptr;
			do
			{
				//If an action was performed, delete it!
				if (nextAction != nullptr)
					delete nextAction;

				nextAction = scheduler->nextAction();
				assert(nextAction != nullptr); //If queue is empty, fail (queue must not be empty while state == GAME)

				//TODO: Add alternative action handling
				const ActionResult* res = nextAction->execute();

				debug_print("Performed %s, result: %s \n", 
					ActionTypeNames[nextAction->getActionType()], 
					res->wasSuccessful() ? "true" : "false");

			} while (nextAction != action);

			delete nextAction; // also deletes action if loop ended successfully 
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
