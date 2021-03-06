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

#ifndef	_SDL_DRIVER_H_
#define	_SDL_DRIVER_H_

void ysdl2Destroy(void);
int ysdl2Type(void);
int ysdl2Init(void);
int ysdl2FullScreen(void);
int ysdl2WindowMode(void);
int ysdl2RegistreTextScreen(void);
int ysdl2RegistreMenu(void);

#endif
