#include "Actor.hpp"
 
Actor::Actor(int x, int y, int ch, const TCODColor &col) :
    x(x),y(y),ch(ch),col(col),destructible(NULL) {
}

Actor::~Actor()
{
	delete destructible;
}
 
void Actor::render(TCODConsole* con) const {
    con->setChar(x,y,ch);
    con->setCharForeground(x,y,col);
}
