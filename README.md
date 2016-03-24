# YIRL isn't a Rogue-like

YIRL is a WIP,
This README is a WIP.
Don't wait to get something functional for now :)

YIRL is a game engine aiming to be fully configruable, fully scriptable and
fully modable.

It's compose by a multitude of library's and one binary to rule them all.

Therses libriry's are divide into 5 tier.
all library's are based on glib:

--------------- tier 1 -----------------

* util : multitude of usefull tool for YIRL
	* debug: debug functions [debug.h]
	* block-array: the array system use by the array entity [block-array.h]
	* strings: yuiStrEqual and yuiStrEqual0 are use to check
		 strict euqlity between strings [utils.h]
	* rand: yuiRand, the yirl random function [utils.h]
	* other: some usefull macro and functions [utils.h]

sound: allow to play sound [sound.h]

-------------- tier 2 -----------------

* entity: entity system 
 	  have int, float, string, array, function and data entitys
	  [entity.h]

-------------- tier 3 -----------------

* script: [script.h]
	* lua: [lua-binding.h, lua-convert.h, lua-script.h]
	* tcc: [tcc-script.h]

* description: [description.h]
	* json: [json-desc.h]

-------------- tier 4 -----------------

* widget:
	* base: [widget.h]
	* sdl: [sdl-driver.h]
	* curses: [curses-driver.h]
	* callback: [widget-callback.h]
	* map: [map.h]
	* text-screen: [text-screen.h]
	* menu: [menu.h]
	* canvas: [canvas.h]
	* contener: [contener.h] 

------------- tier 5 -----------------


* game: main loop and module management,
      This library include all other librarys and allow to easily create
      a game.
      [game.h]
