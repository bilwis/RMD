#ifndef OBJECT_HPP
#define OBJECT_HPP

#include "libtcod.hpp"
#include <string>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

using std::string;
using boost::uuids::uuid;

/** 
* @brief The base class for all objects in the game.
*/
class Object
{
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

	void setForeColor(TCODColor color){ foreground_color = color; }
	void setBackColor(TCODColor color){ background_color = color; }

	void setCharacter(wchar_t ch) { this->ch = ch; }

	virtual void render(TCODConsole* con) = 0;

	~RenderObject();
};

#endif