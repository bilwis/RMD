#ifndef ACTOR_HPP
#define ACTOR_HPP

#include "libtcod.hpp"
#include "Destructible.hpp"
#include "Object.hpp"

#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/archive/xml_oarchive.hpp> // saving
#include <boost/archive/xml_iarchive.hpp> // loading

class Actor : public RenderObject {
private:

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(RenderObject);
		ar & BOOST_SERIALIZATION_NVP(destructible);
	}

public :
    Destructible *destructible;
 
    Actor(int x, int y, int ch, const TCODColor &col);
	Actor(){};
    ~Actor();

    void render(TCODConsole* con);
};

#endif
