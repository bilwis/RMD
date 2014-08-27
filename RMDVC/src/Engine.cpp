#include "Engine.hpp"
#include "Map.hpp"
#include "Actor.hpp"
#include "Action.hpp"
#include "GUI.hpp"
#include "Body.hpp"
#include "Destructible.hpp"
#include "Ai.hpp"

#include <boost/serialization/export.hpp>

BOOST_CLASS_EXPORT_GUID(BodyPart, "BodyPart")
BOOST_CLASS_EXPORT_GUID(Organ, "Organ")
BOOST_CLASS_EXPORT_GUID(PlayerAi, "PlayerAi")
BOOST_CLASS_EXPORT_GUID(MeleeAi, "MeleeAi")
BOOST_CLASS_EXPORT_GUID(MoveAction, "MoveAction")
BOOST_CLASS_EXPORT_GUID(IdleAction, "IdleAction")

Engine::Engine() {
    TCODConsole::initRoot(120,80,"libtcod C++ tutorial",false);
	gameConsole = new TCODConsole(120, 70);

	TCODConsole::root->print(1, 1, "Press 'n' for new game, Press 'l' to load...");
	TCODConsole::root->flush();

	TCOD_key_t key;
	TCODSystem::waitForEvent(TCOD_EVENT_KEY_PRESS, &key, nullptr, true);

	gui = new Gui();


	if (key.c == 'n')
	{
		actors = new ActorMap();
		map = new Map(120, 70);
		scheduler = new ActionScheduler();

		player = new Actor(40, 25, '@', TCODColor::white, 200);

		player->destructible = new Destructible(100);
		player->destructible->body = new Body("Body.xml");
		player->ai = new PlayerAi();

		actors->addActor(player);

		Actor* mob = new Actor(60, 13, '@', TCODColor::yellow, 100);
		mob->ai = new MeleeAi();
		mob->ai->update(mob, this, TCODConsole::checkForKeypress());

		actors->addActor(mob);
	}

	if (key.c == 'l')
	{
		std::ifstream ifs("save.xml");
		boost::archive::xml_iarchive ia(ifs);
		ia & BOOST_SERIALIZATION_NVP(actors);
		ia & BOOST_SERIALIZATION_NVP(map);
		ia & BOOST_SERIALIZATION_NVP(scheduler);
		ia & BOOST_SERIALIZATION_NVP(player);
		ifs.close();

		actors->addActor(player);
	}

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
	delete actors;
    delete map;
}

void Engine::update() {
	TCOD_key_t key;
	TCODSystem::waitForEvent(TCOD_EVENT_KEY_PRESS, &key,NULL, NULL);

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
						oa << BOOST_SERIALIZATION_NVP(actors);
						oa << BOOST_SERIALIZATION_NVP(map);
						oa << BOOST_SERIALIZATION_NVP(scheduler);
						oa << BOOST_SERIALIZATION_NVP(player);
						
						ofs.close();
						debug_print("done.\n");
        		}
        		break;
			default: //Any key that has not been overridden by the above
				
				break;
		}
		
		//Try to update player (if no applicable key is pressed, no action will be scheduled,
		// and action loop is not entered.
		actors->updateActor(player->getUUID(), this, key);

		//Perform actions until players turn

		Action* nextAction = nullptr;
		while (scheduler->isPlayerActionScheduled())
		{
			nextAction = scheduler->nextAction();
			assert(nextAction != nullptr); //If queue is empty, fail (queue must not be empty while state == GAME)

			//TODO: Add alternative action handling
			const ActionResult* res = nextAction->execute();

			//Call the Ai of the actor who just acted (and let it schedule a new action),
			// unless it is the player, whose update is handled in the main update loop.
			//key variable is ignored unless used for debug purposes.
			if (res->getActorUUID() != player->getUUID())
				actors->updateActor(res->getActorUUID(), this, key);

			debug_print("Performed %s for Actor UUID %s, result: %s \n",
				ActionTypeNames[nextAction->getActionType()],
				res->getActorUUID().c_str(),
				res->wasSuccessful() ? "true" : "false");

			delete nextAction;
		};
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
	actors->render(gameConsole);

	TCODConsole::blit(gameConsole, 0, 0, 0, 0, TCODConsole::root, 0, 0);

	gui->render(TCODConsole::root);
}
