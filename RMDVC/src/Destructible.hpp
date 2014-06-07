/*
 * Destructible.hpp
 *
 *  Created on: 19 May 2013
 *      Author: Clemens
 */

#ifndef DESTRUCTIBLE_HPP_
#define DESTRUCTIBLE_HPP_

#include "Body.hpp"

class Destructible{

private:
	float hp, cur_hp;

public:
	Destructible(float hp);
	~Destructible();

	Body *body;

	float damage(float amount);

};


#endif /* DESTRUCTIBLE_HPP_ */
