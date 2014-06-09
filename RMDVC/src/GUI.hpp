#ifndef GUI_HPP
#define GUI_HPP

#include "libtcod.hpp"
#include <string>

using std::string;

/** It holds the most basic parameters: Position, Size and ID as well as Fore and Back Color.
*
* @brief Base class for GUI Elements and Containers.
*/
class Element
{
protected:
	string id;

	TCODColor background_color;
	TCODColor foreground_color;

	int pos_x;
	int pos_y;
	int width;
	int height;

	void setPosX(int x){ pos_x = x; }
	void setPosY(int y){ pos_y = y; }
	void setWidth(int w){ width = w; }
	void setHeight(int h){ height = h; }

	void setForeColor(TCODColor color){ foreground_color = color; }
	void setBackColor(TCODColor color){ background_color = color; }

	Element(string id, int x, int y, int width, int height, TCODColor fore, TCODColor back);

public:

	string getId(){ return id; }
	int getPosX(){ return pos_x; }
	int getPosY(){ return pos_y; }
	int getWidth(){ return width; }
	int getHeight(){ return height; }

	TCODColor getForeColor() { return foreground_color; }
	TCODColor getBackColor() { return background_color; }

	~Element();
};

//Forward declaration for use in GuiContainer
class GuiElement;

/** It provides Element management and rendering functions.
* It is intended to be used as a sort of 'Panel' which can be moved,
* and that holds several GuiElement children which are drawn on it.
*
* @brief A class that can hold several GuiElement children.
*/
class GuiContainer :public Element
{
protected:
	/** The TCODList holding pointers to the 'child' elements.
	* Note: _Nesting GuiContainers (i.e. having a GuiContainer element
	* be a child of a GuiContainer) is not supported_
	*/
	TCODList<GuiElement*>* elements;
	/** Every GuiContainer declares it's own TCODConsole, on which it
	* renders its GuiElement children. The Console is then passed to the instance
	* of the Gui class handling the overall Gui rendering, which blits it
	* onto the root console.
	*
	* @brief The GuiContainer's TCODConsole.
	*/
	TCODConsole* container_console;

	bool drawBorder;
	string title;

public:
	GuiContainer(string id, int x, int y, int width, int height, TCODColor fore, TCODColor back, bool drawBorder = false, string title = "");
	~GuiContainer();

	void addElement(GuiElement* element);
	bool removeElement(string id);

	/** This function calls the render() function of all GuiElements contained
	* in the elements TCODList, after (if drawBorder == true) drawing its own frame and title.
	* These functions draw on the container_console, which is then returned.
	*
	* @brief Renders all 'child' GuiElements on the container_console.
	* @returns The updated/redrawn container_console.
	*/
	const TCODConsole* render();
};

/** It provides parent handling functions, as well as an abstract render() function
* which must be implemented by all children.
*
* @brief Base class for GuiElements.
*/
class GuiElement : public Element
{
protected:
	GuiContainer* parent;
	
	GuiElement(string id, int x, int y, int width, int height, TCODColor fore, TCODColor back, GuiContainer* parent = nullptr);

public:
	void setParent(GuiContainer* parent) { this->parent = parent; }
	const GuiContainer* getParent() { return parent; }

	virtual void render(TCODConsole* con) = 0;

	~GuiElement();
};


/** It contains a render function, which will draw every registered GuiContainer
* with all of its elements onto the given console.
*
* @brief A class which provides Gui handling to the Engine.
*/
class Gui
{
private:
	TCODList<GuiContainer*>* containers;

public:
	Gui();
	~Gui();

	void render(TCODConsole* con);
	void addContainer(GuiContainer* container);
	bool removeContainer(string id);
};

/** @brief A simple multi-line text box.
*/
class GuiTextBox : public GuiElement 
{
protected:
	string text;
	TCOD_alignment_t alignment;

	void render(TCODConsole* con);

public:
	GuiTextBox(string id, int x, int y, int width, int height, string text, TCODColor fore, TCODColor back, TCOD_alignment_t alignment = TCOD_LEFT);
	~GuiTextBox();

	void setText(string text){ this->text = text; }
	string getText(){ return text; }

	void setAlignment(TCOD_alignment_t alignment){ this->alignment = alignment; }
	TCOD_alignment_t getAlignment(){ return alignment; }

};

class ActiveGuiElement : public GuiElement
{

};

class GuiListBox : public GuiElement
{

};

#endif
