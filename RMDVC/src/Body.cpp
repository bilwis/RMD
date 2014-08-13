/*
 * Body.cpp
 *
 *  Created on: 19 May 2013
 *      Author: Clemens
 */

#include "Body.hpp"

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

Part::Part(string id, string name, float surface, PartType type) : Object()
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

	super = NULL;
}

Part::~Part()
{
}

BodyPart::BodyPart(string id, string name, float surface) :
		Part(id, name, surface, TYPE_BODYPART){

	children = new TCODList<Part*>();
}

BodyPart::~BodyPart()
{
	children->clearAndDelete();
	delete children;
}

void BodyPart::addChild(Part *child){
	children->insertBefore(child, 0);
	child->setSuperPart(this);
}

void BodyPart::addChildren(Part **child_array, int count){
	for (int i=0; i < count; i++){
		children->push(child_array[i]);
		child_array[i]->setSuperPart(this);
	}
}

TCODList<Part*>* BodyPart::getChildList()
{
	return children;
}

bool BodyPart::removeChild(string id){
	//Iterate though the list of children and remove it
	for (Part** it = children->begin(); it != children->end(); it++)
	{
		Part *part = *it;
		if (id.compare(part->getId()) == 0){
			children->removeFast(it);
			return true;
		}
	}

	return false;
}

Organ::Organ(string id, string name, float surface,
		tissue_def *tissues, int tissue_count, const char *connector_id, bool is_root):
		Part(id, name, surface, TYPE_ORGAN), tissues(tissues), tissue_count(tissue_count),
		root(is_root), connector(NULL){

	/* The XML parsing function works with pointers to strings only.
	 * Every "id", "name" and any other string is only copied into memory ONCE, and that
	 * is during the initial call of rapidxml.parse. All subsequent references to the variables
	 * are pointers to this initial memory location. The rapidxml memory buffer is wiped
	 * at the end of the loadBody function, however. Therefore all new instances of the Part
	 * or Tissue class, which hold the strings for later use in the program, must ACTUALLY COPY
	 * THE MEMORY to which the "parsing pointer" points and maintain a reference to it via their
	 * own id and name (etc.) variables, which are pointers to the new, copied memory slot.
	 */

	this->connector_id = string(connector_id);
	connected_organs = new TCODList<Organ*>();
}

Organ::~Organ(){
	//The organ destructor automatically removes it from the child
	//list of it's BodyPart. It also calls the destructor on all of its
	//own children.

	debug_print("Organ %s is kill.\n", this->id.c_str());

	BodyPart* bp = static_cast<BodyPart*>(super);
	bp->removeChild(this->id);
	connected_organs->clearAndDelete();
	delete connected_organs;

}

void Organ::linkToConnector(std::map<std::string, Organ*, strless> *organ_map){
	//Check if the Organ is the root, if yes, exit
	if (!connector_id.compare("_ROOT")) { return; }

	//Find the connector in the organ map
	std::map<std::string, Organ*>::const_iterator pos = organ_map->find(connector_id);

	//If not found, error
	if (pos == organ_map->end()){ throw std::exception(); }

	//Otherwise store pointer to connector in connector variable
	connector = pos->second;

	//Add this organ to the root's list of branches
	connector->addConnectedOrgan(this);

	debug_print("LINKED %s to connector %s\n", id.c_str(), pos->first.c_str());
}

string Organ::getConnectorId() {
	if (connector == NULL) { return ""; }
	return connector->getId();
}

void Organ::addConnectedOrgan(Organ *connectee){
	connected_organs->insertBefore(connectee, 0);
}

Body::Body(const char *filename){
	root = loadBody(filename);
	if (root != NULL)
	{
		//go through all of the parsed data and make a list with pointers to the organs
		std::map<std::string, Organ*, strless>* organ_map = new std::map<std::string, Organ*, strless>();
		organ_map = extractOrgans(root, organ_map);

		//go through all of the parsed data and link the organ connections
		// (that is, store the pointer to the connector as identified by
		// "connector_id" in the connector variable)
		linkOrgans(root, organ_map);

		//Organ map is no longer needed
		delete organ_map;

		//go through all of the parsed data and make a list with pointers to every part of the Body
		part_map = new std::map<std::string, Part*, strless>();
		part_map = extractParts(root, part_map);

		//create the formatted and linked part list for the GUI
		part_gui_list = new TCODList<GuiObjectLink*>();
		buildPartList(part_gui_list, root, 0);

#ifdef _DEBUG
		printBodyMap("body.gv", root);
		debug_print("Printed Body Map! \n");
#endif

	}
	else {
		TCODConsole::root->setDefaultForeground(TCODColor::lightRed);
		TCODConsole::root->print(10, 10, "ERROR WHILE CREATING ACTOR BODY - Press any key to continue.");
		TCODConsole::root->print(10, 11, "For detailed error check console window.");
		TCODConsole::root->flush();
		TCODConsole::root->waitForKeypress(true);
	}
}

Body::~Body(){

}

BodyPart* Body::loadBody(const char *filename){

	BodyPart* temproot;

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

	//Temporary tissue map, list & variables
	std::map<std::string, Tissue*, strless> tissue_map;

	char *id = NULL, *name = NULL;
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

		//Create a new Tissue and store the pointer to it in the tissue map
		tissue_map.insert(std::pair<std::string, Tissue*>(std::string(id), new Tissue(id, name, pain, blood_flow, resistance, impairment)));
	}

	//###BODYPART DATA###

	//variables needed for init
	id = NULL;
	name = NULL;

	//Load the bodypart data (First first_node() is "body_def",
	// data starts with the second level "body"
	xml_node<> *body = doc.first_node()->first_node("body");

	//recursively load the data
	temproot = enter(body->first_node(), &tissue_map);

	//Destroy the XML data in memory
	delete[] buffer;

	//Successfully parsed the body definition file!
	//Return the address of the newly created body part.
	return temproot;

	} catch (std::exception& e) {
		std::cerr << "ERROR: " << e.what() << "\n";
		return NULL;
	}
	catch (bdef_parse_error& pe) {
		std::cerr << "ERROR: " << pe.what() << "\n";
		
		return NULL;
	}

	return NULL;
}

std::map<std::string, Organ*, strless>* Body::extractOrgans(BodyPart* bp,
		std::map<std::string, Organ*, strless> *organ_map) {
	//Iterate through the children of the given BodyPart, if it is an Organ,
	//add it to the organ_map; if it is a BodyPart, recursively call this function on it.

	TCODList<Part*>* templ = bp->getChildList();
	for (Part** iterator = templ->begin(); iterator != templ->end(); iterator++){
		Part* p = *iterator;

		if(p->getType() == TYPE_BODYPART){
			organ_map = extractOrgans(static_cast<BodyPart*>(p), organ_map);
		} else if (p->getType() == TYPE_ORGAN) {
			organ_map->insert(std::pair<std::string, Organ*>(std::string(p->getId()), static_cast<Organ*>(p)));
		}
	}

	return organ_map;
}

void Body::linkOrgans(BodyPart* bp, std::map<std::string, Organ*, strless> *organ_map){
	//Iterate through the children of the given BodyPart, if it is an Organ,
	//call its linkToConnector function with the organ_map parameter;
	//if it is a BodyPart, recursively call this function on it.

	TCODList<Part*>* templ = bp->getChildList();
	for (Part** iterator = templ->begin(); iterator != templ->end(); iterator++){
		Part* p = *iterator;

		if(p->getType() == TYPE_BODYPART){
			linkOrgans(static_cast<BodyPart*>(p), organ_map);
		} else if (p->getType() == TYPE_ORGAN) {
			static_cast<Organ*>(p)->linkToConnector(organ_map);
		}
	}
}

BodyPart* Body::enter(rapidxml::xml_node<> *node, std::map<std::string, Tissue*, strless> *tissue_map) {
	using namespace rapidxml;

	int organ_count, bodyparts, it;
	xml_node<> *temp;
	xml_node<> **organ_node_list;

	BodyPart *bp;
	char *id = NULL, *name = NULL, *_name = NULL;
	float surface = 0.0f;

	//Make a count of the body_part nodes in this node and parse all standard
	// nodes for the bodypart of this node
	temp = node->first_node();
	bodyparts = 0;
	while(temp != NULL){
		_name = temp->name();

		if (!strcmp(_name, "body_part")) { bodyparts++; }

		if (!strcmp(_name,"id")) { id = temp->value(); }
		if (!strcmp(_name,"name")) { name = temp->value(); }
		if (!strcmp(_name,"surface")) { surface = atof(temp->value()); }


		temp = temp->next_sibling();
	}

	//if any of the mandatory vars for bodyparts are NULL, ERROR!
	if (id == NULL || name == NULL){
		throw new bdef_parse_error("Not all mandatory BodyPart variables defined!", node);
	}

	//make the bodypart, reset temporary variables for reuse with the organs
	bp = new BodyPart((string)id, (string)name, surface);
	id = NULL; name = NULL;

	//DEBUG: Print new BodyPart
	debug_print("New BodyPart created: \n\tID: %s \n\tName: %s \n\tSurface: %f\n",
			bp->getId().c_str(), bp->getName().c_str(), bp->getSurface());

	//If there are no body_part nodes...
	if (bodyparts == 0){
		//...there must be organs instead!
		// variables
		Part **organs;
		xml_attribute<> *attr;
		xml_node<> *tdef_node;

		char *connector = NULL;
		bool symmetrical = false;

		it = 0;

		//  temp vars for tissue definitions
		char *tdef_id, *tdef_custom_id, *tdef_name;
		float tdef_hit_prob;

		tissue_def *tdefs = NULL;
		int tdef_count = 0, tdef_it = 0;


		//Reset back to the first node in the given node
		temp = node->first_node();

		//scan for organ nodes in the given node
		organ_count = 0;
		while(temp != NULL){
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

		while(temp != NULL){
			_name = temp->name();

			if (!strcmp(_name,"organ")) {
				organ_node_list[it] = temp;
				it++;
			}

			temp = temp->next_sibling();
		}

		//create temporary organ array
		organs = new Part*[organ_count];

		//parse each organ definition in the list
		for (int i=0; i < organ_count; i++){
			//Enter into organ node
			temp = organ_node_list[i]->first_node();

			//Iterate through all nodes within the organ node
			while(temp != NULL){
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
					if (attr != NULL && !strcmp(attr->name(), "symmetrical")){
						symmetrical = true;
					}

					attr = NULL;

					//Get the count of tissue defs
					tdef_count = 0;
					tdef_node = temp->first_node();

					while (tdef_node != NULL)
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
					while (tdef_node != NULL){
						//The value of a tissue_def node is the id of the tissue (e.g. M_ARTERY)
						tdef_id = tdef_node->value();

						tdef_hit_prob = 0;
						tdef_name = NULL;
						tdef_custom_id = NULL;

						//Get the other parameters
						attr = tdef_node->first_attribute();
						while (attr != NULL){

							if (!strcmp(attr->name(), "hit_prob")){ tdef_hit_prob = atof(attr->value()); }
							if (!strcmp(attr->name(), "name")) { tdef_name = attr->value(); }
							if (!strcmp(attr->name(), "custom_id")) {
								tdef_custom_id = attr->value();
							}

							attr = attr->next_attribute();
						}


						//Store the parameters in the tissue definition
						if (tdef_custom_id != NULL) {
							tdefs[tdef_it].custom_id = string(tdef_custom_id);
						}
						else { tdefs[tdef_it].custom_id = ""; }

						if (tdef_name != NULL) {
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

						std::map<std::string, Tissue*>::const_iterator pos
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
			if (id == NULL || name == NULL || connector == NULL || tdefs == NULL){
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
			organs[i] = new Organ(string(id), string(name), surface, tdefs, tdef_count, connector, !strcmp(connector, "_ROOT"));

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

		//...and add all organs to the bodypart

		bp->addChildren(organs, organ_count);
	} else {
		//Go through all nodes and call this function on every
		// node containing a part definition, add the returned BodyPart/Organ
		// to this ones' children
		temp = node->first_node();

		it = 0;
		while(temp != NULL){
			_name = temp->name();

			if (!strcmp(_name, "body_part")) {
				bp->addChild(enter(temp, tissue_map));
			}

			temp = temp->next_sibling();
		}
	}

	//DEBUG: Print BodyPart Parts
#ifdef _DEBUG
	TCODList<Part*>* templ = bp->getChildList();

	debug_print("\n\t Parts:");

	for (Part** iterator = templ->begin(); iterator != templ->end(); iterator++){
		Part* p = *iterator;

		debug_print("\n\t\t Part Name: %s", p->getName().c_str());
	}

	debug_print("\nEND BodyPart.\n");
#endif

	//return this bodypart
	return bp;
}

void Body::removeRandomPart() {
	debug_print("Remove Random Part from Body:\n");

	//choose one randomly from the list
	Part* random_part = NULL;
	srand(time(NULL));
	int stopid = rand() % part_map->size();
	debug_print("Random element chosen: Nr. %i\n", stopid);
	int it_count = 0;

	for(part_map_iterator iterator = part_map->begin(); iterator != part_map->end(); iterator++) {
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

	removePart(random_part->getId());
	return;
}

std::map<std::string, Part*, strless>* Body::extractParts(BodyPart* bp,
		std::map<std::string, Part*, strless>* part_map) {

	TCODList<Part*>* templ = bp->getChildList();
	for (Part** iterator = templ->begin(); iterator != templ->end(); iterator++){
		Part* p = *iterator;

		part_map->insert(std::pair<std::string, Part*>(std::string(p->getId()), p));
		debug_print("Added to part_map: %s \n", p->getId().c_str());

		if(p->getType() == TYPE_BODYPART){
			part_map = extractParts(static_cast<BodyPart*>(p), part_map);
		}
	}

	return part_map;
}

bool Body::removePart(std::string part_id) {
	Part* part = NULL;
	try{
		part = part_map->at(part_id);
	} catch (int e) {
		debug_error("Part %s does not exist.\n", part_id.c_str());
		return false;
	}

	//Remove the chosen part and all downstream elements

	//Part is an Organ: remove it, destructor handles killing the children
	if (part->getType() == TYPE_ORGAN){
		debug_print("Part %s is type ORGAN, deleting...\n", part->getId().c_str());
		Organ* o = static_cast<Organ*>(part);
		delete o;
		debug_print("done.\n");
	}

	//Part is a BodyPart: make list of all children, remove them
	// and the part itself
	//TODO

	//Refresh GUI part list
	part_gui_list->clear();
	buildPartList(part_gui_list, root, 0);

#ifdef _DEBUG
	//Refresh part map
	part_map->clear();
	part_map = extractParts(root, part_map);

	printBodyMap("body_mt.gv", root);
#endif

	return true;
}

void Body::buildPartList(TCODList<GuiObjectLink*>* list, Part* p, int depth)
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

		list->push(
				new GuiObjectLink(
					p->getUUID(),
					new ColoredText(str)
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

		list->push(
				new GuiObjectLink(
					bp->getUUID(),
					new ColoredText(str)
				)
			);

		//Call this function on all children of the BodyPart
		for (Part** it = bp->getChildList()->begin(); it != bp->getChildList()->end(); it++)
		{
			buildPartList(list, *it, depth + 1);
		}

		bp = nullptr;

		return;
	}

	debug_error("ERROR: Tried to call recursive part list building function on invalid Part* (neither TYPE_BODYPART nor TYPE_ORGAN)!");
	return;
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

	TCODList<Part*>* templ = bp->getChildList();
	for (Part** iterator = templ->begin(); iterator != templ->end(); iterator++){
		Part* p = *iterator;

		if(p->getType() == TYPE_BODYPART){
			createSubgraphs(stream, static_cast<BodyPart*>(p));

		} else if (p->getType() == TYPE_ORGAN) {
			Organ* o = static_cast<Organ*>(p);
			*stream << "\t\t" << o->getId() << " [label=\"" << o->getName() << "\"];" << "\n";

		}
	}

	*stream << "\t } \n";
}

void Body::createLinks(std::ofstream* stream, BodyPart* bp) {
	TCODList<Part*>* templ = bp->getChildList();
	for (Part** iterator = templ->begin(); iterator != templ->end(); iterator++){
		Part* p = *iterator;

		if(p->getType() == TYPE_BODYPART){
			createLinks(stream, static_cast<BodyPart*>(p));
		} else if (p->getType() == TYPE_ORGAN) {
			Organ* o = static_cast<Organ*>(p);
			if (!o->getConnectorId().empty()) {
				*stream << o->getConnectorId() << " -> " << o->getId() << ";\n";
			}
		}
	}
}