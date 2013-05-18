#ifndef ACTOR_HPP
#define ACTOR_HPP

#include "libtcod.hpp"

class Actor {
public :
    int x,y; // position on map
    int ch; // ascii code
    TCODColor col; // color
 
    Actor(int x, int y, int ch, const TCODColor &col);
    void render() const;
};

#endif
