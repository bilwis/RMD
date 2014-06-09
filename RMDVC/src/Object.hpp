#ifndef OBJECT_HPP
#define OBJECT_HPP

#include "libtcod.hpp"
#include <string>

using std::string;

class Object
{
protected:
	string id;
public:
	string getId(){ return id; }

	Object(string id);
	~Object();
};

class RenderObject : public Object
{
protected:
	TCODColor background_color;
	TCODColor foreground_color;

	int pos_x;
	int pos_y;

	RenderObject(string id, int x, int y, TCODColor fore, TCODColor back);

public:
	int getPosX(){ return pos_x; }
	int getPosY(){ return pos_y; }

	TCODColor getForeColor() { return foreground_color; }
	TCODColor getBackColor() { return background_color; }

	void setPosX(int x){ pos_x = x; }
	void setPosY(int y){ pos_y = y; }

	void setForeColor(TCODColor color){ foreground_color = color; }
	void setBackColor(TCODColor color){ background_color = color; }

	~RenderObject();
};

class Entity : public RenderObject
{
protected:
	wchar_t ch;

public:
	virtual void render(TCODConsole* con);

};

#endif