/*
 * Body.cpp
 *
 *  Created on: 19 May 2013
 *      Author: Clemens
 */

#include "Body.hpp"

const char* part_type_strings[] = { "BodyPart", "Organ" };

bdef_parse_error::bdef_parse_error(const std::string& msg, rapidxml::xml_node<>* node):
	std::runtime_error("\aError while parsing body definition XML:\n" + msg + " on node " + node->name() + " with value " + node->value() + "\n")
{
};

Tissue::Tissue(string id, string name, float pain,
		float blood_flow, float resistance, float impairment) :
		pain(pain),blood_flow(blood_flow),resistance(resistance),
		impairment(impairment){

	/* The XML parsing function works with pointers to strings only.
	 * Every "id", "name" and any other string is only copied into memory ONCE, and that
	 * is during the initial call of rapidxml.parse. All subsequent references to the variables
	 * are pointers to this initial memory location. The rapidxml memory buffer is wiped
	 * at the end of the loadBody function, however. Therefore all new instances of the Part
	 * or Tissue class, which hold the strings for later use in the program, must ACTUALLY COPY
	 * THE MEMORY to which the "parsing pointer" points and maintain a reference to it via their
	 * own id and name (etc.) variables, which are pointers to the new, copied memory slot.
	 */

	this->id = string(id);
	this->name = string(name);

}

Tissue::~Tissue()
{
}

Part::Part(string id, string name, float surface, PartType type, Body* b) : Object()
{

	/* The XML parsing function works with pointers to strings only.
	 * Every "id", "name" and any other string is only copied into memory ONCE, and that
	 * is during the initial call of rapidxml.parse. All subsequent references to the variables
	 * are pointers to this initial memory location. The rapidxml memory buffer is wiped
	 * at the end of the loadBody function, however. Therefore all new instances of the Part
	 * or Tissue class, which hold the strings for later use in the program, must ACTUALLY COPY
	 * THE MEMORY to which the "parsing pointer" points and maintain a reference to it via their
	 * own id and name (etc.) variables, which are pointers to the new, copied memory slot.
	 */
	this->name = string(name);
	this->id = string(id);

	this->surface = surface;
	this->type = type;

	this->body = b;

	super = "";
}

Part::~Part()
{
}

BodyPart::BodyPart(string id, string name, float surface, Body* b) :
		Part(id, name, surface, TYPE_BODYPART, b){

	children = new std::vector<string>();
}

BodyPart::~BodyPart()
{
	debug_print("BodyPart %s is kill.\n", this->id.c_str());

	delete children;
}

void BodyPart::addChild(string child_uuid){
	body->getPartByUUID(child_uuid)->setSuperPart(getUUID());
	children->push_back(child_uuid);
}

void BodyPart::addChildren(std::vector<string>* child_uuid_vector){
	for (std::vector<string>::iterator it = child_uuid_vector->begin();
		it != child_uuid_vector->end(); it++){
		children->push_back(*it);
		body->getPartByUUID(*it)->setSuperPart(getUUID());
	}
}

std::vector<string>* BodyPart::getChildList()
{
	return new std::vector<string>(children->begin(), children->end());
}

bool BodyPart::removeChild(string uuid){
	
	//Iterate though the list of children and remove the child with the matching UUID
	for (std::vector<string>::iterator it = children->begin(); it != children->end(); it++)
	{
		
		if (uuid.compare(*it) == 0){
			children->erase(it);

			//If the BodyPart is now empty, it removes itself
			if (children->size() == 0)
			{
				body->removePart(getUUID());
			}

			//Dont continue iterating, vector has been changed!
			break;
		}
	}

	return false;
}

Organ::Organ(string id, string name, float surface, Body* b,
		tissue_def *tissues, int tissue_count, const char *connector_id, bool is_root):
		Part(id, name, surface, TYPE_ORGAN, b), tissues(tissues), tissue_count(tissue_count),
		root(is_root){

	/* The XML parsing function works with pointers to strings only.
	 * Every "id", "name" and any other string is only copied into memory ONCE, and that
	 * is during the initial call of rapidxml.parse. All subsequent references to the variables
	 * are pointers to this initial memory location. The rapidxml memory buffer is wiped
	 * at the end of the loadBody function, however. Therefore all new instances of the Part
	 * or Tissue class, which hold the strings for later use in the program, must ACTUALLY COPY
	 * THE MEMORY to which the "parsing pointer" points and maintain a reference to it via their
	 * own id and name (etc.) variables, which are pointers to the new, copied memory slot.
	 */
	if (connector_id == "_ROOT")
	{
		this->connector_id = "_ROOT";
		root = true;
	}
	else {
		this->connector_id = string(connector_id);
	}
	
	connected_organs = new std::vector<string>();
}

Organ::~Organ(){
	debug_print("Organ %s is kill.\n", this->id.c_str());

	delete connected_organs;
}

void Organ::linkToConnector(string connector_uuid){
	this->connector_uuid = connector_uuid;
	debug_print("LINKED %s to connector %s\n", getUUID().c_str(), connector_uuid.c_str());
}

std::vector<string>* Organ::getConnectedOrgans()
{
	return new std::vector<string>(connected_organs->begin(), connected_organs->end());
}

void Organ::addConnectedOrgan(string connectee_uuid){
	connected_organs->push_back(connectee_uuid);
	std::dynamic_pointer_cast<Organ>(body->getPartByUUID(connectee_uuid))->linkToConnector(getUUID());
}

void Organ::removeConnectedOrgan(string uuid) {
	//This function just removes the UUID! It doesn't deregister the Organ from the Body!!!
	for (std::vector<string>::iterator it = connected_organs->begin(); it != connected_organs->end(); it++)
	{
		if (uuid.compare(*it) == 0)
		{
			connected_organs->erase(it);
			is_stump = true;
			//Dont continue iterating, vector has been changed!
			break;
		}
	}
}

Body::Body(const char *filename){
	tissue_map = new std::map<std::string, std::shared_ptr<Tissue>>();
	part_map = new std::map<std::string, std::shared_ptr<Part>>();
	iid_uuid_map = new std::map<std::string, std::string>();
	part_gui_list = new std::vector<GuiObjectLink*>();
	
	string root_uuid = loadBody(filename);
	root = std::dynamic_pointer_cast<BodyPart>(getPartByUUID(root_uuid));
	refreshLists();
	
#ifdef _DEBUG
		printBodyMap("body.gv", root.get());
		debug_print("Printed Body Map! \n");
#endif
}

Body::~Body(){
	delete tissue_map;
	delete part_map;
	delete iid_uuid_map;
	delete part_gui_list;
}

string Body::loadBody(const char *filename){
	
	//###XML FILE HANDLING###
	using namespace rapidxml;
	char *buffer;

	//Load XML file
	try{
	std::ifstream is (filename);

	if (is) {
		//Get file length
		is.seekg(0, is.end);

		int length = is.tellg();
		is.seekg(0, is.beg);

		buffer = new char[length];

		is.read(buffer, length);
		//buffer now holds entire XML file
	} else { return false; }

	//Parse XML file
	// WARNING: FILE MUST BE NULL-TERMINATED!

	xml_document<> doc;
	doc.parse<0>(buffer);


	//###TISSUE DATA###

	//Temporary variables
	char *id = nullptr, *name = nullptr;
	float pain = -1.0f, blood_flow = -1.0f, resistance = -1.0f, impairment = -1.0f; //Initialize with illegal values

	//Load the tissue data (First first_node() is "body_def",
	// data starts with second level "tissues")
	xml_node<> *tissues = doc.first_node()->first_node("tissues");

	char *_name;

	//Iterate through all tissue definitions
	for (xml_node<> *tissue = tissues->first_node();
			tissue;
			tissue = tissue->next_sibling())
	{
		//Iterate through the tissue attributes
		for (xml_node<> *attr = tissue->first_node();
				attr;
				attr = attr->next_sibling())
		{

			_name = attr->name();
			if (!strcmp(_name,"id")){ id = attr->value(); }
			else if (!strcmp(_name,"name")){ name = attr->value(); }
			else if (!strcmp(_name,"pain")){ pain = atof(attr->value()); }
			else if (!strcmp(_name,"blood_flow")){ blood_flow = atof(attr->value()); }
			else if (!strcmp(_name,"resistance")){ resistance = atof(attr->value()); }
			else if (!strcmp(_name,"impairment")){ impairment = atof(attr->value()); }
		}

		debug_print("Read Tissue:\n\tID: %s \n\tName: %s \n\tBlood Flow: %f \n\tResistance: %f \n\tImpairment: %f\n",
				id, name, blood_flow, resistance, impairment);

		//Create a new Tissue and store the shared pointer to it in the tissue map
		std::shared_ptr<Tissue> t_tissue(new Tissue(id, name, pain, blood_flow, resistance, impairment));
		tissue_map->insert(std::pair<std::string, std::shared_ptr<Tissue>>(std::string(id), t_tissue));
	}

	//###BODYPART DATA###

	//maps for child linking and organ linking
	//K: UUID of the child, V: UUID of the parent
	std::map<string, string>* child_map = new std::map<string, string>();
	//K: UUID of the organ, V: IID(!) of its connector
	std::map<string, string>* organ_link_map = new std::map<string, string>();

	//Load the bodypart data (First first_node() is "body_def", 
	// data starts with the second level "body" 
	xml_node<> *body = doc.first_node()->first_node("body");
	string root_uuid = enter(body->first_node(), child_map, organ_link_map);

	//Destroy the XML data in memory
	delete[] buffer;

	//Build IID<->UUID map, which is required for linking 
	makeIdMap();

	//Link children and organs
	for (std::map<string, string>::iterator ch_it = child_map->begin();
		ch_it != child_map->end(); ch_it++) 
	{
		if (getPartByUUID(ch_it->first) == nullptr)
		{
			debug_error("ERROR during body part linking: Part UUID %s was requested, but is not in part map!\n",
				ch_it->first.c_str());
			return nullptr;
		}
		if (getPartByUUID(ch_it->second) == nullptr)
		{
			debug_error("ERROR during body part linking: Part UUID %s was requested, but is not in part map!\n",
				ch_it->second.c_str());
			return nullptr;
		}
		
		BodyPart* parent = dynamic_cast<BodyPart*>(getPartByUUID(ch_it->second).get());
		if (parent == nullptr) 
		{
			debug_error("ERROR during body part linking: Casting from Part* to BodyPart* on Part %s failed!\n",
				ch_it->second.c_str());
			return nullptr;
		}

		parent->addChild(ch_it->first);
	}

	for (std::map<string, string>::iterator or_it = organ_link_map->begin();
		or_it != organ_link_map->end(); or_it++)
	{
		if (getPartByUUID(or_it->first) == nullptr)
		{
			debug_error("ERROR during body part linking: Part UUID %s was requested, but is not in part map!\n",
				or_it->first.c_str());
			return nullptr;
		}
		if (getPartByIID(or_it->second) == nullptr)
		{
			debug_error("ERROR during body part linking: Part IID %s was requested, but is not in part map!\n",
				or_it->second.c_str());
			return nullptr;
		}
		Part* temp = getPartByIID(or_it->second).get();
		Organ* connector = dynamic_cast<Organ*>(temp);
		if (connector == nullptr){
			debug_error("ERROR during body part linking: Casting from Part* to Organ* on Part %s failed!\n",
				or_it->second.c_str());
			return nullptr;
		}
		connector->addConnectedOrgan(or_it->first);
	}

	//Successfully parsed the body definition file!
	//Return the address of the newly created body part.
	return root_uuid;

	} catch (std::exception& e) {
		debug_error("ERROR: %s \n", e.what());
		return NULL;
	}
	catch (bdef_parse_error& pe) {
		debug_error("ERROR: %s \n", pe.what());
		return NULL;
	}

	return NULL;
}

string Body::enter(rapidxml::xml_node<> *node, std::map<string, string>* child_map, std::map<string, string>* organ_link_map) {
	using namespace rapidxml;

	int organ_count, bodyparts, it;
	xml_node<> *temp;
	xml_node<> **organ_node_list;

	char *id = nullptr, *name = nullptr, *_name = nullptr;
	float surface = 0.0f;

	//Make a count of the body_part nodes in this node and parse all standard
	// nodes for the bodypart of this node
	temp = node->first_node();
	bodyparts = 0;
	while (temp != nullptr){
		_name = temp->name();

		if (!strcmp(_name, "body_part")) { bodyparts++; }

		if (!strcmp(_name, "id")) { id = temp->value(); }
		if (!strcmp(_name, "name")) { name = temp->value(); }
		if (!strcmp(_name, "surface")) { surface = atof(temp->value()); }


		temp = temp->next_sibling();
	}

	//if any of the mandatory vars for bodyparts are NULL, ERROR!
	if (id == nullptr || name == nullptr){
		throw new bdef_parse_error("Not all mandatory BodyPart variables defined!", node);
	}

	//make the bodypart, make the pointer to it shared and store it in the part_map
	BodyPart* bp = new BodyPart((string)id, (string)name, surface, this);

	std::shared_ptr<BodyPart> p (bp);

	part_map->insert(
		std::pair<std::string, std::shared_ptr<Part>>(
		bp->getUUID(), std::static_pointer_cast<Part>(p)));

	//reset temporary variables for reuse with the organs
	id = nullptr; name = nullptr;

	//DEBUG: Print new BodyPart
	debug_print("New BodyPart created: \n\tID: %s \n\tName: %s \n\tSurface: %f\n",
			bp->getId().c_str(), bp->getName().c_str(), bp->getSurface());

	//If there are no body_part nodes...
	if (bodyparts == 0){
		//...there must be organs instead!
		// variables
		Organ **organs;
		xml_attribute<> *attr;
		xml_node<> *tdef_node;

		char *connector = nullptr;
		bool symmetrical = false;

		it = 0;

		//  temp vars for tissue definitions
		char *tdef_id, *tdef_custom_id, *tdef_name;
		float tdef_hit_prob;

		tissue_def *tdefs = nullptr;
		int tdef_count = 0, tdef_it = 0;


		//Reset back to the first node in the given node
		temp = node->first_node();

		//scan for organ nodes in the given node
		organ_count = 0;
		while (temp != nullptr){
			if (!strcmp(temp->name(),"organ")) { organ_count++; }
			temp = temp->next_sibling();
		}

		//If there are no organs AND no bodyparts, ERROR!
		if (organ_count == 0){
			throw bdef_parse_error("No body part and no organ definition!", node);
		}

		//compile a list of nodes holding the organ definitions
		//and parse the remaining nodes to make the bodypart
		organ_node_list = new xml_node<>*[organ_count];
		temp = node->first_node();

		while (temp != nullptr){
			_name = temp->name();

			if (!strcmp(_name,"organ")) {
				organ_node_list[it] = temp;
				it++;
			}

			temp = temp->next_sibling();
		}

		//create temporary organ array
		organs = new Organ*[organ_count];

		//parse each organ definition in the list
		for (int i=0; i < organ_count; i++){
			//Enter into organ node
			temp = organ_node_list[i]->first_node();

			//Iterate through all nodes within the organ node
			while (temp != nullptr){
				_name = temp->name();

				//Parse standard tags
				if (!strcmp(_name,"id")) { id = temp->value(); }
				if (!strcmp(_name,"name")) { name = temp->value(); }
				if (!strcmp(_name,"surface")) { surface = atof(temp->value()); }
				if (!strcmp(_name,"connector")) { connector = temp->value(); }

				//Parse the organ tissue definitions, create (or rather link) them
				// and add to the organs
				if (!strcmp(_name,"organ_tissue")) {
					attr = temp->first_attribute();
					if (attr != nullptr && !strcmp(attr->name(), "symmetrical")){
						symmetrical = true;
					}

					attr = nullptr;

					//Get the count of tissue defs
					tdef_count = 0;
					tdef_node = temp->first_node();

					while (tdef_node != nullptr)
					{
						//If any other node than tissue_def, ERROR!
						if (strcmp(tdef_node->name(), "tissue_def")) {
							throw bdef_parse_error("Invalid node for organ tissue (only tissue_def allowed)!", temp);
						}

						tdef_count++;

						tdef_node = tdef_node->next_sibling();
					}

					//Create organ tissue array, double size if organ is symmetrical
					tdefs = new tissue_def[tdef_count * (symmetrical+1)];

					//Enter the organ_tissue node, create the tissue_def for
					// each tissue_def child and store it in tdefs[] array
					tdef_it = 0;
					tdef_node = temp->first_node();
					while (tdef_node != nullptr){
						//The value of a tissue_def node is the id of the tissue (e.g. M_ARTERY)
						tdef_id = tdef_node->value();

						tdef_hit_prob = 0;
						tdef_name = nullptr;
						tdef_custom_id = nullptr;

						//Get the other parameters
						attr = tdef_node->first_attribute();
						while (attr != nullptr){

							if (!strcmp(attr->name(), "hit_prob")){ tdef_hit_prob = atof(attr->value()); }
							if (!strcmp(attr->name(), "name")) { tdef_name = attr->value(); }
							if (!strcmp(attr->name(), "custom_id")) {
								tdef_custom_id = attr->value();
							}

							attr = attr->next_attribute();
						}


						//Store the parameters in the tissue definition
						if (tdef_custom_id != nullptr) {
							tdefs[tdef_it].custom_id = string(tdef_custom_id);
						}
						else { tdefs[tdef_it].custom_id = ""; }

						if (tdef_name != nullptr) {
							tdefs[tdef_it].name = string(tdef_name);
						}
						else { tdefs[tdef_it].name = ""; }
						
						tdefs[tdef_it].hit_prob = tdef_hit_prob;

						//Link the tissue definition to it's base tissue
						// NOTE: the tissue_map variable is a POINTER to the
						//  actual tissue map which is defined in the loadBody (super)
						//  function, therefore it must be dereferenced before use.
						//If the base tissue cannot be linked, ERROR!

						std::string key = tdef_id;

						std::map<std::string, std::shared_ptr<Tissue>>::const_iterator pos
							= tissue_map->find(key);

						if (pos == tissue_map->end()){ throw bdef_parse_error("Tissue not found!", tdef_node); }
						tdefs[tdef_it].tissue = pos->second;

						tdef_it++;
						tdef_node = tdef_node->next_sibling();
					}
				}
				temp = temp->next_sibling();
			}

			//Check whether all necessary data for organ creation has been read
			// if not, ERROR!
			if (id == nullptr || name == nullptr || connector == nullptr || tdefs == nullptr){
				throw bdef_parse_error("Not all necessary data for organ creation found.", organ_node_list[i]);
			}

			//if organ is symmetrical, create the symmetry by duplicating all entries but the last
			// and appending them in reverse order
			int d;
			for (int j=tdef_count-2; j >= 0; j--)
			{
				d = (j - (tdef_count - 1))*-1;
				tdefs[(tdef_count-1)+d] = tdefs[j];
			}

			//Create the organ
			organs[i] = new Organ(string(id), string(name), surface, this, tdefs, tdef_count, connector, !strcmp(connector, "_ROOT"));

			//DEBUG: Print Organ
#ifdef _DEBUG
			debug_print("\tNew Organ created:\n\t\tID: %s \n\t\tName: %s \n\t\tSurface: %f \n\t\tRoot: %s \n\t\tTissues:",
				organs[i]->getId().c_str(), organs[i]->getName().c_str(), organs[i]->getSurface(), connector);

			for (int di = 0; di < (tdef_count * (symmetrical+1))-1; di++){
				debug_print("\n\t\t\tBase Tissue Name: %s \n\t\t\t\tHit Prob.: %f",
					tdefs[di].tissue->getName().c_str(), tdefs[di].hit_prob);

				if (!tdefs[di].name.empty()){ debug_print("\n\t\t\t\tCustom Name: %s", tdefs[di].name.c_str()); }
				if (!tdefs[di].custom_id.empty()){ debug_print("\n\t\t\t\tCustom ID: %s", tdefs[di].custom_id.c_str()); }
			}

			debug_print("\n\tEND Organ.\n");
#endif
		}

		//...and add all organs to the part map. Add the "child note"
		// between the organs and the new bodypart to the child_map.
		// Add the "connector note" between the organs and their connectors to 
		// the organ_link_map.
		for (int i = 0; i < organ_count; i++){
			part_map->insert(
				std::pair<string, std::shared_ptr<Part>>(
				organs[i]->getUUID(),
				std::static_pointer_cast<Part>(std::make_shared<Organ>(*(organs[i])))
				));

			child_map->insert(
				std::pair<string, string>(
				organs[i]->getUUID(),
				bp->getUUID()
				));

			if (organs[i]->getConnectorId() != "_ROOT"){
				organ_link_map->insert(
					std::pair<string, string>(
					organs[i]->getUUID(),
					dynamic_cast<Organ*>(organs[i])->getConnectorId()
					));
			}
		}
		

	} else {
		//...its another BodyPart

		//Go through all nodes and call this function on every
		// node containing a part definition, add the returned BodyPart/Organ
		// to this ones' children (per child_map)
		temp = node->first_node();

		it = 0;
		while (temp != nullptr){
			_name = temp->name();

			if (!strcmp(_name, "body_part")) {
				string child_uuid = enter(temp, child_map, organ_link_map);
				child_map->insert(
					std::pair<string, string>(
					child_uuid,
					bp->getUUID()
					));
				//bp->addChild(enter(temp));
			}

			temp = temp->next_sibling();
		}
	}

	//return this bodyparts uuid
	return bp->getUUID();
}

void Body::makeIdMap()
{
	iid_uuid_map->clear();

	for (std::map<string, std::shared_ptr<Part>>::iterator iterator = part_map->begin(); iterator != part_map->end(); iterator++) {
		iid_uuid_map->insert(std::pair<std::string, std::string>(iterator->second->getId(), iterator->first));
	}
}


void Body::buildPartList(std::vector<GuiObjectLink*>* list, Part* p, int depth)
{
	//debug_print("Depth: %i, Part: %s \n", depth, p->getId());
	if (p->getType() == PartType::TYPE_ORGAN)
	{
		//Append to end of list "gui_list_indent_char [times depth] ORGAN_NAME"
		std::string str = "";

		for (int i = 0; i <= depth; i++)
		{
			str.append(gui_list_indent_char);
		}

		str.append(p->getName());

		list->push_back(
			new GuiObjectLink(
			p->getUUID(),
			new ColoredText(str, part_gui_list_color_organ)
			)
			);

		return;
	}
	if (p->getType() == PartType::TYPE_BODYPART)
	{
		//Append to end of list "gui_list_indent_char [times depth] BODYPART_NAME"
		BodyPart *bp = (BodyPart*)p;
		std::string str = "";

		for (int i = 0; i <= depth; i++)
		{
			str.append(gui_list_indent_char);
		}

		str.append(bp->getName());

		list->push_back(
			new GuiObjectLink(
			bp->getUUID(),
			new ColoredText(str, part_gui_list_color_bodypart)
			)
			);

		//Call this function on all children of the BodyPart
		for (std::vector<string>::iterator it = bp->getChildListRW()->begin(); it != bp->getChildListRW()->end(); it++)
		{
			std::shared_ptr<Part> part = getPartByUUID(*it);
			if (part == nullptr) { continue; }
			buildPartList(list, part.get(), depth + 1);
		}

		bp = nullptr;

		return;
	}

	debug_error("ERROR: Tried to call recursive part list building function on invalid Part* (neither TYPE_BODYPART nor TYPE_ORGAN)!\n");
	return;
}

void Body::refreshLists()
{
	makeIdMap();
	part_gui_list->clear();
	buildPartList(part_gui_list, root.get());
}

void Body::removePart(std::string part_uuid) {

	std::shared_ptr<Part> part = getPartByUUID(part_uuid);
	if (part == nullptr)
	{
		return;
	}

	debug_print("Removing Part %s...\n", part->getId().c_str());

	std::vector<string>* rem_list = new std::vector<string>();

	makeDownstreamPartList(part_uuid, rem_list);
	BodyPart* super = static_cast<BodyPart*>(getPartByUUID(part->getSuperPartUUID()).get());

	//If Part is an Organ, remove it from its connector
	if (part->getType() == TYPE_ORGAN){
		Organ* o = static_cast<Organ*>(part.get());
		Organ* connector = static_cast<Organ*>(getPartByUUID(o->getConnectorUUID()).get());
		connector->removeConnectedOrgan(part_uuid);
	}
	std::vector<string>* temp_super_child_list = super->getChildList();
	for (std::vector<string>::iterator it = temp_super_child_list->begin(); it != temp_super_child_list->end(); it++)
	{
		//Check if any of the IDs in the kill list are in the child list of the super-part as well 
		// (this is the case for organs and their connectees) and remove them.
		for (auto it_rl = rem_list->begin(); it_rl != rem_list->end(); it_rl++)
		{
			if (it->compare(*it_rl) == 0)
			{
				super->removeChild(*it);
			}
		}

		// Also check wether any of the IDs in the kill list are in the connectedOrgans list of any
		// of the organs of the super-part.
		if (getPartByUUID(*it)->getType() == TYPE_ORGAN)
		{
			Organ *o = static_cast<Organ*>(getPartByUUID(*it).get());
			std::vector<string>* temp_connected_list = o->getConnectedOrgans();

			for (auto it_o = temp_connected_list->begin(); it_o != temp_connected_list->end(); it_o++)
			{
				for (auto it_rl = rem_list->begin(); it_rl != rem_list->end(); it_rl++)
				{
					if (it_o->compare(*it_rl) == 0)
					{
						o->removeConnectedOrgan(*it_o);
					}
				}
			}
		}
	}


	//Remove the chosen part from its super Part.
	super->removeChild(part_uuid);

	//Unregister the Part
	unregisterParts(rem_list);

	//Clear the part variable. This should cause the last use of the shared pointer
	// to the part to be freed, therefore destroying the part.
	//The destructor of the derived class should unregister all its children from the part_map,
	// therefore causing their destruction as well.
	part.reset();

	refreshLists();

#ifdef _DEBUG
	printBodyMap("body_mt.gv", root.get());
#endif

	debug_print("done.\n");
}

void Body::makeDownstreamPartList(string part_uuid, std::vector<string>* child_list)
{
	std::shared_ptr<Part> part = getPartByUUID(part_uuid);
	if (part == nullptr)
	{
		return;
	}

	//Part is an Organ: Add all connected organs to the list
	if (part->getType() == TYPE_ORGAN){
		Organ* o = static_cast<Organ*>(part.get());
		for (auto it = o->getConnectedOrgansRW()->begin(); it != o->getConnectedOrgansRW()->end(); it++)
		{
			child_list->push_back(*it);
			makeDownstreamPartList(*it, child_list);
		}
	}

	//Part is a BodyPart: Add all children to the list and call this function on them
	if (part->getType() == TYPE_BODYPART)
	{
		BodyPart* bp = static_cast<BodyPart*>(part.get());
		for (auto it = bp->getChildListRW()->begin(); it != bp->getChildListRW()->end(); it++)
		{
			child_list->push_back(*it);
			makeDownstreamPartList(*it, child_list);
		}
	}
}

void Body::removeParts(std::vector<string>* part_uuids)
{
	//Copy into new vector, because given vector gets changed by removePart function
	std::vector<string>* rem_list = new std::vector<string>(part_uuids->begin(), part_uuids->end());

	for (std::vector<string>::iterator it = rem_list->begin(); it != rem_list->end(); it++)
	{
		removePart(*it);
	}

	delete rem_list;
}

void Body::removeRandomPart() {
	/*
	debug_print("Remove Random Part from Body:\n");

	//choose one randomly from the list
	Part* random_part = NULL;
	srand(time(NULL));
	int stopid = rand() % part_map->size();
	debug_print("Random element chosen: Nr. %i\n", stopid);
	int it_count = 0;

	for (part_map_iterator iterator = part_map->begin(); iterator != part_map->end(); iterator++) {
		if (it_count == stopid)
		{
			random_part = iterator->second;
			break;
		}
		it_count++;
	}

	if (random_part == NULL)
	{
		debug_error("No element %i.\n", stopid);
		debug_print("Remove Random Part from Body failed.\n");
		return;
	}

	debug_print("Chose %s.\n", random_part->getId().c_str());
	if (random_part->getId() == "ROOT") { return; }
	removePart(random_part->getUUID());
	return;
	*/
}

void Body::unregisterPart(string uuid)
{
	auto it = part_map->find(uuid);

	if (it == part_map->end())
	{
		debug_error("ERROR: No Part with UUID %s could be found for unregister!\n", uuid.c_str());
		return;
	}
	
	part_map->erase(it);
}

void Body::unregisterParts(std::vector<string>* uuids)
{
	for (auto it = uuids->begin(); it != uuids->end(); it++)
	{
		unregisterPart(*it);
	}
}

std::shared_ptr<Part> Body::getPartByUUID(std::string uuid)
{
	if (part_map->count(uuid) == 0) 
	{ 
		debug_error("ERROR: No Part with UUID %s found!\n", uuid.c_str());
		return nullptr; 
	}
	return part_map->at(uuid);
}

std::shared_ptr<Part> Body::getPartByIID(std::string iid)
{
	if (iid_uuid_map->count(iid) == 0)
	{ 
		debug_error("ERROR: No Part with IID %s found!", iid.c_str());
		return nullptr;
	}
	return getPartByUUID(iid_uuid_map->at(iid));
}


void Body::printBodyMap(const char* filename, BodyPart* mroot) {
	std::ofstream file;
	file.open(filename);

	file << "digraph G {" << "\n";

	std::cout << "\nCreating Subgraphs...";
	createSubgraphs(&file, mroot);
	std::cout << "done.\n";

	file << "\n";

	std::cout << "Creating Links...";
	createLinks(&file, mroot);
	std::cout << "done.\n";

	file << "}" << "\n";

	file.close();
}


void Body::createSubgraphs(std::ofstream* stream, BodyPart* bp) {

	*stream << "\t" << "subgraph cluster_" << bp->getId() << " {" << "\n";

	*stream << "\t\t" << "label = \"" << bp->getName() << "\";\n";

	for (std::vector<string>::iterator iterator = bp->getChildListRW()->begin(); iterator != bp->getChildListRW()->end(); iterator++){
		std::shared_ptr<Part> part = getPartByUUID(*iterator);
		if (part == nullptr) { continue; }

		if(part->getType() == TYPE_BODYPART){
			createSubgraphs(stream, static_cast<BodyPart*>(part.get()));

		} else if (part->getType() == TYPE_ORGAN) {
			Organ* o = static_cast<Organ*>(part.get());
			*stream << "\t\t" << o->getId() << " [label=\"" << o->getName() << "\"];" << "\n";

		}
	}

	*stream << "\t } \n";
}

void Body::createLinks(std::ofstream* stream, BodyPart* bp) {
	for (std::vector<string>::iterator iterator = bp->getChildListRW()->begin(); iterator != bp->getChildListRW()->end(); iterator++){
		std::shared_ptr<Part> part = getPartByUUID(*iterator);
		if (part == nullptr) { continue; }

		if(part->getType() == TYPE_BODYPART){
			createLinks(stream, static_cast<BodyPart*>(part.get()));
		} else if (part->getType() == TYPE_ORGAN) {
			Organ* o = static_cast<Organ*>(part.get());
			if (!o->getConnectorId().empty()) {
				*stream << o->getConnectorId() << " -> " << o->getId() << ";\n";
			}
		}
	}
}