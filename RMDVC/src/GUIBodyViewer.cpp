#include "GUI.hpp"
#include "Body.hpp"

GuiBodyViewer::GuiBodyViewer(string id, int x, int y,
	int width, int height, TCODColor fore, TCODColor back,
	bool drawBorder, string title) :
	GuiContainer(x, y, width, height, fore, back, false, drawBorder, title)
{
	//BodyPartViewer has three parts: the BP browser ListChooser,
	//the BP info TextBox and the Tissue Browser ListChooser

	//Create ObjectLink lists
	//part_list = new std::vector<GuiObjectLink*>();
	//tissue_list = new std::vector<GuiObjectLink*>();

	//Declare GUI Parts
	//TODO: Make GuiListChooser scrollable, set active element!
	bp_browser = new GuiListChooser("listChooser_BodyViewer_BPBrowser", 
		1, 1,
		(int)(width / 2), height - 2,
		height - 2, 
		gui_default_fore, gui_default_back,
		gui_sel_fore_act, gui_sel_back_act, gui_sel_fore_inact, gui_sel_back_inact);

	bp_info = new GuiTextBox("textBox_BodyViewer_BPInfo", (int)(width / 2) + 1, y + 1,
		(int)(width / 2), (int)(height / 2), "", gui_default_fore, gui_default_back);

	tissue_browser = new GuiListChooser("listChooser_BodyViewer_TissueBrowser",
		(int)(width / 2) + 1, (int)(height / 2) + 1,
		(int)(width / 2), (int)(height / 2),
		(int)(height/2) - 1,
		gui_default_fore, gui_default_back,
		gui_sel_back_act, gui_sel_back_act, gui_sel_fore_inact, gui_sel_back_inact);

	addElement(bp_browser);
	addElement(bp_info);
	addElement(tissue_browser);

	tissue_browser->setVisibility(false);

	//Void active body pointer
	body = nullptr;
}

GuiBodyViewer::~GuiBodyViewer()
{
	//Destroy ObjectLink lists
	//TODO: Remove GuiObjectLink elements?
	//delete part_list;
	//delete tissue_list;
	part_list = nullptr;
	tissue_list = nullptr;

	//Destroy GUI Parts
	delete bp_browser;
	delete bp_info;
	delete tissue_browser;
}

void GuiBodyViewer::setActiveBody(Body* b)
{
	if (b != nullptr) { body = b; }

	//Fill BP_Browser with Parts
	part_list = b->getPartGUIList();
	bp_browser->addItems(part_list);
}

void GuiBodyViewer::activate(Body* b)
{
	bp_browser->reset();
	setActiveBody(b);
	bp_browser->makeActive();
	active_element = bp_browser;
	
	update(TCOD_console_check_for_keypress(0));
}

void GuiBodyViewer::update(TCOD_key_t key)
{
	active_element->update(key);

	if (active_element == bp_browser)
	{
		//Retrieve currently selected Part from bp_browser
		std::vector<std::string>* p_uuid = new std::vector<std::string>();
		if (bp_browser->getSelected(p_uuid) != 1)
		{
			debug_error("ERROR: Expected only one string from GuiListChooser->getSelected(x)!");
			return;
		}

		if (p_uuid->size() == 0)
		{
			debug_error("ERROR: Expected a string from GuiListChooser->getSelected(x), but returned vector is empty!");
			return;
		}

		std::string uuid = p_uuid->at(0);

		Part* p = body->getPartByUUID(uuid).get();
		if (p == nullptr) 
		{ 
			debug_error("ERROR: No Part could be resolved from UUID %s.", uuid.c_str());
			return;
		}

		//Use Part to display info on bp_info and tissue_browser
		
		//bp_info contains ALWAYS:
		// Name: 
		//bp_info contains FOR ORGANS:
		// BodyPart: [BP this organ is part of]
		// Surface Area:
		// Connector: 
		// Connectees: 

		std::string temp_info;
		temp_info.append("Type: ");
		temp_info.append(part_type_strings[p->getType()]);
		temp_info.append("\n\nName: ");
		temp_info.append(p->getName().c_str());

		if (p->getType() == PartType::TYPE_ORGAN)
		{
			
			Organ* o = (Organ*)p;
			temp_info.append("\n\nBodyPart: ");
			temp_info.append(body->getPartByUUID(o->getSuperPartUUID())->getName().c_str());
			temp_info.append("\nSurface Area: ");
			temp_info.append(std::to_string((int)(o->getSurface() * 100)));
			
			temp_info.append("%%\nConnector: ");
			if (o->getConnectorUUID() != "") {
				temp_info.append(body->getPartByUUID(o->getConnectorUUID())->getName().c_str());
			}
			else {
				temp_info.append("None [Root Organ]");
			}
			temp_info.append("\nConnected Organs:");

			//Add Connectees (derive from UUID list returned by o->getConnectedOrganUUIDs())
			std::vector<std::string>* temp = o->getConnectedOrgansRW();

			for (std::vector<std::string>::iterator it = temp->begin(); it != temp->end(); it++)
			{
				if (*it == "") { continue; }
				temp_info.append("\n  ");
				temp_info.append(body->getPartByUUID(*it)->getName().c_str());
			}

			if (o->isStump()){
				temp_info.append("\n\nOrgan is a stump.");
			}

		}
		
		bp_info->setText(temp_info);

		//Handle "special" debug input
#ifdef _DEBUG
		if (key.vk == TCODK_DELETE){ 
			body->removePart(uuid); 
			activate(body);
		}
		
#endif
		//Cleanup
		delete p_uuid;
	}
}


void GuiBodyViewer::render(TCODConsole* con) 
{
	renderSelf(con);

	bp_browser->render(container_console);
	bp_info->render(container_console);
	tissue_browser->render(container_console);

	TCODConsole::blit(container_console, 0, 0, 0, 0, con, pos_x, pos_y);
}