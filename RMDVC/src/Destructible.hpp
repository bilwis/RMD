/*
 * Destructible.hpp
 *
 *  Created on: 19 May 2013
 *      Author: Clemens
 */

#ifndef DESTRUCTIBLE_HPP_
#define DESTRUCTIBLE_HPP_

#include <boost/serialization/access.hpp>
#include <boost/archive/xml_oarchive.hpp> // saving
#include <boost/archive/xml_iarchive.hpp> // loading

class Body;

class Destructible{

private:
	float hp, cur_hp;

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(hp);
		ar & BOOST_SERIALIZATION_NVP(cur_hp);
		ar & BOOST_SERIALIZATION_NVP(body);
	}

public:
	Destructible(float hp);
	Destructible(){};
	~Destructible();

	Body *body;

	float damage(float amount);

};


#endif /* DESTRUCTIBLE_HPP_ */
