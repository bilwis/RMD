#ifndef ACTION_HPP
#define ACTION_HPP

#include "Object.hpp"

class Actor;
class Map;
class ActorMap;

#include <string>
#include <list>

class Action;
class ActionResult;

enum ActionType {
	ACTION_MOVE,
	ACTION_IDLE,
	SIZE_OF_ACTION_TYPE_ENUM
};

//Array length = enum length -> compile error if one is updated without the other!
static const char* ActionTypeNames[SIZE_OF_ACTION_TYPE_ENUM] = { "Move Action", "Idle Action" };

/** This struct holds a pointer to an action, and the time to the actions execution (as double).
* A (double-linked) list of these stucts is used in the ActionScheduler as the queue element.
*/
struct ActionQueueEntry
{
	Action* action;
	double time_to_exec;

	/**Creates a new ActionQueueEntry with the given action and time to execution.
	*/
	ActionQueueEntry(Action* action, double time_to_exec) : action(action), time_to_exec(time_to_exec) {};
	~ActionQueueEntry(){ };
};
 
/** This class is responsible for the correct execution order of actions that are scheduled
* for the Actors in the game. It holds a double-linked list of ActionQueueEntry, which acts
* as the FIFO queue of Actions.
* The "time to execution" (TtE) is calculated as follows is an arbitrary number used to sort
* the action queue. The action with the lowest TtE is executed next, and its TtE substracted from
* all other TtEs in the queue.
*/
class ActionScheduler
{
	std::list<ActionQueueEntry*>* queue;

	ActionQueueEntry* nextPlayerAction = nullptr;

	/** This function calculates the time to execution as follows: (Action cost) / (Actor speed).
	*
	* @param actor_speed The speed of the actor (must be > 0).
	* @param action_cost The cost of the action.
	* @return The time to execution.
	*/
	double calculateTimeToExec(int actor_speed, int action_cost)
	{
		assert(actor_speed > 0);
		return (double)action_cost / (double)actor_speed;
	};

public:
	/** This function enters an action into the Schedulers queue. The time to execution is
	* calculated from the action itself. The action will be inserted at the position in the queue
	* corresponding to its time to execution.
	*
	* @param action A pointer to the action to be scheduled.
	* @param playerAction Wheter the action was scheduled by the PlayerAi, and will cause a block
	*	after it was executed, waiting for the next input by the player.
	*/
	void scheduleAction(Action* action, bool playerAction = false);

	/** This function returns the next action in queue. The action is deleted from the queue.
	* _Important: The action instance must be destroyed by the caller after use, because it cannot be handled by
	* the Scheduler or the Action itself!_
	*
	* @return A pointer to the next action in queue or nullptr, if the queue is empty.
	*/
	Action* nextAction();

	bool isPlayerActionScheduled() {if (nextPlayerAction == nullptr) { return false; } return true; }

	ActionScheduler()
	{
		queue = new std::list<ActionQueueEntry*>();
	};
	~ActionScheduler()
	{
		delete queue;
	};
};

/** This class represents an Action that may be executed by an Actor.
* This is the abstract base class from which all action classes must be derived. 
* It holds the cost, type and a pointer to the executing actor.
*/
class Action : public Object
{
protected:

	const ActionType type;
	Actor* actor;
	
public:
	virtual const int getCost() = 0;
	const int getActorSpeed();
	const ActionType getActionType() { return type; }

	/** This abstract function must be implemented by all Derivates of Action.
	* It dictates what the Action actually _does_.
	*/
	virtual const ActionResult* execute() = 0;

	Action(Actor* actor, ActionType type) : actor(actor), type(type) {};
	virtual ~Action(){};
};

/** This class represents the result of a call to Action::execute(). It holds information
* about wheter the action was successfully executed and, if applicable, which action may be performed
* instead. (Example: MoveAction against a door fails -> execute UseAction with door instead [opening it])
*/
class ActionResult
{
	const bool success;
	const Action* alternative;
	const std::string actor_uuid;

public:
	const bool wasSuccessful() const { return success; };
	const Action* getAlternative() const { return alternative; };

	const std::string getActorUUID() const { return actor_uuid; }

	ActionResult(std::string actor_uuid, bool success, const Action* alternative = nullptr) 
		: actor_uuid(actor_uuid), success(success), alternative(alternative) {};
	~ActionResult(){};
};

/** This class represents an action that an actor may perform to move on the map.
* It holds a pointer to the map object (for obstacle checking) and two int objects
* representing the relative movement the actor shall perform (delta x and delta y).
*/
class MoveAction : public Action
{
	static const int cost = 100;	

	const Map* map;
	ActorMap* actor_map;

	int d_x;
	int d_y;

public:
	const int getCost() { return cost; }

	const ActionResult* execute();

	MoveAction(Actor* actor, const Map* map, ActorMap* actor_map, int d_x, int d_y) : Action(actor, ACTION_MOVE), map(map), actor_map(actor_map), d_x(d_x), d_y(d_y){};
	~MoveAction(){};
};

class IdleAction : public Action
{
	const int cost;
public:
	const int getCost() { return cost; }

	const ActionResult* execute();
	IdleAction(Actor* actor);
};

#endif