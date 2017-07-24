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

#include <glib.h>
#include <stdlib.h>
#include <string.h>

#include "ybytecode.h"
#include "ybytecode-script.h"
#include "entity.h"
#include "rawfile-desc.h"
#include "description.h"
#include "game.h"

static int t = -1;
static void *manager;

Entity *functions;

struct YBytecodeScript {
  YScriptOps ops;
  Entity *map;
};

int YBytecodeScriptDirectReturn;

#define DEF(a, b, c) a,

enum asm_toks {
	YTOK_STR_BASE,
	#include "ybytecode-asm-tok.h"
};

#undef DEF

#define GET_MAP(sm) (((struct YBytecodeScript *)sm)->map)

static int ybytecodeRegistreFunc(void *sm, const char *name, void *arg)
{
  yeCreateData(arg, GET_MAP(sm), name);
  return 0;
}

Entity *ysYbytecodeCreateFunc(Entity *data, Entity *father, const char *name)
{
  Entity *ret;

  yePushBack(GET_MAP(ysYBytecodeManager()), data, name);
  ret = yeCreateFunctionExt(name, manager, father, name,
			    YE_FUNC_NO_FASTPATH_INIT);
  YE_TO_FUNC(ret)->fastPath = yeGetData(data);
  return ret;
}

static void *ybytecodeGetFastPath(void *sm, const char *name)
{
  return yeGetData(yeGet(GET_MAP(sm), name));
}

static void *ybytecodeFastCall(void *opacFunc, va_list ap)
{
  Entity *stack  = yeCreateArrayExt(NULL, NULL,
				    YBLOCK_ARRAY_NOINIT |
				    YBLOCK_ARRAY_NOMIDFREE);
  Entity *ret;
  int iret;
  double fret;
  void *dret;

  for (void *tmp = va_arg(ap, void *);
       tmp != Y_END_VA_LIST; tmp = va_arg(ap, void *)) {
    if (yeIsPtrAnEntity(tmp))
      yePushBack(stack, tmp, NULL);
    else
      yeCreateData(tmp, stack, NULL);
  }
  ret = ybytecode_exec(stack, opacFunc);
  yeDestroy(stack);
  if (!ret) {
    if (ybytecode_error) {
      DPRINT_ERR("%s", ybytecode_error);
      g_free(ybytecode_error);
    }
    return NULL;
  }
  if (YBytecodeScriptDirectReturn)
    return ret;

  switch (yeType(ret)) {
  case YINT:
    iret = yeGetInt(ret);
    yeDestroy(ret);
    return (void *)(size_t)iret;
  case YFLOAT:
    fret = yeGetFloat(ret);
    yeDestroy(ret);
    return (void *)(size_t)fret;
  case YDATA:
    dret = yeGetData(ret);
    yeDestroy(ret);
    return dret;
  default:
    break;
  }
  return ret;
}

static void *ybytecodeCall(void *sm, const char *name, va_list ap)
{
  return ybytecodeFastCall(ybytecodeGetFastPath(sm, name), ap);
}

static int ybytecodeDestroy(void *sm)
{
  if (!sm)
    return -1;
  yeDestroy(GET_MAP(sm));
  g_free(sm);
  return 0;
}

static int isTokSeparato(int tok)
{
  return tok == SPACES || tok == RETURN;
}

static int64_t tokToInstruction(int tok, Entity *tokInfo)
{
  switch (tok) {
  case ADD:
    return '+';
  case SUB:
    return '-';
  case DIV:
    return '/';
  case MULT:
    return '*';
  case YB_INCR_TOK:
    return YB_INCR;
  case END_RET:
    return 'E';
    /* fall through */
  case END:
    return 'e';
  default:
    DPRINT_ERR("'%s' is not an instruction\n", yeTokString(tokInfo, tok));
    break;
  }
  return -1;
}

static int nextNonSeparatorTok(Entity *str, Entity *tokInfo)
{
  int tok;

  while (isTokSeparato((tok = yeStringNextTok(str, tokInfo))));
  return tok;
}

static int tryStoreNumber(int64_t *dest, Entity *str, Entity *tokInfo)
{
  int tok = nextNonSeparatorTok(str, tokInfo);

  if (tok != NUMBER) {
    DPRINT_ERR("expected number, got '%s'\n", yeTokString(tokInfo, tok));
    return -1;
  }
  *dest = atoi(yeTokString(tokInfo, tok));
  return 0;
}

static int parseFunction(Entity *map, Entity *str, Entity *tokInfo)
{
  int tok = YTOK_WORD;
  Entity *funcName = yeCreateString(yeTokString(tokInfo, tok), NULL, NULL);
  int64_t script[1024];
  uint32_t script_len = 1;
  int ret = -1;

  script[0] = 0;
  tok = nextNonSeparatorTok(str, tokInfo);

  if (tok != OPEN_BRACE) {
    DPRINT_ERR("unexpected '%s', expect '{' in function",
	       yeTokString(tokInfo, tok), yeGetString(funcName));
    goto exit;
  }
 still_in_func:
  tok = yeStringNextTok(str, tokInfo);
  switch (tok) {
  case ADD:
  case SUB:
  case DIV:
  case MULT:
    script[script_len] = tokToInstruction(tok, tokInfo);
    if (tryStoreNumber(&script[script_len + 1], str, tokInfo) < 0)
      goto exit;
    if (tryStoreNumber(&script[script_len + 2], str, tokInfo) < 0)
      goto exit;
    if (tryStoreNumber(&script[script_len + 3], str, tokInfo) < 0)
      goto exit;
    script_len += 4;
    goto still_in_func;
  case YB_INCR_TOK:
  case END_RET:
    script[script_len] = tokToInstruction(tok, tokInfo);
    if (tryStoreNumber(&script[script_len + 1], str, tokInfo) < 0)
      goto exit;
    script_len += 2;
    goto still_in_func;
  case END:
    script[script_len] = tokToInstruction(tok, tokInfo);
    script_len += 1;
    goto still_in_func;
  case SPACES:
  case RETURN:
    goto still_in_func;
  case CLOSE_BRACE:
    ret = 0;
    break;
  default:
    DPRINT_ERR("unexpected '%s' in function",
	       yeTokString(tokInfo, tok), yeGetString(funcName));
    goto exit;
  }
  Entity *data;
  if (script_len < (yeMetadataSize(DataEntity) / sizeof(uint64_t))) {
    data = yeCreateDataExt(NULL, NULL, NULL, YE_DATA_USE_OWN_METADATA);
  } else {
    void *realData = malloc(sizeof(uint64_t) * script_len);
    data = yeCreateData(realData, NULL, NULL);
    yeSetDestroy(data, free);
  }
  memcpy(yeGetData(data), script, sizeof(uint64_t) * script_len);
  ysYbytecodeCreateFunc(data, map, yeGetString(funcName));
  yeDestroy(data);
 exit:
  yeDestroy(funcName);
  return ret;
}

static int loadFile(void *opac, const char *fileName)
{
  Entity *map = GET_MAP(opac);
  Entity *str = ygFileToEnt(YRAW_FILE, fileName, NULL);
  int tok;
  Entity *tokInfo = yeTokInfoCreate(NULL, NULL);

  /* populate tokInfo, withstuff declare inside "ybytecode-asm-tok.h" */
#define DEF(a, b, c) YUI_CAT(DEF_, c)(a, b)
#define DEF_string(a, b) yeCreateString(b, tokInfo, NULL);
#define DEF_repeater(a, b) yeTokInfoAddRepeated(b, tokInfo);
#define DEF_separated_string(a, b) yeTokInfoAddSepStr(b, tokInfo);
#define DEF_separated_repeater(a, b) yeTokInfoAddSepRepStr(b, tokInfo);

#include "ybytecode-asm-tok.h"

#undef DEF_string
#undef DEF_repeater
#undef DEF_separated_string
#undef DEF_separated_repeated
#undef DEF

  while ((tok = yeStringNextTok(str, tokInfo)) != YTOK_END) {
    switch (tok) {
    case SPACES:
    case RETURN:
      break;
    case YTOK_WORD:
      parseFunction(map, str, tokInfo);
      break;
    case CPP_COMMENT:
    still_in_comment:
      tok = yeStringNextTok(str, tokInfo);
      if (tok != YTOK_END && tok != RETURN)
	goto still_in_comment;
      break;
    default:
      DPRINT_ERR("error unexpected token: '%s'\n", yeTokString(tokInfo, tok));
      break;
    }
  }
  return -1;
}

static void *ybytecodeAllocator(void)
{
  struct YBytecodeScript *ybRet = g_new0(struct YBytecodeScript, 1);
  YScriptOps *ret = &ybRet->ops;

  if (ret == NULL)
    return NULL;
  ybRet->map = yeCreateArray(NULL, NULL);
  ret->call = ybytecodeCall;
  ret->getFastPath = ybytecodeGetFastPath;
  ret->loadFile = loadFile;
  ret->fastCall = ybytecodeFastCall;
  ret->registreFunc = ybytecodeRegistreFunc;
  ret->destroy = ybytecodeDestroy;
  return (void *)ret;
}

static int ysYBytecodeInit(void)
{
  t = ysRegister(ybytecodeAllocator);
  return t;
}

void * ysYBytecodeManager(void)
{
  if (manager)
    return manager;
  ysYBytecodeInit();
  manager = ysNewManager(NULL, t);
  return manager;
}

int ysYBytecodeEnd(void)
{
  ysDestroyManager(manager);
  manager = NULL;
  return ysUnregiste(t);
}
