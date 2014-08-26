#ifndef AI_HPP
#define AI_HPP

#include "libtcod.hpp"

class Actor;
class Engine;
class Map;

class Ai
{
public:
	virtual void update(Actor* owner, Engine* engine, TCOD_key_t key) = 0;
};

class PlayerAi : public Ai
{
public:
	void update(Actor* owner, Engine* engine, TCOD_key_t key);
};

class MeleeAi : public Ai
{
protected:
	void scheduleMove(Actor* owner, Engine* engine, int d_x, int d_y);
	void scheduleIdle(Actor* owner, Engine* engine);
public:
	void update(Actor* owner, Engine* engine, TCOD_key_t key);
};

#endif