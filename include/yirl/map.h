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

#ifndef	_YIRL_MAP_H_
#define	_YIRL_MAP_H_

#include "yirl/widget.h"
#include "yirl/pos.h"

typedef enum {
  YMAP_FULL = 0,
  YMAP_PARTIAL
} YMapRenderType;

#define	YMAP_SIZE_SPRITE_W  64
#define	YMAP_SIZE_SPRITE_H  64

typedef struct {
  YWidgetState sate;
  Entity *resources;
} YMapState;

/**
 * @return A new entity of type YARRAY, need to be free if father is NULL.
 */
Entity *ywMapCreateDefaultEntity(Entity *father, const char *name,
				 Entity *resources,
				 int baseId, uint32_t w, uint32_t h);

/**
 * create a map of @w * @y elemens
 * @return @entity
 */
Entity *ywMapInitEntity(Entity *entity, Entity *resources,
			int baseId, uint32_t w, uint32_t h);

int ywMapInit(void);
int ywMapEnd(void);
int ycursRegistreMap(void);
int ysdl2RegistreMap(void);

int ywMapType(Entity *map);

int ywMapHasChange(YWidgetState *state);

static inline int ywMapLen(Entity *state)
{
  Entity *tmpLen = yeGetByStrFast(state, "len");
  return  tmpLen ? (uint32_t)yeGetInt(tmpLen) :
    yeLen(yeGetByStrFast(state, "map"));
}

static inline int ywMapW(Entity *state)
{
  return yeGetInt(yeGetByStrFast(state, "width"));
}

static inline int ywMapH(Entity *state)
{
  return ywMapLen(state) / ywMapW(state);
}

int ywMapGetIdByElem(Entity *mapElem);

Entity *ywMapGetCase(Entity *state, Entity *pos);

static inline Entity *ywMapGetNbrEntityAt(Entity *state, Entity *pos, int nbr)
{
  Entity *tmp = ywMapGetCase(state, pos);

  YE_ARRAY_FOREACH(tmp, caseTmp) {
    if (ywMapGetIdByElem(caseTmp) == nbr)
      return caseTmp;
  }
  return NULL;
}

Entity *ywMapPosFromInt(Entity *wid, int newPos,
			Entity *father, const char *name);
int ywMapIntFromPos(Entity *wid, Entity *pos);

Entity *ywMapPushElem(Entity *state, Entity *toPush,
		      Entity *pos, const char *name);

Entity *ywMapPushNbr(Entity *state, int toPush,
		     Entity *pos, const char *name);

static inline Entity *ywMapGetResources(YWidgetState *state)
{
  return ((YMapState *)state)->resources;
}

static inline int ywMapRemoveByEntity(Entity *state, Entity *pos,
				      Entity *elem)
{
  Entity *cur = ywMapGetCase(state, pos);

  if (unlikely(!cur)) {
    return -1;
  }
  yeRemoveChild(cur, elem);
  return 0;
}

static inline int ywMapRemoveByStr(Entity *state, Entity *pos,
				    const char *str)
{
  Entity *cur = ywMapGetCase(state, pos);

  if (unlikely(!cur)) {
    return - 1;
  }
  yeRemoveChild(cur, yeGetByStrFast(cur, str));
  return 0;
}

#define ywMapRemove(sate, pos, elem)					\
  _Generic(elem,							\
	  Entity *: ywMapRemoveByEntity,				\
	  Y_GEN_CLANG_ARRAY(char, ywMapRemoveByStr),			\
	  const char *: ywMapRemoveByStr,				\
	  char *: ywMapRemoveByStr) (sate, pos, elem)


int ywMapMoveByStr(Entity *state, Entity *from,
		   Entity *to, const char *elem);

int ywMapMoveByEntity(Entity *state, Entity *from,
		      Entity *to, Entity *elem);

/**
 * @map	The map to draw on
 * @posStart x, y pos where to start drawing
 * @size a pos use to specify width/height
 */
int ywMapDrawRect(Entity *map, Entity *posStart, Entity *size, int id);

/**
 * @brief add @x and @y to @pos then move @elem at @pos
 * @pos initial position, modified durring operation
 */
static inline int ywMapAdvenceWithPos(Entity *map, Entity *pos,
				      int x, int y, Entity *elem)
{
  if (unlikely(!elem || !map || ! pos)) {
    if (!map)
      DPRINT_WARN("map is NULL");
    else if (!elem)
      DPRINT_WARN("elem is NULL");
    else
      DPRINT_WARN("pos is NULL");
    return -1;
  }
  YE_INCR_REF(elem);
  ywMapRemoveByEntity(map, pos, elem);
  ywPosAddXY(pos, x, y);

  if (ywPosX(pos) < 0)
    ywPosSetX(pos, ywMapW(map) + ywPosX(pos));
  else if (ywPosX(pos) >= ywMapW(map))
    ywPosSetX(pos, ywPosX(pos) - ywMapW(map));

  if (ywPosY(pos) < 0)
    ywPosSetY(pos, ywMapH(map) + ywPosY(pos));
  else if (ywPosY(pos) >= ywMapH(map))
    ywPosSetY(pos, ywPosY(pos) - ywMapH(map));
  ywMapPushElem(map, elem, pos, NULL);
  YE_DESTROY(elem);
  return 0;
}

void ywMapGetSpriteSize(Entity *map,
			unsigned int *sizeSpriteW,
			unsigned int *sizeSpriteH);

void yeMapPixielsToPos(Entity *wid, uint32_t pixX, uint32_t pixY,
		       uint32_t *w, uint32_t *h);

Entity *ywMapPosFromPixs(Entity *wid, uint32_t x, uint32_t y,
			 Entity *father, const char *name);

#endif