#ifndef MAP_HPP
#define MAP_HPP

#include "libtcod.hpp"

class Engine;
class Actor;

#include <map>
#include <string>

struct Vector2 {
	int pos_x;
	int pos_y;

	Vector2(int x, int y) : pos_x(x), pos_y(y) {};
};

struct Tile {
    bool canWalk; // can we walk through this tile?
    Tile() : canWalk(true) {}
};
 
class Map {
protected:
	Tile* tiles;

	void setWall(int x, int y);

public:
    int width,height;

	TCODMap* tmap;
 
    Map(int width, int height);
    ~Map();

    bool isWall(int x, int y) const;

 	void render(TCODConsole* con) const;
};

class ActorMap {
private:
	std::map<std::string, Actor*>* actors;
	std::map<std::string, Vector2*>* actor_pos;

	void updateActorPosition(std::string uuid, int pos_x, int pos_y);
public:
	void addActor(Actor* actor);
	void removeActor(std::string uuid);

	//This function assumes map and actor map have been tested for collisions
	void moveActor(std::string uuid, int pos_x, int pos_y);

	bool isActorRegistered(std::string uuid);

	Actor* getActorByUUID(std::string uuid);
	const Actor* getActorConstByUUID(std::string uuid);

	std::string isOccupied(int pos_x, int pos_y);

	void updateActor(std::string uuid, Engine* eng, TCOD_key_t key);
	void render(TCODConsole* con);

	ActorMap();
	~ActorMap();
};

#endif
