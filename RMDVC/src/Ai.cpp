#include "Ai.hpp"
#include "Action.hpp"
#include "Engine.hpp"
#include "Map.hpp"
#include "Actor.hpp"

void PlayerAi::update(Actor* owner, Engine* engine, TCOD_key_t key)
{
	Action* action = nullptr;

	switch (key.vk) {
	case TCODK_UP:
		action = new MoveAction(owner, engine->map, engine->actors, 0, -1);
		break;
	case TCODK_DOWN:
		action = new MoveAction(owner, engine->map, engine->actors, 0, 1);
		break;
	case TCODK_LEFT:
		action = new MoveAction(owner, engine->map, engine->actors, -1, 0);
		break;
	case TCODK_RIGHT:
		action = new MoveAction(owner, engine->map, engine->actors, 1, 0);
		break;
	default: //No applicable key pressed, exit function
		return;
	}

	if (action != nullptr) {
		engine->scheduler->scheduleAction(action, true);
	}

}

void MeleeAi::update(Actor* owner, Engine* engine, TCOD_key_t key)
{
	engine->map->tmap->computeFov(owner->getPosX(), owner->getPosY(), 10, true, FOV_BASIC);
	if (engine->map->tmap->isInFov(engine->player->getPosX(), engine->player->getPosY()))
	{
		TCODPath* path = new TCODPath(engine->map->tmap);
		path->compute(owner->getPosX(), owner->getPosY(), engine->player->getPosX(), engine->player->getPosY());
		int x, y;
		path->walk(&x, &y, true);

		scheduleMove(owner, engine, x - owner->getPosX(), y- owner->getPosY());
	}
	else
	{
		scheduleIdle(owner, engine);
	}
}

void MeleeAi::scheduleIdle(Actor* owner, Engine* engine)
{
	engine->scheduler->scheduleAction(new IdleAction(owner));
}

void MeleeAi::scheduleMove(Actor* owner, Engine* engine, int d_x, int d_y)
{
	engine->scheduler->scheduleAction(new MoveAction(owner, engine->map, engine->actors, d_x, d_y));
}