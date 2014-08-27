#ifndef OBJECT_HPP
#define OBJECT_HPP

#include "libtcod.hpp"
#include <string>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_serialize.hpp>

#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/archive/xml_oarchive.hpp> // saving
#include <boost/archive/xml_iarchive.hpp> // loading

using std::string;
using boost::uuids::uuid;

/** 
* @brief Base class for all objects in the game.
*/
class Object
{
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(id);
	}

protected:
	uuid id;

public:
	string getUUID(){ return boost::uuids::to_string(id); }

	virtual string toString() { return getUUID(); }

	Object() : id(boost::uuids::random_generator()()) {};
	~Object() {};
};

/** It holds basic drawing information (color, char and position).
* It also includes a rendering function that should be overwritten
* by specific implementations by the inheriting classes.
*
* @brief A class representing an object that may be rendered on screen.
*/
class RenderObject : public Object
{
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Object);
		
		ar & BOOST_SERIALIZATION_NVP(foreground_color.r);
		ar & BOOST_SERIALIZATION_NVP(foreground_color.g);
		ar & BOOST_SERIALIZATION_NVP(foreground_color.b);

		ar & BOOST_SERIALIZATION_NVP(background_color.r);
		ar & BOOST_SERIALIZATION_NVP(background_color.g);
		ar & BOOST_SERIALIZATION_NVP(background_color.b);

		ar & BOOST_SERIALIZATION_NVP(pos_x);
		ar & BOOST_SERIALIZATION_NVP(pos_y);

		ar & BOOST_SERIALIZATION_NVP(ch);
	}

protected:
	TCODColor background_color;
	TCODColor foreground_color;

	int pos_x;
	int pos_y;

	wchar_t ch = 20; //Defaults to space
	
	RenderObject(int x, int y, TCODColor fore, TCODColor back, wchar_t ch = 20);

public:
	int getPosX(){ return pos_x; }
	int getPosY(){ return pos_y; }

	TCODColor getForeColor() { return foreground_color; }
	TCODColor getBackColor() { return background_color; }

	wchar_t getCharacter() { return ch; }

	void setPosX(int x){ pos_x = x; }
	void setPosY(int y){ pos_y = y; }

	void moveByX(int delta) { pos_x += delta; }
	void moveByY(int delta) { pos_y += delta; }

	void setForeColor(TCODColor color){ foreground_color = color; }
	void setBackColor(TCODColor color){ background_color = color; }

	void setCharacter(wchar_t ch) { this->ch = ch; }

	virtual void render(TCODConsole* con) = 0;

	RenderObject(){};
	~RenderObject();
};

#endif