/*
**Copyright (C) 2015 Matthias Gatto
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

#include <glib.h>
#include "menu.h"
#include "container.h"
#include "rect.h"
#include "native-script.h"
#include "entity-script.h"
#include "game.h"

static int t = -1;

typedef struct {
  YWidgetState sate;
  unsigned int current;
} YMenuState;

static void *nmMenuDown(YWidgetState *wid)
{
  ((YMenuState *)wid)->current += 1;

  if (((YMenuState *)wid)->current > yeLen(yeGet(wid->entity, "entries")) - 1)
    ((YMenuState *)wid)->current = 0;
  if (yeGetInt(yeGet(ywMenuGetCurrentEntry(wid->entity), "hiden")))
    return nmMenuDown(wid);
  return (void *)NOACTION;
}

static void *nmMenuUp(YWidgetState *wid)
{
  ((YMenuState *)wid)->current -= 1;

  if (((YMenuState *)wid)->current > yeLen(yeGet(wid->entity, "entries")))
    ((YMenuState *)wid)->current = yeLen(yeGet(wid->entity, "entries")) - 1;
  if (yeGetInt(yeGet(ywMenuGetCurrentEntry(wid->entity), "hiden")))
    return nmMenuUp(wid);
  return (void *)NOACTION;
}

static void *nmMenuMove(va_list ap)
{
  Entity *wid = va_arg(ap, Entity *);
  Entity *eve = va_arg(ap, Entity *);

  if (ywidEveKey(eve) == Y_DOWN_KEY) {
    return nmMenuDown(ywidGetState(wid));
  } else if (ywidEveKey(eve) == Y_UP_KEY) {
    return nmMenuUp(ywidGetState(wid));
  }
  return (void *)NOTHANDLE;
}

static void *nmPanelMove(va_list ap)
{
  YWidgetState *wid = ywidGetState(va_arg(ap, Entity *));
  Entity *eve = va_arg(ap, Entity *);

  if (ywidEveKey(eve) == Y_RIGHT_KEY) {
    return nmMenuDown(wid);
  } else if (ywidEveKey(eve) == Y_LEFT_KEY) {
    return nmMenuUp(wid);
  }
  return (void *)NOTHANDLE;
}

static void *nmMenuNext(va_list ap)
{
  YWidgetState *wid = ywidGetState(va_arg(ap, Entity *));
  Entity *next = yeGet(wid->entity, "entries");

  next = yeGet(next, ((YMenuState *)wid)->current);
  next = yeGet(next, "next");

  return ywidNext(next) ? (void *)BUG : (void *)ACTION;
}


static void *mnActions(va_list ap)
{
  Entity *wid = va_arg(ap, Entity *);
  Entity *eve = va_arg(ap, Entity *);
  void *arg = va_arg(ap, void *);

  void *ret = (void *)ywidActions(wid, ywMenuGetCurrentEntry(wid), eve, arg);
  return ret;
}

void ywMenuUp(Entity *wid)
{
  nmMenuUp(ywidGetState(wid));
}

void ywMenuDown(Entity *wid)
{
  nmMenuDown(ywidGetState(wid));
}

static int mnInit(YWidgetState *opac, Entity *entity, void *args)
{
  (void)args;
  ywidGenericCall(opac, t, init);

  yeRemoveChildByStr(entity, "move");
  if (!yeStrCmp(yeGet(entity, "mn-type"), "panel")) {
    yeCreateFunction("panelMove", ysNativeManager(), entity, "move");
  } else {
    yeCreateFunction("menuMove", ysNativeManager(), entity, "move");
  }
  ((YMenuState *)opac)->current = yeGetInt(yeGet(entity, "current"));
  return 0;
}

static int mnDestroy(YWidgetState *opac)
{
  g_free(opac);
  return 0;
}

static int mnRend(YWidgetState *opac)
{
  ywidGenericRend(opac, t, render);
  opac->hasChange = 0;
  return 0;
}

InputStatue ywMenuCallActionOnByEntity(Entity *opac, Entity *event,
				       int idx, void *arg)
{
  return ywMenuCallActionOnByState(ywidGetState(opac), event, idx, arg);
}

InputStatue ywMenuCallActionOnByState(YWidgetState *opac, Entity *event,
				      int idx, void *arg)
{
  InputStatue ret;

  if (idx < 0)
    return NOTHANDLE;
  ((YMenuState *)opac)->current = idx;

  ret = ywidActions(opac->entity, ywMenuGetCurrentEntry(opac->entity),
		    event, arg);
  if (ret == NOTHANDLE)
    return NOACTION;
  return ret;
}

static InputStatue mnEvent(YWidgetState *opac, Entity *event)
{
  InputStatue ret = NOTHANDLE;

  (void)opac;

  if (!event)
    return NOTHANDLE;

  if (ywidEveType(event) == YKEY_DOWN) {
    if (ywidEveKey(event) == '\n') {
      ret = ywMenuCallActionOn(opac, event, ((YMenuState *)opac)->current, NULL);
    } else {
      ret = (InputStatue)yesCall(yeGet(opac->entity, "move"), opac->entity, event);
    }
  } else if (ywidEveType(event) == YKEY_MOUSEDOWN) {
    ret = ywMenuCallActionOn(opac, event,
			     ywMenuPosFromPix(opac->entity,
					      ywPosX(ywidEveMousePos(event)),
					      ywPosY(ywidEveMousePos(event))),
			     NULL);
  }
  return ret;
}


static void *alloc(void)
{
  YMenuState *ret = g_new0(YMenuState, 1);
  YWidgetState *wstate = (YWidgetState *)ret;

  if (!ret)
    return NULL;
  wstate->render = mnRend;
  wstate->init = mnInit;
  wstate->destroy = mnDestroy;
  wstate->handleEvent = mnEvent;
  wstate->type = t;
  return  ret;
}

int ywMenuHasChange(YWidgetState *opac)
{
  return opac->hasChange;
}

int ywMenuPosFromPix(Entity *wid, uint32_t x, uint32_t y)
{
  Entity *entries = yeGet(wid, "entries");
  Entity *pos = yeGet(wid, "wid-pix");

  YE_ARRAY_FOREACH_EXT(entries, entry, it) {
    Entity *rect = yeGet(entry, "$rect");
    if (ywRectContain(rect, x - ywRectX(pos), y - ywRectY(pos), 1))
      return it.pos;
  }
  return -1;
}

int ywMenuGetCurrent(YWidgetState *opac)
{
  return ((YMenuState *)opac)->current;
}

Entity *ywMenuGetCurrentEntry(Entity *entity)
{
  return yeGet(yeGet(entity, "entries"),
	       ywMenuGetCurrent(ywidGetState(entity)));
}

int ywMenuInit(void)
{
  if (t != -1)
    return t;
  t = ywidRegister(alloc, "menu");
  ysRegistreNativeFunc("menuMove", nmMenuMove);
  ysRegistreNativeFunc("panelMove", nmPanelMove);
  ysRegistreNativeFunc("menuNext", nmMenuNext);
  ysRegistreNativeFunc("menuActions", mnActions);
  return t;
}

int ywMenuEnd(void)
{
  if (ywidUnregiste(t) < 0)
    return -1;
  t = -1;
  return 0;
}

Entity *ywMenuGetEntry(Entity *container, int idx)
{
  return ywCntGetEntry(container, idx);
}
