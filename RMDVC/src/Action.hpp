#ifndef ACTION_HPP
#define ACTION_HPP

#include "Object.hpp"

class Actor;
class Map;
class ActorMap;

#include <string>
#include <list>
#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/list.hpp>

class Action;
class ActionResult;

enum ActionType {
	ACTION_MOVE,
	ACTION_IDLE,
	ACTION_NULL,
	SIZE_OF_ACTION_TYPE_ENUM
};

//Array length = enum length -> compile error if one is updated without the other!
static const char* ActionTypeNames[SIZE_OF_ACTION_TYPE_ENUM] = { "Move Action", "Idle Action", "Null Action" };

/** 
* A (double-linked) list of these stucts is used in the ActionScheduler as the queue element.
*
* @brief A struct holding a pointer to an action, and the time to the actions execution (as double).
*/
struct ActionQueueEntry
{
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(action);
		ar & BOOST_SERIALIZATION_NVP(time_to_exec);
	}

public:
	Action* action;
	double time_to_exec;

	/**Creates a new ActionQueueEntry with the given action and time to execution.
	*/
	ActionQueueEntry(Action* action, double time_to_exec) : action(action), time_to_exec(time_to_exec) {};
	ActionQueueEntry() {};
	~ActionQueueEntry() {};
};
 
/** It holds a double-linked list of ActionQueueEntry, which acts
* as the FIFO queue of Actions.
* The "time to execution" (TtE) is calculated as follows is an arbitrary number used to sort
* the action queue. The action with the lowest TtE is executed next, and its TtE substracted from
* all other TtEs in the queue.
* See the [action system reference](action_help.html) for more.
*
* @brief A class responsible for the correct execution order of actions scheduled for Actors.
*/
class ActionScheduler
{
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(queue);
		ar & BOOST_SERIALIZATION_NVP(nextPlayerAction);
	}

	std::list<ActionQueueEntry*>* queue;

	/**When a action is scheduled with scheduleAction(Action, playerAction = true), this pointer
	* is set to the ActionQueueEntry that is added to the queue. When it is dequeued via nextAction(),
	* the Scheduler sets nextPlayerAction to the nullptr. This allows the game loop to idle until
	* the next player action is queued.
	*/
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

	/** This function returns true if an action scheduled by the PlayerAi is in the action queue.
	* If there is none, it returns false. This allows the game loop to idle when there is no player 
	* action scheduled.
	*/
	bool isPlayerActionScheduled() {if (nextPlayerAction == nullptr) { return false; } return true; }

	ActionScheduler() : queue(new std::list<ActionQueueEntry*>()) {};
	~ActionScheduler()
	{
		delete queue;
	};
};

/** 
* This is the abstract base class from which all action classes must be derived. 
* It holds the cost, type and a pointer to the executing actor.
* See the [action system reference](action_help.html) for more.
*
* @brief A class representing an Action that may be executed by an Actor.
*/
class Action : public Object
{
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Object);
		ar & BOOST_SERIALIZATION_NVP(type);
		ar & BOOST_SERIALIZATION_NVP(actor);
	}

protected:
	ActionType type;
	Actor* actor;
	
public:
	/**This function returns the cost of the action.
	*/
	virtual const int getCost() = 0;
	const int getActorSpeed();
	const ActionType getActionType() { return type; }

	/** This abstract function must be implemented by all Derivates of Action.
	* It dictates what the Action actually _does_.
	*/
	virtual const ActionResult* execute() = 0;

	Action(Actor* actor, ActionType type) : actor(actor), type(type) {};
	Action():type(ACTION_NULL){};
	virtual ~Action(){};
};

/** It holds information
* about wheter the action was successfully executed and, if applicable, which action may be performed
* instead. (Example: MoveAction against a door fails -> execute UseAction with door instead [opening it])
* 
* @brief A class encapsulating the result of a call to Action::execute(). 
*/
class ActionResult
{
private:
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

/**
* It holds a pointer to the map object (for obstacle checking) and two int objects
* representing the relative movement the actor shall perform (delta x and delta y).
* 
* @brief A class representing an action that an actor may perform to move on the map.
*/
class MoveAction : public Action
{
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Action);
		ar & BOOST_SERIALIZATION_NVP(map);
		ar & BOOST_SERIALIZATION_NVP(actor_map);
		ar & BOOST_SERIALIZATION_NVP(d_x);
		ar & BOOST_SERIALIZATION_NVP(d_y);
	}
	static const int cost = 100;	

	Map* map;
	ActorMap* actor_map;

	int d_x;
	int d_y;

public:
	//TODO: Add dynamic cost depending on terrain etc.
	const int getCost() { return cost; }

	const ActionResult* execute();

	MoveAction(Actor* actor, Map* map, ActorMap* actor_map, int d_x, int d_y) : Action(actor, ACTION_MOVE), map(map), actor_map(actor_map), d_x(d_x), d_y(d_y){};
	MoveAction(){};
	~MoveAction(){};
};

/** Its cost is set to be equal to the actors speed.
* This action is necessary to hold a place in the action queue for the actor.
* See the [action system reference](action_help.html) for more.
*
* @brief A class representing an action that an actor may perform to 'pass' its turn.
*/
class IdleAction : public Action
{
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Action);
		ar & BOOST_SERIALIZATION_NVP(cost);
	}

	int cost;
public:
	const int getCost() { return cost; }

	const ActionResult* execute();
	IdleAction(Actor* actor);
	IdleAction():cost(0){};
};

#endif