#ifndef AI_HPP
#define AI_HPP

#include "libtcod.hpp"

#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>

class Actor;
class Engine;
class Map;

/** @brief Base class for all Ai modules.
*/
class Ai
{
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{ }

public:
	virtual void update(Actor* owner, Engine* engine, TCOD_key_t key) = 0;
};

/** @brief A class that enables parsing of keyboard inputs into actions of the player Actor.
*/
class PlayerAi : public Ai
{
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Ai);
	}
public:
	void update(Actor* owner, Engine* engine, TCOD_key_t key);
};

/** @brief A class representing a basic melee monster Ai.
*/
class MeleeAi : public Ai
{
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Ai);
	}

protected:
	void scheduleMove(Actor* owner, Engine* engine, int d_x, int d_y);
	void scheduleIdle(Actor* owner, Engine* engine);
public:
	void update(Actor* owner, Engine* engine, TCOD_key_t key);
};

#endif