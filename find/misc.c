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
      ++orig;
    } else {
      *p++ = ch;
    }
  }
  *p  = '\0';
}

/*
 * queryuser --
 *      print a message to standard error and then read input from standard
 *      input. If the input is 'y' then 1 is returned.
 */
int
queryuser(char **argv)
{
  int ch, first, nl;

  (void)fprintf(stderr, "\"%s", *argv);
  while (*++argv) {
    (void)fprintf(stderr, " %s", *argv);
  }
  (void)fprintf(stderr, "\"? ");
  (void)fflush(stderr);

  first = ch = getchar();
  for (nl = 0;;) {
    if (ch == '\n') {
      nl = 1;
      break;
    }
    if (ch == EOF) {
      break;
    }
    ch = getchar();
  }

  if (!nl) {
    (void)fprintf(stderr, "\n");
    (void)fflush(stderr);
  }
  return (first == 'y');
}

/*
 * show_path --
 *      called on SIGINFO
 */
/* ARGSUSED */
void
show_path(int sig)
{
  extern FTSENT *g_entry;
  int errno_bak;

  if (g_entry == NULL) {
    /*
     * not initialized yet.
     * assumption; pointer assigment is atomic.
     */
    return;
  }

  errno_bak = errno;
  write(STDERR_FILENO, "find path: ", 11);
  write(STDERR_FILENO, g_entry->fts_path, g_entry->fts_pathlen);
  write(STDERR_FILENO, "\n", 1);
  errno = errno_bak;
}
