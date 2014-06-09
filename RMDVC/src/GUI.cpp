#include "GUI.hpp"

//Commentate

Gui::Gui(){
	containers = new TCODList<GuiContainer*>();
};
Gui::~Gui(){
	containers->clearAndDelete();
	delete containers;
};

void Gui::addContainer(GuiContainer* container){
	containers->push(container);
}

void Gui::render(TCODConsole* con){
	//Iterate through all GuiContainers, call their render function
	// and blit the result on the given console at the coordinates that 
	// the GuiContainer is set to.
	for (GuiContainer** it = containers->begin(); it != containers->end(); it++)
	{
		GuiContainer *container = *it;
		TCODConsole::blit(container->render(), 0, 0, 0, 0, con,
			container->getPosX(),
			container->getPosY());
	}
}

Element::Element(string id, int x, int y, int width, int height, TCODColor fore, TCODColor back){
	this->id = id;
	pos_x = x;
	pos_y = y;
	this->width = width;
	this->height = height;
	foreground_color = fore;
	background_color = back;
}

Element::~Element(){}

GuiElement::GuiElement(string id, int x, int y, int width, int height, TCODColor fore, TCODColor back, GuiContainer* parent):
Element(id, x, y, width, height, fore, back)
{
	this->parent = parent;
}

GuiElement::~GuiElement(){}

GuiContainer::GuiContainer(string id, int x, int y, int width, int height,
	TCODColor fore, TCODColor back, bool drawBorder, string title)
	:Element(id,x,y,width, height, fore, back){
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

void GuiTextBox::render(TCODConsole* con){
	con->setDefaultBackground(background_color);
	con->setDefaultForeground(foreground_color);
	con->setAlignment(alignment);
	con->printRect(pos_x, pos_y, width, height, text.c_str());
}