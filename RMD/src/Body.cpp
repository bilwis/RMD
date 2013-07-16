/*
 * Body.cpp
 *
 *  Created on: 19 May 2013
 *      Author: Clemens
 */

#include "Body.hpp"

bdef_parse_error::bdef_parse_error(const char* msg, rapidxml::xml_node<>* node):msg(msg), node(node){}

Tissue::Tissue(const char *id, const char *name, float pain,
		float blood_flow, float resistance, float impairment) :
		id(id),name(name),pain(pain),blood_flow(blood_flow),resistance(resistance),
		impairment(impairment){

}

Part::Part(const char *id, const char *name, float surface, PartType type):
	id(id), name(name), surface(surface), type(type){

}

Part::~Part()
{

}

BodyPart::BodyPart(const char *id, const char *name, float surface):
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
}

void BodyPart::addChildren(Part **child_array, int count){
	for (int i=0; i < count; i++){
		children->push(child_array[i]);
	}
}

TCODList<Part*>* BodyPart::getChildList()
{
	return children;
}

bool BodyPart::removeChild(char *id){
	//Iterate trough the list of (pointers to) the child parts.
	for (Part** it = children->begin(); it != children->end(); it++)
	{
		Part *part = *it;
		//If id's match
		if (strcmp(part->getId(), id) == 0){
			//Destroy the Part and remove the pointer from the list.
			// Why destroy? Because the pointer in the children list of the parent
			// is the only reference to it, so it must be destroyed before being
			// removed from that list!
			delete part;
			children->removeFast(it);
			return true;
		}
	}

	return false;
}

Organ::Organ(const char *id, const char *name, float surface,
		tissue_def *tissues, int tissue_count, const char *connector_id, bool is_root):
		Part(id, name, surface, TYPE_ORGAN), tissues(tissues), tissue_count(tissue_count),
		connector_id(connector_id), root(is_root), connector(NULL){
	connected_organs = new TCODList<Organ*>();
}

Organ::~Organ(){
	connected_organs->clearAndDelete();
	delete connected_organs;
}

void Organ::linkToConnector(std::map<std::string, Organ*, strless> *organ_map){
	//Check if the Organ is the root, if yes, exit
	if (!strcmp(connector_id, "_ROOT")) { return; }

	//Find the connector in the organ map
	std::map<std::string, Organ*>::const_iterator pos = organ_map->find(connector_id);

	//If not found, error
	if (pos == organ_map->end()){ throw std::exception(); }

	//Otherwise store pointer to connector in connector variable
	connector = pos->second;

	//Add this organ to the root's list of branches
	connector->addConnectedOrgan(this);

	std::cout << "\nLINKED " << id << " to connector " << pos->first;
}

const char* Organ::getConnectorId() {
	if (connector == NULL) { return NULL; }
	return connector->getId();
}

void Organ::addConnectedOrgan(Organ *connectee){
	connected_organs->insertBefore(connectee, 0);
}

Body::Body(const char *filename){
	root = NULL;
	if (loadBody(filename))
	{
		printBodyMap("body.gv");
		std::cout << "Printed Body Map!\n";
	}
}

Body::~Body(){

}

bool Body::loadBody(const char *filename){

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

	char *id, *name;
	float pain, blood_flow, resistance, impairment;

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

		std::cout << "Read Tissue:\n\tID: " << id << "\n\tName: " << name
				<< "\n\tBlood Flow: " << blood_flow << "\n\tResistance: " << resistance
				<< "\n\tImpairment: " << impairment << "\n";

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
	root = enter(body->first_node(), &tissue_map);

	//go through all of the parsed data and make a list with pointers to the organs
	std::map<std::string, Organ*, strless>* organ_map = new std::map<std::string, Organ*, strless>();
	organ_map = extractOrgans(root, organ_map);

	/*DEBUG: Print all objects in the organ map
	std::map<std::string, Organ*>::iterator iter;
	for (iter = organ_map->begin(); iter != organ_map->end(); ++iter){
		std::cout << "\n" << iter->first << " = " << iter->second->getName();
	}
	*/

	//go through all of the parsed data and link the organ connections
	// (that is, store the pointer to the connector as identified by
	// "connector_id" in the connector variable)
	linkOrgans(root, organ_map);

	//Organ map is no longer needed
	delete organ_map;

	//Destroy the XML data in memory
	delete[] buffer;

	//Successfully parsed the body definition file!
	return true;

	} catch (std::exception& e) {
			std::cerr << "ERROR: " << e.what() << "\n";
			return false;
	}

	return false;
}

std::map<std::string, Organ*, strless>* Body::extractOrgans(BodyPart* bp,
		std::map<std::string, Organ*, strless> *organ_map) {

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
	char *id, *name, *_name;
	float surface;

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
	bp = new BodyPart(id, name, surface);
	id = NULL; name = NULL;

	//DEBUG: Print new BodyPart
	std::cout << "\nNew BodyPart created: \n"
			<< "\t ID: " << bp->getId()
			<< "\n\t Name: " << bp->getName()
			<< "\n\t Surface: " << bp->getSurface();

	//If there are no body_part nodes...
	if (bodyparts == 0){
		//...there must be organs instead!
		// variables
		Part **organs;
		xml_attribute<> *attr;
		xml_node<> *tdef_node;

		char *connector;
		bool symmetrical = false;

		it = 0;

		//  temp vars for tissue definitions
		char *tdef_id, *tdef_custom_id, *tdef_name;
		float tdef_hit_prob;

		tissue_def *tdefs;
		int tdef_count, tdef_it;


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
						//The value of a tissue_def node is the id of the tissue
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
						tdefs[tdef_it].custom_id = tdef_custom_id;
						tdefs[tdef_it].name = tdef_name;
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

			//Check whether all necessary data for organ creation has been read+
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
			organs[i] = new Organ(id, name, surface, tdefs, tdef_count, connector, !strcmp(connector, "_ROOT"));

			//DEBUG: Print Organ
			std::cout << " \n\tNew Organ created: \n"
					<< "\t\t ID: " << organs[i]->getId()
					<< "\n\t\t Name: " << organs[i]->getName()
					<< "\n\t\t Surface: " << organs[i]->getSurface()
					<< "\n\t\t Root?: " << connector
					<< "\n\t\t Tissues:";

			for (int di = 0; di < (tdef_count * (symmetrical+1))-1; di++){
				std::cout << "\n\t\t\t Base Tissue Name: "
						<< tdefs[di].tissue->getName()
						<< "\n\t\t\t\t Hit Prob.: " << tdefs[di].hit_prob;
				if (tdefs[di].name != NULL){ std::cout << "\n\t\t\t\t Custom Name: " << tdefs[di].name; }
				if (tdefs[di].custom_id != NULL){ std::cout << "\n\t\t\t\t Custom ID: " << tdefs[di].custom_id; }
			}

			std::cout << "\n\tEND Organ.\n";
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
	TCODList<Part*>* templ = bp->getChildList();

	std::cout << "\n\t Parts:";

	for (Part** iterator = templ->begin(); iterator != templ->end(); iterator++){
		Part* p = *iterator;

		std::cout << "\n\t\t Part Name: "
			<< p->getName();
	}

	std::cout << "\nEND BodyPart.\n";

	//return this bodypart
	return bp;
}

void Body::removeRandomBodyPart() {
	//TODO: Implement

	//iterate through the bodyparts and make a list

	//choose one randomly from the list

	//iterate again through all bodyparts and remove the
	// chosen part
}

void Body::printBodyMap(const char* filename) {
	std::ofstream file;
	file.open(filename);

	file << "digraph G {" << "\n";

	createSubgraphs(&file, root);
	file << "\n";
	createLinks(&file, root);

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
			if (o->getConnectorId() != NULL) {
				*stream << o->getConnectorId() << " -> " << o->getId() << ";\n";
			}
		}
	}
}

