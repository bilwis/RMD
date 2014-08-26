#include "Action.hpp"
#include "Map.hpp"
#include "Actor.hpp"

void ActionScheduler::scheduleAction(Action* action, bool playerAction)
{
	assert(action != nullptr);
	bool inserted = false;

	double time_to_exec = calculateTimeToExec(action->getActorSpeed(), action->getCost());
	double prev_time = 0.0;

	ActionQueueEntry* td = new ActionQueueEntry(action, time_to_exec);

	if (queue->size() == 0)
	{
		queue->push_front(td);
		inserted = true;
	}
	else {
		for (auto it = queue->begin(); it != queue->end(); it++)
		{
			if ((*it)->time_to_exec > time_to_exec && prev_time <= time_to_exec)
			{
				queue->insert(it, td);
				inserted = true;
			}
			prev_time = (*it)->time_to_exec;
		}
		//If not inserted, it must be added at the end
		if (!inserted)
		{
			queue->push_back(td);
			inserted = true;
		}
	}

	if (inserted = true && playerAction)
		nextPlayerAction = td;
}

Action* ActionScheduler::nextAction()
{
	if (queue->empty()) { return nullptr; }

	ActionQueueEntry* ent = queue->front();
	Action* action = ent->action;
	
	//Need to deep copy value from ActionQueueEntry, otherwise it would always reduce by
	// ent->tte - ent->tte, because this is first in queue.
	double reduction = ent->time_to_exec;

	//Reduce time_to_exec for all elements in queue by the
	// time_to_exec of the next action.
	for (auto it = queue->begin(); it != queue->end(); it++)
	{
		(*it)->time_to_exec -= reduction;
	}

	if (ent == nextPlayerAction)
		nextPlayerAction = nullptr;

	queue->pop_front();
	return action;
}

const ActionResult* MoveAction::execute()
{
	if (map->isWall(actor->getPosX() + d_x, actor->getPosY() + d_y))
	{
		return new const ActionResult(actor->getUUID(), false);
	}

	std::string occ_actor = actor_map->isOccupied(actor->getPosX() + d_x, actor->getPosY() + d_y);
	if (occ_actor != "")
	{
		//TODO: return interact/attack Action
		return new const ActionResult(actor->getUUID(), false);
	}

	actor_map->moveActor(actor->getUUID(), actor->getPosX() + d_x, actor->getPosY() + d_y);

	return new const ActionResult(actor->getUUID(), true);
}

const int Action::getActorSpeed()
{
	return actor->getSpeed();
}

IdleAction::IdleAction(Actor* actor) : Action(actor, ACTION_IDLE), cost(actor->getSpeed())
{

}

const ActionResult* IdleAction::execute()
{
	return new const ActionResult(actor->getUUID(), true);
}