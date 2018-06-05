#include <sys/cdefs.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <fts.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "find.h"

/*
 * brace_subst --
 *    Replace occurrences of {} in orig with path, and place it in a malloced
 *    area of memory set in store.
 */
void
brace_subst(char *orig, char **store, char *path, int *len)
{
  int nlen, plen, rest;
  char ch, *p, *ostore;

  plen = strlen(path);
  for (p = *store; (ch = *orig) != '\0'; ++orig) {
    if (ch == '{' && orig[1] == '}') {
      /* Length of string after the {}. */
      rest = strlen(&orig[2]);

      nlen = *len;
      while ((p - *store) + plen + rest + 1 > nlen) {
        nlen *= 2;
      }

      if (nlen > *len) {
        ostore = *store;
        if ((*store = realloc(ostore, nlen)) == NULL) {
          err(1, "realloc");
        }
        *len = nlen;
        p += *store - ostore;   /* Relocate. */
      }
      memmove(p, path, plen);
      p += plen;
      ++org;
    } else {
      *p++ = ch;
    }
  }
  *p  = '\0';
}

