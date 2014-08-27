#ifndef MAP_HPP
#define MAP_HPP

#include "libtcod.hpp"
class Engine;
class Actor;

#include <map>
#include <string>
#include <boost/serialization/access.hpp>
#include <boost/serialization/split_member.hpp>

/** @brief A struct representing a two-dimensional vector.
*/
struct Vector2 {
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(pos_x);
		ar & BOOST_SERIALIZATION_NVP(pos_y);
	}

public:
	int pos_x;
	int pos_y;

	Vector2(int x, int y) : pos_x(x), pos_y(y) {};
	Vector2(){};
};

/** @brief A stuct encapsulating map tile properties.
*/
struct Tile {
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(canWalk);
	}

public:
    bool canWalk; // can we walk through this tile?
    Tile() : canWalk(true) {}
};
 
/** @brief A class encapsulating functions and members representing a map. 
*/
class Map {
private:
	
	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		// invoke serialization of the base class 
		ar << BOOST_SERIALIZATION_NVP(width);
		ar << BOOST_SERIALIZATION_NVP(height);
	}

	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		// invoke serialization of the base class 
		ar >> BOOST_SERIALIZATION_NVP(width);
		ar >> BOOST_SERIALIZATION_NVP(height);
		
		tiles = new Tile[width*height];
		setWall(30, 22);
		setWall(50, 22);

		tmap = new TCODMap(width, height);
		tmap->clear(true, true);
		tmap->setProperties(30, 22, false, false);
		tmap->setProperties(50, 22, false, false);
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER();

protected:
	Tile* tiles;

	void setWall(int x, int y);

public:
    int width,height;
	TCODMap* tmap;

    bool isWall(int x, int y) const;

 	void render(TCODConsole* con) const;

	Map(int width, int height);
	Map(){};
	~Map();
};

/** @brief A class holding pointers and positions to the actors currently loaded.
*/
class ActorMap {
private:
	std::map<std::string, Actor*>* actors;
	std::map<std::string, Vector2*>* actor_pos;

	void updateActorPosition(std::string uuid, int pos_x, int pos_y);

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(actors);
		ar & BOOST_SERIALIZATION_NVP(actor_pos);
	}
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

	ActorMap() : actors(new std::map<std::string, Actor*>()), actor_pos(new std::map<std::string, Vector2*>) {};
	~ActorMap();
};

#endif
