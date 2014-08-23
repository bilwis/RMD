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
#include "GUI_structs.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <exception>
#include <map>
#include <memory>
#include <stdlib.h>
#include <time.h>
#include <vector>

using std::string;

/**The comparator for the tissue_map and organ_map map keys, which are std::string instances.
 */
class strless {
public:
	bool operator() (const string & first, const string & second) const {
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
	string id;
	string name;
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
	string getName() const {
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
	string getId() const {
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
	Tissue(string id, string name, float pain,
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
	std::shared_ptr<Tissue> tissue;
	float hit_prob;
	string name;
	string custom_id;
};

enum PartType{
	TYPE_BODYPART = 0,
	TYPE_ORGAN = 1
};

extern const char* part_type_strings[];

class Body;

/**BodyPart and Organ are derived from this class. In itself it holds
 * the Name, the internal ID and the relative surface of a part as well as a pointer
 * to the node of the organ tree that it is a child of.
 *
 * @brief This class is the abstract superclass for all parts that make up a body.
 */
class Part: public Object{

protected:

	/** This _internal ID_ is different from the UUID that this and every other instance of
	* a class that derives from Object posesses. It is used to refer to _specific parts_ of the
	* Body object that is created during runtime by parsing the body-definition XML.
	*
	* Example (internal ID is "LEFT_HAND"):
	*     if (part_hit.getId() == "LEFT_HAND"){
	*      print("Your hand is hit and you drop your weapon!");
	*     }
	*
	* The same sort of reference can neither be made by name, because several parts may share a name;
	* nor by UUID, because it is generated during runtime and different every time the body-definition XML
	* is parsed into a Body object.
	*/
	string id;

	/** A pointer to the Body instance this Part is... well, a part of.
	*
	*/
	Body* body;
	
	string name;
	float surface;
	PartType type;

	/**This is the UUID of the node of the organ tree that this Part is a child of.
	 * It is NULL for the root element.
	 */
	string super;

	/**This function is only called by Part's child classes BodyPart and Organ to
	 * assign the base variables.
	 *
	 * @param id The internal part identifier.
	 * @param name The name of the part.
	 * @param surface The surface area of the part. See getSurface().
	 * @param type The type of part, i.e. Organ or BodyPart. See PartType enum.
	 * @param b The body this Part is part of.
	 */
	Part(string id, string name, float surface, PartType type, Body* b);

public:
	string toString() { return name; }

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
	string getName() const {
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
	string getId() const {
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
	void setSuperPart(string bp_uuid) {
		super = bp_uuid;
	}

	/**Returns the UUID of the node that this Part is a child of.
	 *
	 * @return The UUID of a BodyPart.
	 */
	std::string getSuperPartUUID() const {
		return super;
	}

	virtual void testFunction() {
		return;
	};

	virtual ~Part();
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

	string connector_id;
	string connector_uuid; //The upstream root

	bool root; //Is it root?

	std::vector<string>* connected_organs; //The downstream branches

	bool is_stump = false; //When Organs downstream are removed, the Organ is marked as stump

	/**
	* @brief Links this organ to its local root.
	* @param connector_uuid The UUID of the connector.
	*/
	void linkToConnector(string connector_uuid);

public:
	/**Creates a new instance of the organ class.
	 *
	 * @param id The organ's unique internal identifier.
	 * @param name The name of the organ.
	 * @param surface The relative surface area of the organ, see Part.
	 * @param b The body this Organ is part of.
	 * @param tissues A pointer to the first element of the tissue_def array.
	 * @param tissue_count The number of elements in the array pointed at by tissues.
	 * @param connector_id The internal id of the connector organ, later used when the organ map
	 * is available. See linkToConnector(std::map<std::string, Organ*> *organ_map).
	 * @param is_root Whether or not this is the root element, which is the only organ
	 * without a connector.
	 */
	Organ(string id, string name, float surface, Body* b,
			tissue_def *tissues, int tissue_count, const char *connector_id,
			bool is_root=false);

	/**The destructor handles deregistering with the BodyPart that this Organ is a child of,
	 * as well as the destruction of all downstream organs connected to this one.
	 */
	~Organ();

	/**
	* @brief Returns the UUID of the upstream root organ.
	*/
	string getConnectorUUID(){
		return connector_uuid;
	};

	/**
	* @brief Returns the internal ID of the upstream root organ.
	*/
	string getConnectorId(){
		return connector_id;
	};

	/**
	* @brief Returns a new vector containing all connected organs.
	*/
	std::vector<string>* getConnectedOrgans();

	/**
	* @brief Returns the pointer to the connected_organs vector;
	*/
	std::vector<string>* getConnectedOrgansRW() { return connected_organs; }

	/**If this organ is removed/destroyed, all branches are also removed.
	 *
	 * @brief Called by the branch organs to register with their root.
	 * @param connectee The branch connecting to this organ.
	 */
	void addConnectedOrgan(string connectee_uuid);
	void removeConnectedOrgan(string uuid);

	bool isStump() { return is_stump; }

	/**Whether or not this is the root element, which is the only organ
	 * without a connector.
	 *
	 * @return true, if the Organ is the root; false otherwise.
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
 * Instances of this class contain a vector of the UUIDs of Part objects, which are the Organs or BodyParts
 * connected to this one.
 *
 *@brief The code representation of a part of the body (such as 'Left Arm').
 */
class BodyPart: public Part{
private:
	std::vector<string>* children;

public:
	/**@brief Adds a Part to the BodyPart's list of children.
	 * @param child A shared pointer to the Part object to add.
	 * @param body The body this BodyPart and the child are part of. This reference
	 *  is necessary to ensure the childs super reference is set to a shared_ptr of this bodypart.
	 */
	void addChild(string child_uuid);

	/**@brief Adds several Parts to the BodyPart's list of children.
	 * @param child_array A pointer to the pointer pointing to the first of the Part objects to add.
	 * @param count The count of objects to add.
	 */
	void addChildren(std::vector<string>* child_uuid_vector);

	/**@brief Returns a pointer to a new vector containing all the UUIDs 
	 * of the children of this BodyPart.
	 */
	std::vector<string>* getChildList();

	/**@brief Returns the pointer to the children vector.
	*/
	std::vector<string>* getChildListRW() { return children; }

	/**@brief Removes a child object from the body part. If the child to be
	 * destroyed is the last one, this function will destroy the BodyPart itself.
	 *
	 * @param uuid The UUID of the object to remove.
	 * @return Whether the removal has succeeded.
	 */
	bool removeChild(string uuid);

	/**Creates a new instance of the BodyPart class. See Part() constructor.
	 *
	 * @param id The internal part identifier.
	 * @param name The name of the part.
	 * @param surface The surface area of the part. See getSurface().
	 * @param b The body this BodyPart is part of.
	 */
	BodyPart(string id, string name, float surface, Body* b);
	~BodyPart();
};

/**This class represents the uppermost level of the body definition. It holds several maps that
 * are necessary for code handling of Actor bodies, the two most important being the
 * part_map and the tissue_map, which hold shared pointers to every Part and Tissue element that
 * the Body is "composed" of. The shared pointers are shared with the Parts themselves (e.g., the 
 * child vector of the BodyParts holds some of the shared pointers (namely those pointing to it's children)).
 * This means that the Body instance is effectively "managing" the Parts and Tissues that it is
 * composed of. It also contains functions related to damage handling, loading and
 * saving of body definitions.
 *
 * @brief The code representation of a body of an Actor.
 */
class Body{
private:
	/**This shared pointer points to the root BodyPart of the stored Body. It can be used
	* for recursive iteration through the child lists of the "attached" bodyparts, for example.
	*/
	std::shared_ptr<BodyPart> root;

	const TCODColor part_gui_list_color_bodypart = TCODColor::azure;
	const TCODColor part_gui_list_color_organ = TCODColor::darkRed;

	/**This map holds a list of all Tissue elements that are defined in the [body-definition XML](xml_help.html)
	* that was used to create this Body instance. The key is the UUID, the value a shared pointer
	* to the Tissue instance. This shared pointer is shared, for example, by the Organs in the part_map,
	* who hold a list of shared pointers to the Tissue elements that they are "composed" of.
	*/
	std::map<std::string, std::shared_ptr<Tissue>>* tissue_map;

	/**This map holds a list of all Parts (BodyParts and Organs) of a body, the key
	 * being the UUID, and the value a shared pointer to the Part.
	 */
	std::map<std::string, std::shared_ptr<Part>>* part_map;

	/**This map holds a list of all UUID's and all internal id's of all Parts (BodyParts and Organs)
	* of a body. The key is the internal id, the value the UUID. 
	*
	* Because the parts are internally stored in the part_map map, whose values are pointers to
	* the part objects and whose keys are the UUIDs of the parts, this map is needed to access
	* parts by their internal id as defined the body-definition XML.
	*/
	std::map<std::string, std::string>* iid_uuid_map;

	/**This list holds all Parts (BodyParts and Organs) of a body in a GuiObjectLink format.
	* The UUID stored is that of the part, the ColoredText is formatted to represent the "depth"
	* of the part within the body structure. (See GuiBodyViewer class for "usage")
	*/
	std::vector<GuiObjectLink*>* part_gui_list;

	/**This function loads and parses a [body-definition XML](xml_help.html).
	 * The library used for this is RapidXML.
	 * __The file must be null-terminated!__
	 *
	 * @param filename The _null-terminated_ file to load.
	 * @return Returns the UUID of the root bodypart.
	 */
	string loadBody(const char *filename);

	/**This is the function that is recursively called on all nodes of the body
	 * definition XML. The node it is pointed at _must_ be a '<body_part>' node.
	 * The function will create the BodyPart that is defined in that node and return its UUID.
	 *
	 * If the node contains organ definitions, the function creates the Organ objects,
	 * and adds an "link note" into a temporary map, which is used in the loadBody() function to
	 * link the newly created Parts together.
	 *
	 * If the node contains further '<body_part>' nodes, the function calls itself on all
	 * of them.
	 *
	 * Please refer to the [body-definition XML help](xml_help.html)
	 * or the source code for more information on the Body XML Parsing.
	 * @param node The '<body_part>' XML node to parse.
	 * @param child_map A map of Parts to be linked as parent <-> child, the child UUID being the key
	 *  and the parent UUID being the value.
	 * @param organ_map A map of Organs to be linked as connector <-> connectee, the connectee UUID being the key
	 *  and the connector UUID being the value.
	 * @return The UUID of the BodyPart that is defined by node.
	 */
	string enter(rapidxml::xml_node<> *node, std::map<string, string>* child_map, std::map<string, string>* organ_link_map);

	/**This function iterates through the given Part map and creates a map whose keys are the internal
	* id's and whose values are the UUID (the iid_uuid_map !) and refreshes iid_uuid_map.
	*/
	void makeIdMap();

	/**This function recursively builds a list of GuiObjectLink for all Parts of the Body - 
	* it links the UUID of the Part to a ColoredText containing the name of the Part. 
	* The text is colored corresponding to part_gui_list_color_bodypart or part_gui_list_color_organ.
	* 
	* @param list The vector to modify.
	* @param p The Part to start from.
	* @param depth This should be 0 on first call and is increased by the recursive calling of this function
	*  to correcly indent the texts.
	*/
	void buildPartList(std::vector<GuiObjectLink*>* list, Part* p, int depth=0);

	/**This function calls makeIdMap() and buildPartList(...) to refresh the iid_uuid_map and
	* the part_gui_list to match the part_map.
	*/
	void refreshLists();

	/**This function removes an element from the part_map, identified by the given UUID.
	* Due to the nature of the part_map (storing shared_pointers accessible by the UUID of the
	* Part they point at), removal of the element will cause the destruction of the shared_pointer
	* (because it should not be referenced anywhere else) and therefore, the Part it points at.
	*
	* @param uuid The UUID of the Part to unregister.
	*/
	void unregisterPart(string uuid);
	void unregisterParts(std::vector<string>* uuids);

	/**This function iterates recursively through all Parts downstream of the given Part
	* and adds - for Organs - all connected_organs, - for BodyParts - all children to the
	* given vector. The first call to this function should therefore modify the vector to 
	* contain all children of all parts downstream of the given Part, identified by the given UUID.
	* Note that this function will not check for duplicates.
	* 
	* @param part_uuid The UUID of the Part to list the children and children's children of.
	* @param child_list A vector, which will be modified by this function to contain all children
	*  and children's children (...) of the given Part.
	*/
	void makeDownstreamPartList(std::string part_uuid, std::vector<string>* child_list);
	
	void createSubgraphs(std::ofstream* stream, BodyPart* bp);
	void createLinks(std::ofstream* stream, BodyPart* bp);

public:
	/**This function creates a new instance of the Body class and loads and parses the body definition
	 * from a [body-definition XML](xml_help.html).
	 * @param filename The path to the [body-definition XML](xml_help.html).
	 */
	Body(const char *filename);
	~Body();

	/**This function returns a shared pointer to the root BodyPart.
	* @return A shared pointer pointing at the root BodyPart.
	*/
	std::shared_ptr<BodyPart> getRootBP()
	{
		if (root != nullptr) { return root; }
	};

	/**This function removes the a Part of the Body, identified by the given UUID.
	 * It also handles removal of all Parts downstream of that Part and removal of 
	 * now-empty Parts upstream of it.
	 *
	 * @param part_uuid The UUID of the Part to remove.
	 */
	void removePart(std::string part_uuid);

	/**This function removes several parts one after another. For that purpose, it
	 * calls removePart() on all elements of the given vector of UUIDs.
	 *
	 * @param part_uuids A vector of UUIDs to remove.
	 */
	void removeParts(std::vector<string>* part_uuids);

	/**This function removes a random Part of the Body.
	 */
	void removeRandomPart();

	/**This function returns the part_gui_list.
	*/
	std::vector<GuiObjectLink*>* getPartGUIList() { return part_gui_list; }

	/**This function returns a shared pointer to the Part identified by the given UUID,
	* or a nullptr if the Part could not be found.
	*
	* @param uuid The UUID of the Part to get.
	* @return A shared pointer to the Part, or a nullptr.
	*/
	std::shared_ptr<Part> getPartByUUID(std::string uuid);
	/**This function returns a shared pointer to the Part identified by the given IID,
	* or a nullptr if the Part could not be found.
	*
	* @param uuid The IID of the Part to get.
	* @return A shared pointer to the Part, or a nullptr.
	*/
	std::shared_ptr<Part> getPartByIID(std::string iid);

	void printBodyMap(const char* filename, BodyPart* mroot);

};

/**An exception that is thrown by the functions loading and parsing the [body-definition XML](xml_help.html).
 */
class bdef_parse_error:public std::runtime_error
{
public:
	bdef_parse_error(const std::string& msg, rapidxml::xml_node<>* node);
};


#endif /* BODY_HPP_ */
