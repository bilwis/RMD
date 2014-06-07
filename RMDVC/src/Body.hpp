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
#include "Diagnostics.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <exception>
#include <map>
#include <stdlib.h>
#include <time.h>

/**The comparator for the tissue_map and organ_map map keys, which are std::string instances.
 */
class strless {
public:
	bool operator() (const std::string & first, const std::string & second) const {
		return first.compare(second) < 0;
	}
};

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
	/**This value represents the amount of vascularisation of the Tissue in question.
	 * The higher the value, the more blood flows from the tissue when it is destroyed.
	 *
	 * @return A float value representing the blood flow from/through the tissue.
	 */
	float getBloodFlow() const {
		return blood_flow;
	}

	/**The name of the Tissue is what is reported to the player when he inspects a body.
	 * It is distinct from its internal id returned by getId().
	 *
	 * @return The 'given name' of the tissue.
	 */
	const char* getName() const {
		return name;
	}

	/**Returns the internal id of the tissue (distinct from its name) so special functions
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

	/**This value represents the amount (quality) of nervous innervation of the Tissue in question.
	 * The higher the value, the more pain the Actor experiences when the tissue is destroyed.
	 *
	 * @return A float value representing the pain that damage to this tissue causes.
	 */
	float getPain() const {
		return pain;
	}

	/**This value represents how much this tissue deforms an intruding object, such as bullets or
	 * knives, etc.
	 *
	 * @return A float value representing the resistance against intruding objects.
	 */
	float getResistance() const {
		return resistance;
	}

	/**This value represents how much an Actor is impaired by damage to this tissue.
	 *
	 * @return A float value representing the impairment that is inflicted upon tissue damage.
	 */
	float getImpairment() const {
		return impairment;
	}

	/** This function creates a new tissue definition.
	 *  It should only be called by the Body constructor parsing the
	 *  body XML file!
	 *
	 * @brief Creates a new tissue definition.
	 *
	 * @param id Internal tissue identifier.
	 * @param name Common name for the tissue.
	 * @param pain The pain damage to this tissue causes.
	 * @param blood_flow The blood loss damage to this tissue causes.
	 * @param resistance How much this tissue absorbs energy.
	 * @param impairment How much damage to this tissue impairs the Actor.
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
public:
	Tissue *tissue;
	float hit_prob;
	const char *name;
	const char *custom_id;
};

enum PartType{
	TYPE_BODYPART = 0,
	TYPE_ORGAN = 1
};

/**BodyPart and Organ are derived from this class. In itself it holds
 * the Name, the ID and the relative surface of a part as well as a pointer
 * to the node of the organ tree that it is a child of.
 *
 * @brief This class is the abstract superclass for all parts that make up a body.
 */
class Part{
protected:
	const char *id;
	const char *name;
	float surface;
	PartType type;

	/**This is a pointer to the node of the organ tree that this Part is a child of.
	 * It is NULL for the root element, and a pointer to a BodyPart for all others.
	 */
	Part* super;

	/**This function is only called by Part's child classes BodyPart and Organ to
	 * assign the base variables.
	 *
	 * @param id The internal part identifier.
	 * @param name The name of the part.
	 * @param surface The surface area of the part. See getSurface().
	 * @param type The type of part, i.e. Organ or BodyPart. See PartType enum.
	 */
	Part(const char *id, const char *name, float surface, PartType type);

public:
	/**Surface Area in RMD's body definition XML is not absolute.
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

	/**The name of the Part is what is reported to the player when he inspects a body.
	 * It is distinct from its internal id returned by getId().
	 *
	 * @return The 'given name' of the part.
	 */
	const char* getName() const {
		return name;
	}

	/**Returns the internal id of the part (distinct from its name) so special functions
	 * may be assigned to certain parts and so that they may be referenced from code.
	 *
	 *     if (part_hit.getId() == "LEFT_HAND"){
	 *      print("Your hand is hit and you drop your weapon!");
	 *     }
	 *
	 * @return Internal part identifier.
	 */
	const char* getId() const {
		return id;
	}

	/**Returns the type of this part. This is necessary to distinguish Organs from BodyParts
	 * when only a Part* is given.
	 *
	 * @return Enum value TYPE_BODYPART or TYPE_ORGAN.
	 */
	PartType getType() const {
		return type;
	}

	/**Sets the given Part (BodyPart) as the node that this Part is a child of.
	 *
	 */
	void setSuperPart(Part* bp) {
		super = bp;
	}

	/**Returns the node that this Part is a child of.
	 *
	 * @return A pointer to a Part, or BodyPart.
	 */
	Part* getSuperPart() const {
		return super;
	}

	~Part();
};

/**This class represents the second-deepest level of the body definition, "above" the Tissue objects
 * and "below" the BodyPart objects.
 * _This definition of 'Organ' is very much different from the common meaning of 'Organ', as in
 * internal organs such as the stomach or liver._
 * Instances of this class contain an array of tissue definitions, a reference to the
 * organ to which this one is connected (upstream root) and a list of references to the organs
 * which are connected to this one (downstream branches).
 *
 * @brief The 'leaves' of the Body tree.
 */
class Organ: public Part{
private:
	tissue_def *tissues;
	int tissue_count;

	const char *connector_id;

	bool root; //Is it root?

	Organ *connector; //The upstream root
	TCODList<Organ*> *connected_organs; //The downstream branches

public:
	/**Creates a new instance of the organ class.
	 *
	 * @param id The organ's unique internal identifier.
	 * @param name The name of the organ.
	 * @param surface The relative surface area of the organ, see Part.
	 * @param tissues A pointer to the first element of the tissue_def array.
	 * @param tissue_count The number of elements in the array pointed at by tissues.
	 * @param connector_id The internal id of the connector organ, later used when the organ map
	 * is available. See linkToConnector(std::map<std::string, Organ*> *organ_map).
	 * @param is_root Whether or not this is the root element, which is the only organ
	 * without a connector.
	 */
	Organ(const char *id, const char *name, float surface,
			tissue_def *tissues, int tissue_count, const char *connector_id,
			bool is_root=false);

	/**The destructor handles deregistering with the BodyPart that this Organ is a child of,
	 * as well as the destruction of all downstream organs connected to this one.
	 */
	~Organ();

	/**
	 * @brief Links this organ to its local root.
	 * @param organ_map A map which links the internal id strings to the
	 * organ pointers.
	 */
	void linkToConnector(std::map<std::string, Organ*, strless> *organ_map);

	const char* getConnectorId();

	/**If this organ is removed/destroyed, all branches are also removed.
	 *
	 * @brief Called by the branch organs to register with their root.
	 * @param connectee The branch connecting to this organ.
	 */
	void addConnectedOrgan(Organ *connectee);

	/**Whether or not this is the root element, which is the only organ
	 * without a connector.
	 *
	 * @return Is this Organ the root organ?
	 */
	bool isRoot() const {
		return root;
	}
};

/**This class represents the upper level of the body definition. Every BodyPart connects upstream
 * to _one_ BodyPart (except for the root) and downstream to _either_ one or more BodyPart objects _or_ one
 * or more Organ objects. _It cannot hold both BodyPart and Organ children, because Organs represent the
 * 'leaves' of the body tree. (No branches grow from leaves, right?)_
 *
 * Instances of this class contain a TCODList of Part objects, which are the Organs or BodyParts
 * connected to this one.
 *
 *@brief The code representation of a part of the body (such as 'Left Arm').
 */
class BodyPart: public Part{
private:
	TCODList<Part*> *children;

public:
	/**@brief Adds a Part to the BodyPart's list of children.
	 * @param child A pointer to the Part object to add.
	 */
	void addChild(Part *child);

	/**@brief Adds several Parts to the BodyPart's list of children.
	 * @param child_array A pointer to the pointer pointing to the first of the Part objects to add.
	 * @param count The count of objects to add.
	 */
	void addChildren(Part **child_array, int count);

	/**@brief Returns a list of all the children of this BodyPart.
	 * @return A pointer to the children list.
	 */
	TCODList<Part*>* getChildList();

	/**@brief Removes a child object from the body part.
	 * @param id The internal id of the object to remove.
	 * @return Whether the removal has succeeded.
	 */
	bool removeChild(const char *id);

	/**Creates a new instance of the BodyPart class. See Part() constructor.
	 *
	 * @param id The internal part identifier.
	 * @param name The name of the part.
	 * @param surface The surface area of the part. See getSurface().
	 */
	BodyPart(const char *id, const char *name, float surface);
	~BodyPart();
};

/**This class represents the uppermost level of the body definition. It holds a single BodyPart
 * which is the root part. It also contains functions related to damage handling, loading and
 * saving of body definitions.
 *
 * @brief The code representation of a body of an Actor.
 */
class Body{
private:
	BodyPart *root;

	/**This map holds a list of all Parts (BodyParts and Organs) of a body, the key
	 * being the internal id, and the value a pointer to the Part.
	 */
	std::map<std::string, Part*, strless> *part_map;
	typedef std::map<std::string, Part*, strless>::iterator part_map_iterator;

	/**This function loads and parses a [body-definition XML](xml_help.html).
	 * The library used for this is RapidXML.
	 * __The file must be null-terminated!__
	 *
	 * @param filename The _null-terminated_ file to load.
	 * @return Returns the loaded Body as a BodyPart with all children.
	 */
	BodyPart* loadBody(const char *filename);

	/**This is the function that is recursively called on all nodes of the body
	 * definition XML. The node it is pointed at _must_ be a '<body_part>' node.
	 * The function will always create and return the BodyPart that is defined in that node.
	 *
	 * If the node contains organ definitions, the function creates the Organ objects,
	 * adds them to the BodyPart and returns it.
	 *
	 * If the node contains further '<body_part>' nodes, the function calls itself on all
	 * of them.
	 *
	 * The tissue map is needed for organ creation. Please refer to the [body-definition XML help](xml_help.html)
	 * or the source code for more.
	 * @param node The '<body_part>' XML node to parse.
	 * @param tissue_map A map of the 'default' tissues, sorted by their internal id.
	 * @return The BodyPart that is defined by node.
	 */
	BodyPart* enter(rapidxml::xml_node<> *node, std::map<std::string, Tissue*,strless> *tissue_map);

	/**This function is first called on the root BodyPart by the Body constructor and recursively on all its children.
	 * It extracts all Organs and maps them by their internal id's.
	 *
	 * @param bp The BodyPart to extract the organs from.
	 * @param organ_map The Organ map to modify.
	 * @return The modified Organ map.
	 */
	std::map<std::string, Organ*, strless>* extractOrgans(BodyPart* bp, std::map<std::string, Organ*, strless> *organ_map);

	/**This function recusively iterates through all BodyParts and Organs that lie downstream of the given BodyPart
	 * and compiles a map of pointers to them, keyed by their internal ids.
	 *
	 * @param bp The BodyPart to extract the parts from.
	 * @param part_map The Part map to modify.
	 * @return The modified Part map.
	 */
	std::map<std::string, Part*, strless>* extractParts(BodyPart* bp, std::map<std::string, Part*, strless> *part_map);

	/**This function recursively iterates through bodyparts until it reaches the Organ "leaves" and calls the
	 * Organ.linkToConnector(organ_map) function on them.
	 *
	 * @param bp The BodyPart to "scan" for organs.
	 * @param organ_map A map which links the internal id strings of the organs to pointers to their instance.
	 */
	void linkOrgans(BodyPart* bp, std::map<std::string, Organ*, strless> *organ_map);

	void createSubgraphs(std::ofstream* stream, BodyPart* bp);
	void createLinks(std::ofstream* stream, BodyPart* bp);

public:
	/**This function creates a new instance of the Body class and loads and parses the body definition
	 * from a [body-definition XML](xml_help.html).
	 * @param filename The path to the [body-definition XML](xml_help.html).
	 */
	Body(const char *filename);
	~Body();

	/**This function removes the a Part of the Body, identified by the given internal id.
	 * It also handles deregistering and destruction of the removed elements via their destructors.
	 *
	 * @param part_id The internal id of the Part to remove.
	 * @returns True if the operation was succesful, False otherwise.
	 */
	bool removePart(std::string part_id);

	/**This function removes a random Part of the Body.
	 */
	void removeRandomPart();

	void printBodyMap(const char* filename, BodyPart* mroot);

};

/**An exception that is thrown by the functions loading and parsing the [body-definition XML](xml_help.html).
 */
class bdef_parse_error:std::exception
{
private:
	const char* msg;
	rapidxml::xml_node<> *node;

public:
	bdef_parse_error(const char* msg, rapidxml::xml_node<>* node);

	virtual const char* what() const throw()
	{
		std::ostringstream os;

		os << "Error while parsing body definition XML:\n"
		<< msg
		<< "\n On node: '"
		<< node->name()
		<< "' with value: '"
		<< node->value()
		<< "'\n";

		return os.str().c_str();
	}
};


#endif /* BODY_HPP_ */