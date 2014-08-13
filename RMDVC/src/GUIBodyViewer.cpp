#include "GUI.hpp"

GuiBodyViewer::GuiBodyViewer(string id, int x, int y,
	int width, int height, TCODColor fore, TCODColor back,
	bool drawBorder, string title) :
	GuiContainer(x, y, width, height, fore, back, false, drawBorder, title)
{
	//BodyPartViewer has three parts: the BP browser ListChooser,
	//the BP info TextBox and the Tissue Browser ListChooser

	//Create ObjectLink lists
	part_list = new TCODList<GuiObjectLink*>();
	tissue_list = new TCODList<GuiObjectLink*>();

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


	bp_info->setVisibility(false);
	tissue_browser->setVisibility(false);

	//Void active body pointer
	body = nullptr;
}

GuiBodyViewer::~GuiBodyViewer()
{
	//Destroy ObjectLink lists
	part_list->clearAndDelete();
	tissue_list->clearAndDelete();
	delete part_list;
	delete tissue_list;

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


void GuiBodyViewer::render(TCODConsole* con) 
{
	GuiContainer::render(con);
}