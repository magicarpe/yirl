/*
**Copyright (C) 2016 Matthias Gatto
**
**This program is free software: you can redistribute it and/or modify
**it under the terms of the GNU Lesser General Public License as published by
**the Free Software Foundation, either version 3 of the License, or
**(at your option) any later version.
**
**This program is distributed in the hope that it will be useful,
**but WITHOUT ANY WARRANTY; without even the implied warranty of
**MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**GNU General Public License for more details.
**
**You should have received a copy of the GNU Lesser General Public License
**along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <yirl/game.h>
#include <yirl/menu.h>
#include <yirl/map.h>
#include <yirl/container.h>
#include <yirl/text-screen.h>
#include <yirl/rect.h>
#include <list/list.h>

static Entity *globMousePos;

void *init(int nbArgs, void **args)
{
  Entity *mod = args[0];
  Entity *init;
  Entity *map = yeCreateArray(mod, "battle");
  Entity *cp;

  yeCreateString("battle", map, "<type>");
  yePushBack(map, yeGetByStr(mod, "resources.battle-map"), "resources");
  cp = yeCreateArray(map, "player 1");
  ylist_init_from_array(yeGetByStr(mod, "player1 fleets"), cp, "fleet");
  cp = yeCreateArray(map, "player 2");
  ylist_init_from_array(yeGetByStr(mod, "player2 fleets"), cp, "fleet");
  yeCreateInt(2, map, "cursor id");
  init = yeCreateArray(NULL, NULL);
  yeCreateString("battle", init, "name");
  yeCreateFunction("battleInit", ygGetManager("tcc"), init, "callback");
  ywidAddSubType(init);
  return NULL;
}

#include "actions.c"

void *battleInit(int nbArgs, void **args)
{
  Entity *gc = yeCreateArray(NULL, NULL);
  Entity *main = args[0];
  Entity *layers = yeCreateArray(main, "entries");
  Entity *cur_layer;
  Entity *resources = yeGetByStrFast(main, "resources");
  Entity *entity;
  Entity *textScreen;
  Entity *panel;

  globMousePos = ywPosCreateInts(0, 0, NULL, NULL);
  yeCreateFunction("battleAction", ygGetTccManager(), main, "action");
  Entity *pos = ywPosCreateInts(0, 0, main, "_cursos pos");

  if (!yeGetByStr(main, "player 1") || !yeGetByStr(main, "player 2")) {
    DPRINT_ERR("unable to get players fleets");
    return NULL;
  }
  /* create maps */
  entity = yeCreateArray(layers, NULL);
  panel = yeCreateArray(layers, NULL);
  textScreen = yeCreateArray(layers, NULL);

  yeCreateInt(80, entity, "size");
  yeCreateInt(5, panel, "size");
  layers = yeCreateArray(entity, "entries");
  yeCreateString("container", entity, "<type>");
  yeCreateString("stacking", entity, "cnt-type");

  cur_layer = ywMapCreateDefaultEntity(layers, NULL,
				       resources,
				       0, 25, 25);
  yeReCreateString("rgba: 255 255 255 255", cur_layer, "background");
  yeCreateString("map", cur_layer, "<type>");


  cur_layer = ywMapCreateDefaultEntity(layers, NULL, resources, -1, 25, 25);
  yeCreateString("map", cur_layer, "<type>");
  ywMapDrawRect(cur_layer, ywRectCreateInts(0, 0, 25, 5, gc, NULL), 3);

  /* battle specific fields */
  yeCreateInt(0, entity, "_state");
  yePushBack(main, yeGetByStr(main, "player 1"), "current_player");

  ywMapPushElem(getLayer(main, 1), yeGetByStr(main, "cursor id"), pos, NULL);

  yeCreateString("text-screen", textScreen, "<type>");
  yeReCreateString("rgba: 200 200 200 255", textScreen, "background");

  printFLeet(yeCreateString("", textScreen, "text"),
	     yeGetByStr(yeGetByStr(main, "current_player"), "fleet"));

  yeCreateString("menu", panel, "<type>");
  yeReCreateString("rgba: 100 100 100 255", panel, "background");
  yeCreateString("panel", panel, "mn-type");
  layers = yeCreateArray(panel, "entries");

  cur_layer = yeCreateArray(layers, NULL);
  yeCreateString("add ship", cur_layer, "text");
  yeCreateFunction("addShipCallback", ygGetTccManager(), cur_layer, "action");

  cur_layer = yeCreateArray(layers, NULL);
  yeCreateString("remove ship", cur_layer, "text");
  yeCreateFunction("helloWorld", ygGetTccManager(), cur_layer, "action");

  cur_layer = yeCreateArray(layers, NULL);
  yeCreateString("next ship", cur_layer, "text");
  yeCreateFunction("helloWorld", ygGetTccManager(), cur_layer, "action");

  cur_layer = yeCreateArray(layers, NULL);
  yeCreateString("end positioning", cur_layer, "text");
  yeCreateFunction("nextStateCallback", ygGetTccManager(), cur_layer, "action");

  void *ret = ywidNewWidget(main, "container");
  yeDestroy(gc);
  return ret;
}
