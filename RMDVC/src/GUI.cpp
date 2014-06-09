#include "GUI.hpp"

//Commentate

Gui::Gui(){
	containers = new TCODList<GuiContainer*>();
	active_elements = new TCODList<ActiveGuiElement*>();
};
Gui::~Gui(){
	containers->clearAndDelete();
	delete containers;

	active_elements->clearAndDelete();
	delete active_elements;

	delete current_active;
};

void Gui::addContainer(GuiContainer* container){
	containers->push(container);
}

void Gui::registerActiveElement(ActiveGuiElement* element)
{
	active_elements->push(element);
}

//TODO: Add Remove Functions !

void Gui::makeActive(ActiveGuiElement* element)
{
	current_active = element;
	element->makeActive(this);
}

bool Gui::makeActive(string id)
{
	for (ActiveGuiElement** it = active_elements->begin(); it != active_elements->end(); it++)
	{
		ActiveGuiElement *element = *it;
		if (element->getId() == id)
		{
			makeActive(element);
			return true;
		}
	}
	return false;
}

void Gui::update(TCOD_key_t key){
	for (ActiveGuiElement** it = active_elements->begin(); it != active_elements->end(); it++)
	{
		ActiveGuiElement *element = *it;
		if (element == current_active)
		{
			element->update(key);
		} else {
			//TODO: Do you have to update all Active Elements that are not active?
			element->update(key);
		}
	}
}

void Gui::render(TCODConsole* con){
	//Iterate through all GuiContainers, call their render function
	// and blit the result on the given console at the coordinates that 
	// the GuiContainer is set to.
	for (GuiContainer** it = containers->begin(); it != containers->end(); it++)
	{
		GuiContainer *container = *it;
		if (container->isVisible()){
			TCODConsole::blit(container->render(), 0, 0, 0, 0, con,
				container->getPosX(),
				container->getPosY());
		}
	}
}

GuiRenderObject::GuiRenderObject(string id, int x, int y, int width, int height, TCODColor fore, TCODColor back) :
RenderObject(id, x, y, fore, back){
	this->width = width;
	this->height = height;
}

GuiRenderObject::~GuiRenderObject()
{

}

GuiElement::GuiElement(string id, int x, int y, int width, int height, TCODColor fore, TCODColor back, GuiContainer* parent):
GuiRenderObject(id, x, y, width, height, fore, back)
{
	this->parent = parent;
}

GuiElement::~GuiElement(){}

GuiContainer::GuiContainer(string id, int x, int y, int width, int height,
	TCODColor fore, TCODColor back, bool drawBorder, string title)
	:GuiRenderObject(id, x, y, width, height, fore, back){
	elements = new TCODList<GuiElement*>();
	container_console = new TCODConsole(width, height);
	this->drawBorder = drawBorder;
	this->title = string(title);
}

GuiContainer::~GuiContainer()
{
	elements->clearAndDelete();
	delete elements;
	delete container_console;
}

void GuiContainer::addElement(GuiElement* element)
{
	elements->push(element);
	element->setParent(this);
}

const TCODConsole* GuiContainer::render()
{
	if (!visible) { return nullptr; }

	//Set Defaults
	container_console->setDefaultBackground(background_color);
	container_console->setDefaultForeground(foreground_color);

	container_console->setBackgroundFlag(TCOD_BKGND_SET);

	//Draw Border, if requested
	if (drawBorder){
		if (!title.empty())
		{
			container_console->printFrame(0, 0, width, height, true,
				TCOD_BKGND_SET, title.c_str());
		} else {
			container_console->printFrame(0, 0, width, height, true,
				TCOD_BKGND_SET);
		}
	}
	else {
		container_console->rect(0, 0, width, height, true);
	}

	//Iterate through GuiElements, render them to container_console
	//TODO: If necessary, implement z-ordering.
	for (GuiElement** it = elements->begin(); it != elements->end(); it++)
	{
		GuiElement *element = *it;
		element->render(container_console);
	}
	return container_console;
}

GuiTextBox::GuiTextBox(string id, int x, int y, int width, int height, string text, TCODColor fore, TCODColor back, TCOD_alignment_t alignment)
	:GuiElement(id, x, y, width, height, fore, back, nullptr){
	this->text = text;
	this->alignment = alignment;
}

GuiTextBox::~GuiTextBox()
{

}

void GuiTextBox::render(TCODConsole* con){
	if (!visible) { return; }

	con->setDefaultBackground(background_color);
	con->setDefaultForeground(foreground_color);
	con->setAlignment(alignment);
	con->printRect(pos_x, pos_y, width, height, text.c_str());
}

ActiveGuiElement::ActiveGuiElement(string id, int x, int y, int width, int height,
	TCODColor fore, TCODColor back,
	TCODColor sel_fore_act, TCODColor sel_back_act,
	TCODColor sel_fore_inact, TCODColor sel_back_inact):
	GuiElement(id, x, y, width, height, fore, back)
{
	sel_fore_active = sel_fore_act;
	sel_back_active = sel_back_act;
	sel_fore_inactive = sel_fore_inact;
	sel_back_inactive = sel_back_inact;

	items = new TCODList <GuiObjectLink*>();
}

ActiveGuiElement::~ActiveGuiElement()
{
	items->clearAndDelete();
	delete items;
}

void ActiveGuiElement::addItem(Object* o, string text, TCODColor fore, TCODColor back)
{
	ColoredText* t = new ColoredText(text, fore, back);
	addItem(o, t);
}

void ActiveGuiElement::addItem(Object* o, ColoredText* text)
{
	items->push(new GuiObjectLink(o, text));
	item_count++;
	item_change = true;
}

void ActiveGuiElement::makeActive(Gui* gui)
{
	if (gui->getCurrentActiveElement() == this){
		active = true;
		return;
	}
		
	throw new std::exception("Tried to activate Element, but Gui does not list it as the active one!");
}

GuiListChooser::GuiListChooser(string id, int x, int y, int width, int height,
	TCODColor fore, TCODColor back,
	TCODColor sel_fore_act, TCODColor sel_back_act,
	TCODColor sel_fore_inact, TCODColor sel_back_inact) :
	ActiveGuiElement(id, x, y, width, height,
	fore, back, sel_fore_act, sel_back_act, sel_fore_inact, sel_back_inact){}

GuiListChooser::~GuiListChooser()
{
	delete selected;
}

bool GuiListChooser::removeItem(string text)
{
	for (GuiObjectLink** it = items->begin(); it != items->end(); it++)
	{
		GuiObjectLink *item = *it;
		if (item == selected){ selected = nullptr; }
		if (text.compare(item->text->getText())){

			items->remove(item);
			delete item;

			item_count--;
			item_change = true;

			return true;
		}
	}
	return false;
}

int GuiListChooser::getSelected(Object* obj[])
{
	//Only one item!
	obj = new Object*[]{selected->object};
	return 1;
}

void GuiListChooser::update(TCOD_key_t key)
{
	//If list is empty, return
	if (item_count <= 0 || items->isEmpty()) { return; }
	
	//If item list changed, update selected_index to point at selected item
	if (item_change && selected != nullptr){
		for (int i = 0; i < item_count; i++)
		{
			if (selected == items->get(i)){
				selected_index = i;
				break;
			}
		}
	}

	//If no selection (after init or if selected element got removed)
	// select first item in list.
	if (selected == nullptr) 
	{ 
		selected_index = 0; 
	} 
	
	//If key pressed, modify selected_index accordingly, while "wrapping around"
	// but only, if Element is currently the active one
	if (active){
		switch (key.vk)
		{
		case list_up:
			if (selected_index > 0) { selected_index--; }
			else { selected_index = item_count - 1; }
			break;

		case list_down:
			if (selected_index < item_count - 1) { selected_index++; }
			else { selected_index = 0; }
			break;
		}
	}
	//Refresh selected pointer to point at newly selected object
	selected = items->get(selected_index);

	//selected pointer and selected_index are "synchronized" now
	item_change = false;

}

void GuiListChooser::render(TCODConsole* con)
{
	if (!visible) { return; }

	int index = 0;

	con->setDefaultBackground(background_color);
	con->setDefaultForeground(foreground_color);

	con->rect(pos_x, pos_y, width, height, true);
	
	for (GuiObjectLink** it = items->begin(); it != items->end(); it++)
	{
		GuiObjectLink *item = *it;

		//GuiListChooser will use the Fore color of the Item,
		// but overwrite it's Back color with it's own back_inactive
		// (or, on selection, the back_active color)
		con->setDefaultForeground(item->text->getForeColor());

		if (item == selected)
		{
			if (active) {
				con->setDefaultBackground(sel_back_active);
			} else {
				con->setDefaultBackground(sel_back_inactive); 
			}

		} else {
			con->setDefaultBackground(background_color);
		}

		con->printRect(pos_x, pos_y + index, width, 1, item->text->getText().c_str());
		index++;
	}

}
