#include "Map.hpp"
#include "Actor.hpp"
#include "Ai.hpp"

Map::Map(int width, int height) : width(width),height(height) {
    tiles=new Tile[width*height];
    setWall(30,22);
    setWall(50,22);

	tmap = new TCODMap(width, height);
	tmap->clear(true, true);
	tmap->setProperties(30, 22, false, false);
	tmap->setProperties(50, 22, false, false);
}

Map::~Map() {
	delete tmap;
    delete [] tiles;
}

bool Map::isWall(int x, int y) const {
    return !tiles[x+y*width].canWalk;
}
 
void Map::setWall(int x, int y) {
    tiles[x+y*width].canWalk=false;
}

void Map::render(TCODConsole* con) const {
    static const TCODColor darkWall(0,0,100);
    static const TCODColor darkGround(50,50,150);

	for (int x=0; x < width; x++) {
	    for (int y=0; y < height; y++) {
	        con->setCharBackground( x,y,
	            isWall(x,y) ? darkWall : darkGround );
	    }
	}
}

ActorMap::~ActorMap()
{
	delete actor_pos;
	//All references to any Actor* are invalid after destroying ActorMap !!!
	delete actors;
}

void ActorMap::addActor(Actor* actor)
{
	actors->insert(std::make_pair(actor->getUUID(),actor));
	actor_pos->insert(std::make_pair(actor->getUUID(), new Vector2(actor->getPosX(), actor->getPosY())));
}

Actor* ActorMap::getActorByUUID(std::string uuid)
{
	return actors->at(uuid);
}

void ActorMap::moveActor(std::string uuid, int pos_x, int pos_y)
{
	Actor* actor = getActorByUUID(uuid);
	//assert(actor != nullptr);
	
	actor->setPosX(pos_x);
	actor->setPosY(pos_y);

	updateActorPosition(uuid, pos_x, pos_y);
}

void ActorMap::updateActorPosition(std::string uuid, int pos_x, int pos_y)
{
	Vector2* pos = actor_pos->at(uuid);
	pos->pos_x = pos_x;
	pos->pos_y = pos_y;
}

std::string ActorMap::isOccupied(int pos_x, int pos_y)
{
	for (auto it = actor_pos->begin(); it != actor_pos->end(); it++)
	{
		if (it->second->pos_x == pos_x && it->second->pos_y == pos_y)
			return it->first;
	}

	return "";
}

void ActorMap::updateActor(std::string uuid, Engine* eng, TCOD_key_t key)
{
	Actor* actor = getActorByUUID(uuid);
	if (actor->ai != nullptr)
		actor->ai->update(actor, eng, key);
}

void ActorMap::render(TCODConsole* con)
{
	for (auto it = actors->begin();	it != actors->end(); it++) {
		it->second->render(con);
	}
}
