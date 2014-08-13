#ifndef GUI_STRUCTS_HPP
#define GUI_STRUCTS_HPP

#include "libtcod.hpp"
#include "Object.hpp"
#include <string>
#include <memory>

using std::string;

//Default Options
//TODO: Put into config file
const TCODColor gui_default_fore = TCODColor::white;
const TCODColor gui_default_back = TCODColor::black;

const TCODColor gui_sel_fore_act = TCODColor::yellow;
const TCODColor gui_sel_back_act = TCODColor::grey;
const TCODColor gui_sel_fore_inact = TCODColor::lightYellow;
const TCODColor gui_sel_back_inact = TCODColor::darkGrey;

const TCOD_keycode_t list_up = TCODK_UP;
const TCOD_keycode_t list_down = TCODK_DOWN;
const TCOD_keycode_t list_select = TCODK_ENTER;

const std::string gui_list_indent_char = " ";

/**
* @brief A class encapsulating a string and background and foreground color information.
*/
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
	~ColoredText(){};

	string getText(){ return text; }
	TCODColor getForeColor() { return foreground_color; }
	TCODColor getBackColor() { return background_color; }
};

/** This struct is for use with ActiveGuiElement instances which present some form
* of list of objects. It provides a convenient way of linking (for example)
* game entities to thier associated list entry by associating their UUID with a ColoredText.
*
* @brief A struct encapsulating an Objects UUID and a pointer to an associated ColoredText.
*/
struct GuiObjectLink
{
	std::string object_uuid;
	ColoredText* text;

	GuiObjectLink(std::string uuid, ColoredText* t){ object_uuid = uuid; text = t; };
	~GuiObjectLink(){ delete text; };
};

#endif
