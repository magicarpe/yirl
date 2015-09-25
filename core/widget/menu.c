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
#include "widget-callback.h"
#include "menu.h"

static int t = -1;


typedef struct {
  YWidgetState sate;
  unsigned int current;
  unsigned int hasChange;
  int moveSinIdx;
  int actionSin0;
} YMenuState;

static int nmMenuDown(YWidgetState *wid, YEvent *eve, Entity *arg)
{
  (void)arg;
  (void)eve;

  ((YMenuState *)wid)->current += 1;

  if (((YMenuState *)wid)->current > yeLen(yeGet(wid->entity, "entries")) - 1)
    ((YMenuState *)wid)->current = 0;
  return NOACTION;
}

static int nmMenuUp(YWidgetState *wid, YEvent *eve, Entity *arg)
{
  (void)arg;
  (void)eve;
  
  ((YMenuState *)wid)->current -= 1;

  if (((YMenuState *)wid)->current > yeLen(yeGet(wid->entity, "entries")))
    ((YMenuState *)wid)->current = yeLen(yeGet(wid->entity, "entries")) - 1;
  return NOACTION;
}

static int nmMenuMove(YWidgetState *wid, YEvent *eve, Entity *arg)
{
if (eve->key == Y_DOWN_KEY) {
  return nmMenuDown(wid, eve, arg);
 } else if (eve->key == Y_UP_KEY) {
  return nmMenuUp(wid, eve, arg);
 }
 return NOTHANDLE;
}

#define yeForeach(entity, idx, entry)					\
  Entity *entry = yeGet(entity, 0);					\
  for (int idx = 0; (entry = yeGet(entries, idx)); ++idx)

static int mnInit(YWidgetState *opac, Entity *entity, void *args)
{
  (void)args;
  YMenuState *state = ((YMenuState *)opac);
  Entity *entries = yeGet(entity, "entries");

  opac->entity = entity;
  state->moveSinIdx = ywidAddSignal(opac, "move");
  ywidBind(opac, "move", "menuMove");
  ywidGenericInit(opac, t);
  state->actionSin0 = state->moveSinIdx + 1;
  yeForeach(entries, i, entry) {
    char *tmp = g_strdup_printf("action-%d", i);
    int ret = ywidAddSignal(opac, tmp);
    Entity *action;

    g_free(tmp);
    if (ret != state->actionSin0 + i)
      return -1;
    action = yeGet(entry, "action");
    if (action)
      ywidBindBySinIdx(opac, ret, yeGetString(yeGet(entry, "action")));
  }
  return 0;
}

static int mnDestroy(YWidgetState *opac)
{
  ywidFinishSignal(opac);
  g_free(opac);
  return 0;
}

static int mnRend(YWidgetState *opac)
{
  int ret = ywidGenericRend(opac, t);  
  ((YMenuState *)opac)->hasChange = 0;
  return ret;
}

static InputStatue mnEvent(YWidgetState *opac, YEvent *event)
{
  InputStatue ret = NOTHANDLE;

  (void)opac;
  if (!event)
    event = ywidGenericWaitEvent();

  if (!event)
    return NOTHANDLE;

  if (event->type != YKEY_DOWN)
    goto exit;
  if (event->key == Y_ESC_KEY) {
    ret = ACTION;

  } else if (event->key == '\n') {
    ret = ywidCallSignal(opac, event, NULL,
			 ((YMenuState *)opac)->current +
			 ((YMenuState *)opac)->actionSin0);
    if (ret == BUG)
      ret = ACTION;

  } else {
    ywidCallSignal(opac, event, NULL,  ((YMenuState *)opac)->moveSinIdx);
  }

  ((YMenuState *)opac)->hasChange = 1;
 exit:
  g_free(event);
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
  wstate->signals = g_array_new(1, 1, sizeof(YSignal *));
  ret->hasChange = 1;
  return  ret;
}

int ywMenuHasChange(YWidgetState *opac)
{
  return ((YMenuState *)opac)->hasChange;
}

int ywMenuGetCurrent(YWidgetState *opac)
{
  return ((YMenuState *)opac)->current;
}

int ywMenuInit(void)
{
  if (t != -1)
    return t;
  t = ywidRegister(alloc, "menu");
  ywinAddCallback(ywinCreateNativeCallback("menuMove", nmMenuMove));
  return t;
}

int ywMenuEnd(void)
{
  if (ywidUnregiste(t) < 0)
    return -1;
  t = -1;
  return 0;
}