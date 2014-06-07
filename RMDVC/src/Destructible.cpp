/*
 * Destructible.cpp
 *
 *  Created on: 19 May 2013
 *      Author: Clemens
 */

#include "Destructible.hpp"

Destructible::Destructible(float hp):
	hp(hp),body(NULL){
	cur_hp = hp;
}

Destructible::~Destructible(){
	delete body;
}


