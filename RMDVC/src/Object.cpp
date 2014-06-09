#include "Object.hpp"

Object::Object(string id)
{ 
	this->id = id;
}

Object::~Object()
{

}

RenderObject::RenderObject(string id, int x, int y,  TCODColor fore, TCODColor back):
	Object(id){
	pos_x = x;
	pos_y = y;
	foreground_color = fore;
	background_color = back;
}

RenderObject::~RenderObject(){}