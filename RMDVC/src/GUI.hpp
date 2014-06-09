#ifndef GUI_HPP
#define GUI_HPP

#include "libtcod.hpp"
#include "Object.hpp"
#include <string>

using std::string;

const TCODColor gui_default_fore = TCODColor::white;
const TCODColor gui_default_back = TCODColor::black;

const TCOD_keycode_t list_up = TCODK_UP;
const TCOD_keycode_t list_down = TCODK_DOWN;
const TCOD_keycode_t list_select = TCODK_ENTER;

class ColoredText
{
protected:
	TCODColor foreground_color;
	TCODColor background_color;

	string text;

public:
	ColoredText(string text, 
		TCODColor fore = gui_default_fore,
		TCODColor back = gui_default_back)
	{
		this->text = text; 
		foreground_color = fore;
		background_color = back;
	};
	~ColoredText();

	string getText(){ return text; }
	TCODColor getForeColor() { return foreground_color; }
	TCODColor getBackColor() { return background_color; }
};

struct GuiObjectLink
{
	Object* object;
	ColoredText* text;

	GuiObjectLink(Object* o, ColoredText* t){ object = o; text = t; };
	~GuiObjectLink(){};
};

class GuiRenderObject : public RenderObject
{
protected:
	int width;
	int height;

	bool visible = true;

	GuiRenderObject(string id, int x, int y, int width, int height, TCODColor fore, TCODColor back);

public:
	int getWidth(){ return width; }
	int getHeight(){ return height; }

	void setWidth(int w){ width = w; }
	void setHeight(int h){ height = h; }

	bool isVisible(){ return visible; }
	void setVisibility(bool visible) { this->visible = visible; }

	~GuiRenderObject();
};

//Forward declaration for use in GuiContainer
class GuiElement;
class Gui;

/** It provides Element management and rendering functions.
* It is intended to be used as a sort of 'Panel' which can be moved,
* and that holds several GuiElement children which are drawn on it.
*
* @brief A class that can hold several GuiElement children.
*/
class GuiContainer :public GuiRenderObject
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
class GuiElement : public GuiRenderObject
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
protected:
	bool active = false;

	bool item_change = false; //Set true when changing item list
							  // handle in update(), then set false
	
	//The colors for the selected item(s), when
	// the Element is focus (active) and when it's not.
	TCODColor sel_fore_active; 
	TCODColor sel_back_active;
	TCODColor sel_fore_inactive;
	TCODColor sel_back_inactive;

	TCODList<GuiObjectLink*>* items;
	int item_count = 0;

	ActiveGuiElement(string id, int x, int y, int width, int height,
		TCODColor fore, TCODColor back,
		TCODColor sel_fore_act, TCODColor sel_back_act,
		TCODColor sel_fore_inact, TCODColor sel_back_inact);

public:
	~ActiveGuiElement();

	bool isActive(){ return active; }
	void makeActive(Gui* gui);

	void setSelectionForeColorInactive(TCODColor color){ sel_fore_inactive = color; }
	void setSelectionBackColorInactive(TCODColor color){ sel_back_inactive = color; }
	void setSelectionForeColorActive(TCODColor color){ sel_fore_active = color; }
	void setSelectionBackColorActive(TCODColor color){ sel_back_active = color; }

	TCODColor getSelectionForeColorInactive() { return sel_fore_inactive; }
	TCODColor getSelectionBackColorInactive() { return sel_back_inactive; }
	TCODColor getSelectionForeColorActive() { return sel_fore_active; }
	TCODColor getSelectionBackColorActive() { return sel_back_active; }

	void addItem(Object* o, string text, TCODColor fore = gui_default_fore, TCODColor back = gui_default_back);
	void addItem(Object* o, ColoredText* text);

	/* This has to be virtual, because the removed Item might be part of the
	current selection, which needs to be handled in order to avoid having
	the selection pointer pointing at a destroyed element!*/
	virtual bool removeItem(string text) = 0; 
	

	int getItemCount(){ return item_count; }
	
	virtual void update(TCOD_key_t key) = 0;

	//There may be multiple Objects selected. Return the number
	// of Objects and modify the parameter array within the function to 
	// point at the Objects.
	virtual int getSelected(Object* obj[]) = 0;
};

class GuiListChooser : public ActiveGuiElement
{
protected:
	GuiObjectLink* selected = nullptr;
	int selected_index = 0;

	void render(TCODConsole* con);
public:
	GuiListChooser(string id, int x, int y, int width, int height,
		TCODColor fore, TCODColor back,
		TCODColor sel_fore_act, TCODColor sel_back_act,
		TCODColor sel_fore_inact, TCODColor sel_back_inact);
	~GuiListChooser();

	bool removeItem(string text);

	void update(TCOD_key_t key);
	int getSelected(Object* obj[]);
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
	TCODList<ActiveGuiElement*>* active_elements;
	ActiveGuiElement* current_active = nullptr;

	void makeActive(ActiveGuiElement* element);
public:
	Gui();
	~Gui();

	void render(TCODConsole* con);
	void update(TCOD_key_t key);

	void addContainer(GuiContainer* container);
	bool removeContainer(string id);

	void registerActiveElement(ActiveGuiElement* element);
	bool removeActiveElement(string id);

	bool makeActive(string id);

	const ActiveGuiElement* getCurrentActiveElement() { return current_active; }
};

#endif
