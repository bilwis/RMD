#include "Actor.hpp"
 
Actor::Actor(int x, int y, int ch, const TCODColor &col, int speed) :
   RenderObject(x,y,col, TCODColor::black, ch), speed(speed), destructible(NULL) {
}

Actor::~Actor()
{
	delete destructible;
}
 
void Actor::render(TCODConsole* con) {
    con->setChar(pos_x,pos_y,ch);
    con->setCharForeground(pos_x,pos_y,foreground_color);
}
