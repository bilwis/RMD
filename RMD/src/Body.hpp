/*
 * Body.hpp
 *
 *  Created on: 19 May 2013
 *      Author: Clemens
 */

#ifndef BODY_HPP_
#define BODY_HPP_

#include "rapidxml.hpp"
#include "libtcod.hpp"
#include <iostream>
#include <fstream>
#include <map>

/** This class holds the definition of a tissue, i.e. the 'leaves' of the body 'tree'.
 * Tissue definitions are generalized, not localized, so a 'muscle' tissue in the 'right leg'
 * body part is not distinct from a 'muscle' tissue in the head, etc.
 * Exceptions to this must be added to the tissue_def declaration in the [body-definition XML](xml_help.html).
 *
 * @brief Holds the definition of a tissue, such as skin or bone.
 */
class Tissue{
private:
	const char *id;
	const char *name;
	float pain;
	float blood_flow;
	float resistance;
	float impairment;

public:
	float getBloodFlow() const {
		return blood_flow;
	}

	const char* getName() const {
		return name;
	}

	/** Returns the internal id of the tissue (distinct from its name) so special functions
	 * may be assigned to certain tissues and so that they may be referenced from code.
	 *
	 *     if (tissue_hit.getId() == "NERVE"){
	 *      print("You are wracked with pain!");
	 *     }
	 *
	 * The internal id is also used in the XML file itself, for the organ definitions.
	 * @return Internal tissue identifier.
	 */
	const char* getId() const {
		return id;
	}

	float getPain() const {
		return pain;
	}

	float getResistance() const {
		return resistance;
	}

	float getImpairment() const {
		return impairment;
	}

	/** This function creates a new tissue definition.
	 *  It should only be called by the Body constructor parsing the
	 *  body XML file!
	 *
	 * @brief Creates a new tissue definition.
	 *
	 * @param id Internal tissue identifier
	 * @param name Common name for the tissue
	 * @param pain The pain damage to this tissue causes
	 * @param blood_flow The blood loss damage to this tissue causes
	 * @param resistance How much this tissue absorbs energy
	 * @param impairment How much damage to this tissue impairs the Actor
	 */
	Tissue(const char *id, const char *name, float pain,
			float blood_flow, float resistance, float impairment);
	~Tissue();
};

/** An Organ consists of many tissues, all of which are predefined in the
 * [body-definition XML](xml_help.html). However, each organ contains the tissue in
 * different quantities (and qualities!), which is represented by this struct.
 * It holds the pointer to the default tissue, a hit probability modifier representing
 * the quantity of the tissue within the organ, and, optionally, a custom name and id for
 * the tissue in this particular organ.
 *
 * For example:
 *
 *     Left Index Finger
 *      Bone:
 *       Hit probability: 90%
 *
 *     Left Thigh
 *      Bone:
 *       Name: Femur
 *       Hit probability: 60%
 *
 * et cetera.
 *
 * @brief This struct represents the link between an organ and its tissues.
 */
struct tissue_def{
	Tissue *tissue;
	float hit_prob;
	char *name;
	char *custom_id;
};

/** Currently, only BodyPart and Organ are derived from this class. In itself it holds
 * the Name, the ID and the relative surface of a part.
 *
 * @brief This class is the abstract superclass for all parts that make up a body.
 */
class Part{
protected:
	const char *name;
	const char *id;
	float surface;

	Part(const char *id, const char *name, float surface);
	~Part();

public:
	/** Surface Area in RMD's body definition XML is not absolute.
	 * It represents the percentage of the total surface of the BodyPart _one level
	 * above_ that this Part occupies.
	 *
	 * Example:
	 *
	 *     Body
	 *      Torso				18%
	 *      Left Arm			9%
	 *       Left Upper Arm		40%
	 *       Left Lower Arm		40%
	 *       Left Hand				20%
	 *        Left Wrist				20%
	 *        Left Palm				30%
	 *        Left Fingers				50% (10% each)
	 *      Right Arm			9%
	 *      Left Leg			18%
	 *      Right Leg			18%
	 *      Head				9%
	 *
	 * For a more detailed description, consult the [body-definition XML Help](xml_help.html).
	 * @brief Returns the relative surface area.
	 * @return The relative surface area, as defined in the [body-definition XML](xml_help.html).
	 */
	float getSurface() const {
		return surface;
	}

	const char* getName() const {
		return name;
	}

	const char* getId() const {
		return id;
	}

};

class BodyPart:Part{
private:
	TCODList<Part*> *children;

public:
	BodyPart(const char *id, const char *name, float surface);
	~BodyPart();

	void addChild(Part *child);
	bool removeChild(char *id);
};

class Organ:Part{
private:
	tissue_def *tissues;
	int tissue_count;

	bool root;
	Organ *connector;

	//TODO: Add "connected organs" list for removal!

public:
	Organ(const char *id, const char *name, float surface,
			tissue_def *tissues, int tissue_count, bool is_root=false);
	~Organ();

	void linkConnector(Organ *connector);

	bool isRoot() const {
		return root;
	}
};

class Body{
private:
	BodyPart *root;

	/**This function loads and parses a [body-definition XML](xml_help.html).
	 * The library used for this is RapidXML.
	 * __The file must be null-terminated!__
	 *
	 * @param filename The _null-terminated_ file to load.
	 * @return Returns whether or not the file was successfully loaded and parsed.
	 */
	bool loadBody(const char *filename);

public:
	Body(const char *filename);
	~Body();

};


#endif /* BODY_HPP_ */
