#include "Action.hpp"

void ActionScheduler::scheduleAction(Action* action)
{
	assert(action != nullptr);

	double time_to_exec = calculateTimeToExec(action->getActorSpeed(), action->getCost());
	double prev_time = 0.0;

	if (queue->size() == 0)
	{
		queue->push_front(new ActionQueueEntry(action, time_to_exec));
	}
	else {
		for (auto it = queue->begin(); it != queue->end(); it++)
		{
			if ((*it)->time_to_exec > time_to_exec && prev_time <= time_to_exec)
			{
				queue->insert(it, new ActionQueueEntry(action, time_to_exec));
			}
			prev_time = (*it)->time_to_exec;
		}
	}
}

Action* ActionScheduler::nextAction()
{
	if (queue->empty()) { return nullptr; }

	ActionQueueEntry* ent = queue->front();
	Action* action = ent->action;
	
	//Reduce time_to_exec for all elements in queue by the
	// time_to_exec of the next action.
	for (auto it = queue->begin(); it != queue->end(); it++)
	{
		(*it)->time_to_exec -= ent->time_to_exec;
	}

	queue->pop_front();
	return action;
}

const ActionResult* MoveAction::execute()
{
	if (map->isWall(actor->getPosX() + d_x, actor->getPosY() + d_y))
	{
		return new const ActionResult(false);
	}

	actor->moveByX(d_x);
	actor->moveByY(d_y);

	return new const ActionResult(true);
}