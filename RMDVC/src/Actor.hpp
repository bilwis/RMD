#ifndef ACTOR_HPP
#define ACTOR_HPP

#include "libtcod.hpp"
#include "Object.hpp"

class Ai;
class Destructible;

#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/archive/xml_oarchive.hpp> // saving
#include <boost/archive/xml_iarchive.hpp> // loading

/** It may be expanded by adding pointers to 'modules', such as Destructible, 
* which makes it an entity that can be hurt and destroyed; or Ai, which makes it act 
* by specified routines.
*
* @brief A class representing an entity that may perform actions.
*/
class Actor : public RenderObject {
private:
	int speed;

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(RenderObject);
		ar & BOOST_SERIALIZATION_NVP(destructible);
		ar & BOOST_SERIALIZATION_NVP(ai);
		ar & BOOST_SERIALIZATION_NVP(speed);
	}

public :
    Destructible* destructible;
	Ai* ai;

	const int getSpeed() { return speed; }
 
    Actor(int x, int y, int ch, const TCODColor &col, int speed);
	Actor(){};
    ~Actor();

    void render(TCODConsole* con);
};

#endif
