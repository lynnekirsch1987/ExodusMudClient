#include <stdio.h>
#include <sys/types.h>
#include "merc.h"
EPL_FUNC *load_room_func (FILE * fp)
{
  return (NULL);
}

void load_room_prog (ROOM_INDEX_DATA * pRoom, char *filename)
{
  char tcbuf[MAX_STRING_LENGTH];
  EPL_PROG *pEplProg;
  EPL_FUNC *pEplFunc;
  FILE *fp;
  sprintf (tcbuf, "Loading room program '%s' for room vnum %d.\n",
	   filename, pRoom->vnum);
  log_string (tcbuf);
  if ((fp = fopen (filename, "r")) == NULL)
    return;
  pEplProg = alloc_perm (sizeof (*pEplProg));
//  pEplProg = reinterpret_cast<EPL_PROG *>(alloc_perm(sizeof(*pEplProg)));
  pEplProg->running = FALSE;
  pEplProg->wait = 0;
  pEplProg->funcs = NULL;
  pRoom->eplprog = pEplProg;
  while (1)
    {
      if ((pEplFunc = load_room_func (fp)) != NULL)
	{
	  pEplFunc->next = pEplProg->funcs;
	  pEplProg->funcs = pEplFunc;
	}
    }
}
