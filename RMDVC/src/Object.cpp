#include "Object.hpp"


RenderObject::RenderObject(int x, int y,  TCODColor fore, TCODColor back, wchar_t ch):
	Object(){
	pos_x = x;
	pos_y = y;
	foreground_color = fore;
	background_color = back;
	this->ch = ch;
}

RenderObject::~RenderObject(){}