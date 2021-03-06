/*
**Copyright (C) 2017 Matthias Gatto
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

#ifndef _SDL2_CANVAS_SDL_H_
#define _SDL2_CANVAS_SDL_H_

struct SDL_Surface;
typedef struct SDL_Surface SDL_Surface;

void sdlFreeSurface(void *surface);
int sdlCanvasCacheTexture(Entity *state, Entity *elem);
uint32_t sdlCanvasPixInfo(Entity *obj, int x, int y);
SDL_Surface *sdlCopySurface(SDL_Surface *surface, Entity *rEnt);
int sdlCanvasCacheImg(Entity *elem, Entity *resource, const char *imgPath,
		      Entity *rEnt);
int sdlMergeSurface(Entity *textSrc, Entity *srcRect,
		    Entity *textDest, Entity *destRect);

#endif
