/*
 * Destructible.cpp
 *
 *  Created on: 19 May 2013
 *      Author: Clemens
 */

#include "Destructible.hpp"

Destructible::Destructible(){

}

Destructible::~Destructible(){
	delete body;
}


