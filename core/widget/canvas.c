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

#include "canvas.h"
#include "widget-callback.h"

static int t = -1;

static int canvasInit(YWidgetState *opac, Entity *entity, void *args)
{
  const char *action;
  Entity *initer = yeGet(entity, "init");

  ywidGenericCall(opac, t, init);

  ((YCanvasState *)opac)->actionIdx = ywidAddSignal(opac, "action");
  action = yeGetString(yeGet(entity, "action"));
  ywidBind(opac, "action", action);
  if (initer) {
    YCallback *callback = ywinGetCallbackByStr(yeGetString(initer));

    if (callback)
      ywidCallCallback(callback, opac, NULL, entity);
  }

  (void)args;
  return 0;
}

static int canvasDestroy(YWidgetState *opac)
{
  g_free(opac);
  return 0;
}

static int canvasRend(YWidgetState *opac)
{
  ywidGenericCall(opac, t, render);
  return 0;
}

static InputStatue canvasEvent(YWidgetState *opac, YEvent *event)
{
  InputStatue ret = NOTHANDLE;

  /* set pos */
  ret = ywidCallSignal(opac, event, NULL,
		       ((YCanvasState *)opac)->actionIdx);

  opac->hasChange = ret == NOTHANDLE ? 0 : 1;
  return ret;
}

static void *alloc(void)
{
  YCanvasState *ret = g_new0(YCanvasState, 1);
  YWidgetState *wstate = (YWidgetState *)ret;

  if (!ret)
    return NULL;

  wstate->render = canvasRend;
  wstate->init = canvasInit;
  wstate->destroy = canvasDestroy;
  wstate->handleEvent = canvasEvent;
  wstate->type = t;
  return  ret;
}

int ywCanvasInit(void)
{
  ywidInitCallback();
  if (t != -1)
    return t;

  t = ywidRegister(alloc, "canvas");
  return t;
}

int ywCanvasEnd(void)
{
  if (ywidUnregiste(t) < 0)
    return -1;
  t = -1;
  return 0;
}
