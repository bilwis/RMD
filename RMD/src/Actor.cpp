#include "Actor.hpp"
 
Actor::Actor(int x, int y, int ch, const TCODColor &col) :
    x(x),y(y),ch(ch),col(col),destructible(NULL) {
}

Actor::~Actor()
{
	delete destructible;
}
 
void Actor::render() const {
    TCODConsole::root->setChar(x,y,ch);
    TCODConsole::root->setCharForeground(x,y,col);
}
