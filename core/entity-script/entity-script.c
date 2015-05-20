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

#include "entity-script.h"

void *yesVCall(void *sm, Entity *func, va_list *ap)
{
  if (!func)
    return NULL;
  if (!yeGetFunction(func))
    return NULL;
  return ysVCall(sm, yeGetFunction(func), YE_TO_FUNC(func)->nArgs, ap);
}

void *yesCall(void *sm, Entity *func, ...)
{
  void *ret;
  va_list ap;

  if (!func)
    return NULL;
  va_start(ap, func);
  ret = yesVCall(sm, func, &ap);
  va_end(ap);
  return ret;
}
