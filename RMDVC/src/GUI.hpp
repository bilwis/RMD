#ifndef GUI_HPP
#define GUI_HPP

#include "libtcod.hpp"
#include "Object.hpp"
#include "GUI_structs.hpp"

class Body;

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "Diagnostics.hpp"

using std::string;


/** It includes width and height information as well as a switch to toggle rendering
* on and off (bool visible).
*
* @ A class representing an object that may be rendered on screen as part of the GUI.
*/
class GuiRenderObject : public RenderObject
{
protected:
	int width;
	int height;

	bool visible = true;

	GuiRenderObject(int x, int y, int width, int height, TCODColor fore, TCODColor back);

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
	/** The vector holding pointers to the 'child' elements.
	* Note: _Nesting GuiContainers (i.e. having a GuiContainer element
	* be a child of a GuiContainer) is not supported_
	*/
	std::vector<GuiElement*>* elements;
	/** Every GuiContainer declares it's own TCODConsole, on which it
	* renders its GuiElement children. The Console is then passed to the instance
	* of the Gui class handling the overall Gui rendering, which blits it
	* onto the root console.
	*
	* @brief The GuiContainer's TCODConsole.
	*/
	TCODConsole* container_console;

	/** A dynamic GuiContainer is marked to be updated every turn by the Gui handling class.
	* This should only be used for integral Gui parts such as health bars etc.
	*/
	bool dynamic; 

	bool draw_border;
	string title;

public:
	GuiContainer(int x, int y, int width, int height, TCODColor fore, TCODColor back, bool dynamic = false, bool draw_border = false, string title = "");
	~GuiContainer();

	bool isDynamic() { return dynamic; };
	void setDynamic(bool value) { dynamic = value; };

	void addElement(GuiElement* element);
	bool removeElement(string id);

	virtual void update(TCOD_key_t key);

	/** This function renders only the GuiContainers own graphic elements (such as the border).
	* It does not call the render() function of it's children to allow for custom rendering
	* (such as with ActiveElements).
	* Note that this function should be called before calling the child render function(s), otherwise
	* it will overwrite the output on the console.
	*
	* @brief Renders only the GuiContainers own graphic elements onto the given console.
	*/
	void renderSelf(TCODConsole* con);

	/** This function calls the render() function of all GuiElements contained
	* in the elements vector, after calling renderSelf() .
	* These functions draw on the container_console, which is then blitted onto the given console.
	*
	* @brief Renders all 'child' GuiElements on the given console.
	*/
	virtual void render(TCODConsole* con);
};

/** It provides parent handling functions.
*
* @brief Base class for gui elements (controls).
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

public:
	GuiTextBox(string id, int x, int y, int width, int height, string text, TCODColor fore, TCODColor back, TCOD_alignment_t alignment = TCOD_LEFT);
	~GuiTextBox();

	void setText(string text){ this->text = text; }
	string getText(){ return text; }

	void setAlignment(TCOD_alignment_t alignment){ this->alignment = alignment; }
	TCOD_alignment_t getAlignment(){ return alignment; }

	void render(TCODConsole* con);
};

/** There may be multiple active elements in a GUI, but only one can be the currently
* active one (the one "in focus"), the class implements methods for handling/checking activation.
* Since all GUI elements reacting to input are for choosing/selecting something,
* this class defines colors for the selected items, while the element is active and while
* it is inactive. 
*
* It also implements a vector of GuiObjectLink structs representing the items that may
* be selected and functions for adding items and tracking their number (item_count).
*
* Since this is the base class, removeItem(), update(), getSelected() and render() are abstract and
* must be implemented by the derived class for the specific implementation.
*
* @brief Base class for active GUI elements, i.e. elements reacting to input.
*/
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

	std::vector<GuiObjectLink*>* items;
	int item_count = 0;

	int max_item_display = 0;

	ActiveGuiElement(string id, int x, int y, int width, int height, int max_item_display,
		TCODColor fore, TCODColor back,
		TCODColor sel_fore_act, TCODColor sel_back_act,
		TCODColor sel_fore_inact, TCODColor sel_back_inact);

public:
	~ActiveGuiElement();

	bool isActive(){ return active; }
	virtual void makeActive();
	virtual void makeInactive();

	void setSelectionForeColorInactive(TCODColor color){ sel_fore_inactive = color; }
	void setSelectionBackColorInactive(TCODColor color){ sel_back_inactive = color; }
	void setSelectionForeColorActive(TCODColor color){ sel_fore_active = color; }
	void setSelectionBackColorActive(TCODColor color){ sel_back_active = color; }

	TCODColor getSelectionForeColorInactive() { return sel_fore_inactive; }
	TCODColor getSelectionBackColorInactive() { return sel_back_inactive; }
	TCODColor getSelectionForeColorActive() { return sel_fore_active; }
	TCODColor getSelectionBackColorActive() { return sel_back_active; }

	void addItem(std::string object_uuid, string text, TCODColor fore = gui_default_fore, TCODColor back = gui_default_back);
	void addItem(std::string object_uuid, ColoredText* text);

	void addItems(std::vector<GuiObjectLink*>* list);

	/* This has to be virtual, because the removed Item might be part of the
	current selection, which needs to be handled in order to avoid having
	the selection pointer pointing at a destroyed element!*/
	virtual bool removeItem(string text) = 0; 

	int getItemCount(){ return item_count; }
	
	virtual void update(TCOD_key_t key) = 0;

	//There may be multiple Objects selected. Return the number
	// of Objects and modify the parameter array within the function to 
	// return the UUIDs of the selected Objects.
	virtual int getSelected(std::vector<std::string>* obj_uuids) = 0;

	virtual void reset(){
		//items->clearAndDelete();
		items->erase(items->begin(), items->end());
		item_count = 0;
		item_change = true;
	};
};

/**
* @brief A class representing a list of objects on the GUI, from which _one_ may be chosen.
*/
class GuiListChooser : public ActiveGuiElement
{
protected:
	GuiObjectLink* selected = nullptr;
	int selected_index = 0;

public:
	GuiListChooser(string id, int x, int y, int width, int height,
		int max_item_display,
		TCODColor fore, TCODColor back,
		TCODColor sel_fore_act, TCODColor sel_back_act,
		TCODColor sel_fore_inact, TCODColor sel_back_inact);
	~GuiListChooser();

	bool removeItem(string text);

	void update(TCOD_key_t key);
	int getSelected(std::vector<std::string>* obj_uuids);
	void reset(){ 
		selected = nullptr;
		selected_index = 0;
		items->erase(items->begin(), items->end());
		item_count = 0; 
		item_change = true; 
	}

	void render(TCODConsole* con);
};

/** 
* ###Mockup
* ![Mockup of the BodyViewer](../img/mockup_bodyviewer.png "GuiBodyViewer Mockup")
*
* @brief A class representing a "body inspector", which can display a bodies composition and state.
*/
class GuiBodyViewer : public GuiContainer
{
protected:
	Body* body;
	GuiListChooser* bp_browser;
	GuiTextBox* bp_info;
	GuiListChooser* tissue_browser;

	std::vector<GuiObjectLink*>* part_list;
	std::vector<GuiObjectLink*>* tissue_list;

	ActiveGuiElement* active_element;

	void setActiveBody(Body* b);
public:
	GuiBodyViewer(string id, int x, int y, int width, int height, TCODColor fore, TCODColor back, bool drawBorder = false, string title = "");
	~GuiBodyViewer();

	void activate(Body* b);
	void render(TCODConsole* con);
	void update(TCOD_key_t key);

	Body* getActiveBody() { return body; };
	
};

/** It contains a render function, which will draw every registered GuiContainer
* with all of its elements onto the given console.
*
* @brief A class which provides Gui handling functions to the Engine.
*/
class Gui
{
private:
	//The active one, and all marked as dynamic are updated every turn
	std::map<string, GuiContainer*>* containers;

	GuiContainer* current_active = nullptr;

	GuiContainer* getContainerFromUUID(string uuid);

	const GuiContainer* getCurrentActiveContainer() { return current_active; }
	void makeActive(GuiContainer* container);
public:
	Gui();
	~Gui();

	void render(TCODConsole* con);

	/**@brief This function sends key input to the active container (window)
	*/
	void update(TCOD_key_t key);

	void addContainer(GuiContainer* container);
	bool removeContainer(string uuid);
	bool makeActive(string uuid);

	/** This function is called when the engine switches out of the GameState::GUI state
	* and closes all active elements (makes them invisible).
	*/
	void exitGUIState();
	
};

#endif
