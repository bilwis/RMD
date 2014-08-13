#ifndef ACTOR_HPP
#define ACTOR_HPP

#include "libtcod.hpp"
#include "Destructible.hpp"

class Actor : public Object {
public :
    int x,y; // position on map
    int ch; // ascii code
    TCODColor col; // color

    Destructible *destructible;
 
    Actor(int x, int y, int ch, const TCODColor &col);
    ~Actor();
    void render(TCODConsole* con) const;
};

#endif
