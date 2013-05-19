/*
 * Body.cpp
 *
 *  Created on: 19 May 2013
 *      Author: Clemens
 */

#include "Body.hpp"

Tissue::Tissue(const char *id, const char *name, float pain,
		float blood_flow, float resistance, float impairment) :
		id(id),name(name),pain(pain),blood_flow(blood_flow),resistance(resistance),
		impairment(impairment){

}

Part::Part(const char *id, const char *name, float surface):
	id(id), name(name), surface(surface){

}

BodyPart::BodyPart(const char *id, const char *name, float surface):
		Part(id, name, surface){
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

bool BodyPart::removeChild(char *id){
	//Iterate trough the list of (pointers to) the child parts.
	for (Part** it = children->begin(); it != children->end(); it++)
	{
		Part *part = *it;
		//If id's match
		if (strcmp(part->id, id) == 0){
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
		tissue_def *tissues, int tissue_count, bool is_root):
		Part(id, name, surface), tissues(tissues), tissue_count(tissue_count),
		root(is_root), connector(NULL){

}

void Organ::linkConnector(Organ *connector){
	if (connector){
		this->connector = connector;
	}
}

Body::Body(const char *filename){
	root = NULL;
	if (loadBody(filename))
	{
		std::cout << "SUCCESS!";
	}
}

Body::~Body(){

}

bool Body::loadBody(const char *filename){

	//###XML FILE HANDLING###
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
	using namespace rapidxml;

	xml_document<> doc;
	doc.parse<0>(buffer);


	//###TISSUE DATA###

	//Load the tissue data (First first_node() is "body_def",
	// data starts with second level "tissues")
	xml_node<> *tissues = doc.first_node()->first_node("tissues");

	//Temporary tissue map, list & variables
	std::map<std::string, Tissue*> tissue_map;

	char *id, *name;
	float pain, blood_flow, resistance, impairment;

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
			char *_name = attr->name();

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
		tissue_map[std::string(id)] = new Tissue(id, name, pain, blood_flow, resistance, impairment);
	}

	//###BODYPART DATA###


	delete[] buffer;

	} catch (rapidxml::parse_error& e) {
			std::cerr << "Error while parsing body definition file: " << e.what() << "\n";
			return false;
	}

	return false;
}


